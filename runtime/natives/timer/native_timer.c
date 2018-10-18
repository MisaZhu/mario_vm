#include "mario_js.h"

#include <unistd.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct st_timer {
	uint32_t usec;
	uint32_t counter;
	bool repeat;
	var_t* env;
	var_t* callback;
} timer_t;

#define TIMER_MAX 32
#define TIMER_STEP 100 

static timer_t _timers[TIMER_MAX] = { };
static pthread_mutex_t _timerLocker;

extern uint32_t _isignalNum;


int setTimer(uint32_t usec, bool repeat, var_t* env, var_t* callback) {
	pthread_mutex_lock(&_timerLocker);
	int i;
	for(i=0; i<TIMER_MAX; i++) {
		if(_timers[i].usec == 0) {
			_timers[i].usec = usec;
			_timers[i].counter = 0;
			_timers[i].repeat = repeat;
			_timers[i].env = var_ref(env);
			_timers[i].callback = var_ref(callback);
			pthread_mutex_unlock(&_timerLocker);
			return i;
		}
	}
	_debug("Set timer failed, Too many timers!\n");
	pthread_mutex_unlock(&_timerLocker);
	return -1;
}

void cancelTimer(int id) {
	if(id < 0 || id >= TIMER_MAX)
		return;

	pthread_mutex_lock(&_timerLocker);
	if(_timers[id].usec == 0) {
		_timers[id].usec = 0;
		var_unref(_timers[id].env, true);
		var_unref(_timers[id].callback, true);
	}
	pthread_mutex_unlock(&_timerLocker);
}

void tryTimers(vm_t* vm) {
	int i;
	for(i=0; i<TIMER_MAX; i++) {
		if(_timers[i].usec == 0) 
			continue;

		_timers[i].counter += TIMER_STEP;
		if(_timers[i].counter >= _timers[i].usec) {
			_timers[i].counter = 0;
			
			interrupt(vm, _timers[i].env, _timers[i].callback, NULL);

			if(!_timers[i].repeat) {
				_timers[i].usec = 0;
				var_unref(_timers[i].callback, true);
				var_unref(_timers[i].env, true);
			}
		}
	}

	usleep(TIMER_STEP);
}

void* timerThread(void* p) {
	vm_t* vm = (vm_t*)p;
	pthread_detach(pthread_self());

	while(!vm->terminated) {
		pthread_mutex_lock(&_timerLocker);
		tryTimers(vm);
		pthread_mutex_unlock(&_timerLocker);
	}

	pthread_mutex_destroy(&_timerLocker);
	return NULL;
}


/*=====timer native functions=========*/
var_t* native_timer_set(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	uint32_t usec = (uint32_t)get_int(env, "usec");
	bool repeat = get_int(env, "repeat");
	var_t* callback = get_obj(env, "callback");

	int id = setTimer(usec, repeat, env, callback);
	return var_new_int(id);
}

var_t* native_timer_cancel(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	int id = get_int(env, "id");
	cancelTimer(id);
	return NULL;
}

void native_timer_init(void* data) {
	_debug("Start timer thread.\n");
	vm_t* vm = (vm_t*)data;
	pthread_mutex_init(&_timerLocker, NULL);
	pthread_t pid;
	pthread_create(&pid, NULL, timerThread, vm);
}

void native_timer_close(void* data) {
	(void)data;
	pthread_mutex_destroy(&_timerLocker);
}

void reg_native_timer(vm_t* vm) {
	vm_reg_init(vm, native_timer_init, vm);
	vm_reg_native(vm, "", "setTimer(usec, repeat, callback)", native_timer_set, NULL);
	vm_reg_native(vm, "", "cancelTimer(id)", native_timer_cancel, NULL);
	vm_reg_close(vm, native_timer_close, vm);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

