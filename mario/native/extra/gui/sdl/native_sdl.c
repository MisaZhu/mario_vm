#include "mario_js.h"

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*=====gui native functions=========*/
var_t* native_sdl_init(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)env; (void)data;
	int res = SDL_Init(SDL_INIT_VIDEO);
	if(res == 0)
		return var_new_int(true);
	return var_new_int(false);
}

var_t* native_sdl_quit(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)env; (void)data;
	SDL_Quit();
	return NULL;
}

#define CLS_EVENT "Event"

static SDL_Event event;
var_t* native_sdl_pollEvent(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)env; (void)data;
	
	SDL_PollEvent(&event);
	var_t* v = var_new_obj(NULL, NULL);
	var_add(v, "type", var_new_int(event.type));

	if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) { //keyboard event
		var_t* ke = var_new_obj(NULL, NULL);
		var_add(ke, "code", var_new_int(event.key.keysym.sym));
		var_add(v, "keyboard", ke);
	}
	return v;
}

var_t* native_sdl_getDisplayMode(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)env; (void)data;
	SDL_DisplayMode current;
	if(SDL_GetCurrentDisplayMode(0, &current) != 0) {
		return NULL;
	}
	var_t* v = var_new_obj(NULL, NULL);
	var_add(v, "w", var_new_int(current.w));
	var_add(v, "h", var_new_int(current.h));
	return v;
}

#define CLS_WINDOW "Window"
#define CLS_CANVAS "Canvas"

var_t* native_sdl_createWindow(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	
	const char* title = get_str(env, "title");
	int x = get_int(env, "x");
	int y = get_int(env, "y");
	int w = get_int(env, "w");
	int h = get_int(env, "h");
	int full = get_int(env, "fullscreen");

	int flags = 0;
	if(full != 0)
		flags |= SDL_WINDOW_FULLSCREEN;
	SDL_Window *win = SDL_CreateWindow(title, x, y, w, h, flags);

	var_t* v = new_obj(vm, CLS_WINDOW, 0);
	v->value = win;
	v->freeFunc = _free_none;//don't destroy win automaticly.
	var_add(v, "title", var_new_str(title));
	var_add(v, "x", var_new_int(x));
	var_add(v, "y", var_new_int(y));
	var_add(v, "w", var_new_int(w));
	var_add(v, "h", var_new_int(h));
	return v;
}

var_t* native_window_destroy(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	
	var_t* var = get_obj(env, THIS);
	if(var != NULL && var->value != NULL) {
		SDL_DestroyWindow((SDL_Window *)var->value);
	}
	return NULL;
}

var_t* native_window_getCanvas(vm_t* vm, var_t* env, void *data) {
	(void)data;
	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Window* win = (SDL_Window*)var->value;
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 
				SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	var_t* v = new_obj(vm, CLS_CANVAS, 0);
	if(v == NULL) 
		return NULL;
	v->value = ren;
	v->freeFunc = _free_none; //don't auto destroy
	return v;
}

//Font
#define CLS_SURFACE "Surface"
#define CLS_TEXTURE "Texture"
#define CLS_FONT "Font"

var_t* native_font_init(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)env; (void)data;
	int res = TTF_Init();
	if(res == 0)
		return var_new_int(true);
	return var_new_int(false);
}

var_t* native_font_quit(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)env; (void)data;
	TTF_Quit();
	return NULL;
}

var_t* native_font_open(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	const char* fname = get_str(env, "fname");
	int size = get_int(env, "size");

	TTF_Font* font = TTF_OpenFont(fname, size);
	if(font == NULL)
		return NULL;

	var_t* v = new_obj(vm, CLS_FONT, 0);
	v->value = font;
	v->freeFunc = _free_none;
	var_add(v, "size", var_new_int(size));
	return v;
}

var_t* native_font_close(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	
	var_t* var = get_obj(env, THIS);
	if(var != NULL && var->value != NULL) {
		TTF_CloseFont((TTF_Font*)var->value);
	}
	return NULL;
}

var_t* native_font_sizeOf(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	
	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL) {
		return NULL;
	}
	TTF_Font* font = (TTF_Font*)var->value;
	const char* s = get_str(env, "text");
	int w, h;
	TTF_SizeUTF8(font, s, &w, &h);

	var_t* v = var_new_obj(NULL, NULL);
	var_add(v, "w", var_new_int(w));
	var_add(v, "h", var_new_int(h));
	return v;
}

SDL_Surface* _font_genSurface(TTF_Font* font, const char* text, uint32_t color) {
	if(font == NULL || text == NULL)
		return NULL;

	SDL_Color clr;
	clr.a = (color >> 24) & 0xFF;
	clr.r = (color >> 16) & 0xFF;
	clr.g = (color >> 8) & 0xFF;
	clr.b = (color) & 0xFF;

	return TTF_RenderUTF8_Blended(font, text, clr);
}

var_t* native_font_genSurface(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	
	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL) {
		return NULL;
	}
	TTF_Font* font = (TTF_Font*)var->value;
	const char* text = get_str(env, "text");
	uint32_t color = (uint32_t)get_int(env, "color");
	
	SDL_Surface* surface = _font_genSurface(font, text, color);
	if(surface == NULL)
		return NULL;

	var_t* v = new_obj(vm, CLS_SURFACE, 0);
	v->value = surface;
	v->freeFunc = _free_none;
	var_add(v, "w", var_new_int(surface->w));
	var_add(v, "h", var_new_int(surface->h));
	return v;
}

var_t* native_font_genTexture(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	
	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL) {
		return NULL;
	}
	TTF_Font* font = (TTF_Font*)var->value;
	const char* text = get_str(env, "text");
	uint32_t color = (uint32_t)get_int(env, "color");
	
	SDL_Surface* surface = _font_genSurface(font, text, color);
	if(surface == NULL)
		return NULL;
		
	var = get_obj(env, "canvas");
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Renderer* ren = (SDL_Renderer*)var->value;

	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surface);
	if(tex == NULL) {
		SDL_FreeSurface(surface);
		return NULL;
	}

	var_t* v = new_obj(vm, CLS_TEXTURE, 0);
	v->value = tex;
	v->freeFunc = _free_none;
	var_add(v, "w", var_new_int(surface->w));
	var_add(v, "h", var_new_int(surface->h));
	SDL_FreeSurface(surface);
	return v;
}

var_t* native_surface_genTexture(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Surface* surface = (SDL_Surface*)var->value;
	
	var = get_obj(env, "canvas");
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Renderer* ren = (SDL_Renderer*)var->value;

	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surface);
	var_t* v = new_obj(vm, CLS_TEXTURE, 0);
	v->value = tex;
	v->freeFunc = _free_none;
	var_add(v, "w", var_new_int(surface->w));
	var_add(v, "h", var_new_int(surface->h));
	return v;
}

//Texture
var_t* native_texture_destroy(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Texture* tex = (SDL_Texture*)var->value;
	SDL_DestroyTexture(tex);
	return NULL;
}
//Canvas

var_t* native_canvas_destroy(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	SDL_DestroyRenderer(ren);
	return NULL;
}

var_t* native_canvas_refresh(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	SDL_RenderPresent(ren);
	return NULL;
}

var_t* native_canvas_clear(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	SDL_RenderClear(ren);
	return NULL;
}

static uint32_t _gfxColor(int color)  { // AARRGGBB -> AABBGGRR
	uint32_t uc = (uint32_t)color;
	uint32_t a = (uc >> 24) & 0xFF;
	uint32_t r = (uc >> 16) & 0xFF;
	uint32_t g = (uc >> 8) & 0xFF;
	uint32_t b = (uc) & 0xFF;

	uc = (a << 24) | (b << 16) | (g << 8) | r;
	return uc;
}

var_t* native_canvas_rectangle(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x1 = get_int(env, "x1");
	int y1 = get_int(env, "y1");
	int x2 = get_int(env, "x2");
	int y2 = get_int(env, "y2");
	int color = get_int(env, "color");
	rectangleColor(ren, x1, y1, x2, y2, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_roundedRectangle(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x1 = get_int(env, "x1");
	int y1 = get_int(env, "y1");
	int x2 = get_int(env, "x2");
	int y2 = get_int(env, "y2");
	int rad = get_int(env, "rad");
	int color = get_int(env, "color");
	roundedRectangleColor(ren, x1, y1, x2, y2, rad, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_filledRoundedRectangle(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x1 = get_int(env, "x1");
	int y1 = get_int(env, "y1");
	int x2 = get_int(env, "x2");
	int y2 = get_int(env, "y2");
	int rad = get_int(env, "rad");
	int color = get_int(env, "color");
	roundedBoxColor(ren, x1, y1, x2, y2, rad, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_filledRectangle(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x1 = get_int(env, "x1");
	int y1 = get_int(env, "y1");
	int x2 = get_int(env, "x2");
	int y2 = get_int(env, "y2");
	int color = get_int(env, "color");
	boxColor(ren, x1, y1, x2, y2, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_line(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x1 = get_int(env, "x1");
	int y1 = get_int(env, "y1");
	int x2 = get_int(env, "x2");
	int y2 = get_int(env, "y2");
	int color = get_int(env, "color");
	lineColor(ren, x1, y1, x2, y2, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_aaline(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x1 = get_int(env, "x1");
	int y1 = get_int(env, "y1");
	int x2 = get_int(env, "x2");
	int y2 = get_int(env, "y2");
	int color = get_int(env, "color");
	aalineColor(ren, x1, y1, x2, y2, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_thickLine(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x1 = get_int(env, "x1");
	int y1 = get_int(env, "y1");
	int x2 = get_int(env, "x2");
	int y2 = get_int(env, "y2");
	int w = get_int(env, "w");
	int color = get_int(env, "color");
	thickLineColor(ren, x1, y1, x2, y2, w, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_circle(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x = get_int(env, "x");
	int y = get_int(env, "y");
	int rad = get_int(env, "rad");
	int color = get_int(env, "color");
	circleColor(ren, x, y, rad, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_aacircle(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x = get_int(env, "x");
	int y = get_int(env, "y");
	int rad = get_int(env, "rad");
	int color = get_int(env, "color");
	aacircleColor(ren, x, y, rad, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_filledCircle(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;

	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	int x = get_int(env, "x");
	int y = get_int(env, "y");
	int rad = get_int(env, "rad");
	int color = get_int(env, "color");
	filledCircleColor(ren, x, y, rad, _gfxColor(color));
	return NULL;
}

var_t* native_canvas_drawText(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Renderer* ren = (SDL_Renderer*)var->value;

	const char* text = get_str(env, "text");
	int x = get_int(env, "x");
	int y = get_int(env, "y");
	uint32_t color = get_int(env, "color");

	var = get_obj(env, "font");
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Surface* surface = _font_genSurface((TTF_Font*)var->value, text, color);
	if(surface == NULL)
		return NULL;
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surface);
	SDL_FreeSurface(surface);
	if(tex == NULL) {
		SDL_DestroyTexture(tex);
		return NULL;
	}
	
	SDL_Rect sr;
	SDL_Rect dr;

	sr.x = 0;
	sr.y = 0;
	sr.w = surface->w;
	sr.h = surface->h;

	dr.x = x;
	dr.y = y;
	dr.w = surface->w;
	dr.h = surface->h;

	SDL_RenderCopy(ren, tex, &sr, &dr);
	SDL_DestroyTexture(tex);
	return NULL;
}

var_t* native_canvas_copySurface(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Renderer* ren = (SDL_Renderer*)var->value;

	var = get_obj(env, "surface");
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Surface* surface = (SDL_Surface*)var->value;
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surface);
	if(tex == NULL) {
		SDL_DestroyTexture(tex);
		return NULL;
	}

	SDL_Rect sr;
	var = get_obj(env, "srcRect");
	sr.x = get_int(var, "x");
	sr.y = get_int(var, "y");
	sr.w = get_int(var, "w");
	sr.h = get_int(var, "h");

	SDL_Rect dr;
	var = get_obj(env, "dstRect");
	dr.x = get_int(var, "x");
	dr.y = get_int(var, "y");
	dr.w = get_int(var, "w");
	dr.h = get_int(var, "h");

	SDL_RenderCopy(ren, tex, &sr, &dr);
	SDL_DestroyTexture(tex);
	return NULL;
}

var_t* native_canvas_copyTexture(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Renderer* ren = (SDL_Renderer*)var->value;

	var = get_obj(env, "texture");
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Texture* tex = (SDL_Texture*)var->value;;

	SDL_Rect sr;
	var = get_obj(env, "srcRect");
	sr.x = get_int(var, "x");
	sr.y = get_int(var, "y");
	sr.w = get_int(var, "w");
	sr.h = get_int(var, "h");

	SDL_Rect dr;
	var = get_obj(env, "dstRect");
	dr.x = get_int(var, "x");
	dr.y = get_int(var, "y");
	dr.w = get_int(var, "w");
	dr.h = get_int(var, "h");

	SDL_RenderCopy(ren, tex, &sr, &dr);
	return NULL;
}

var_t* native_canvas_setColor(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;

	var_t* var = get_obj(env, THIS);
	if(var == NULL || var->value == NULL) {
		return NULL;
	}
	SDL_Renderer* ren = (SDL_Renderer*)var->value;
	uint32_t color = (uint32_t)get_int(env, "color");

	int a = (color >> 24) & 0xFF;
	int r = (color >> 16) & 0xFF;
	int g = (color >> 8) & 0xFF;
	int b = (color) & 0xFF;

	SDL_SetRenderDrawColor(ren, r, g, b, a);
	return NULL;
}

//Image
#define CLS_IMAGE "Image"

var_t* native_surface_destroy(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	
	var_t* var = get_obj(env, THIS);
	if(var != NULL && var->value != NULL) {
		SDL_FreeSurface((SDL_Surface *)var->value);
	}
	return NULL;
}

var_t* native_image_loadSurface(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	const char* fname = get_str(env, "fname");
	SDL_Surface* surface = IMG_Load(fname);
	if(surface == NULL)
		return NULL;

	var_t* v = new_obj(vm, CLS_SURFACE, 0);
	v->value = surface;
	v->freeFunc = _free_none;
	var_add(v, "w", var_new_int(surface->w));
	var_add(v, "h", var_new_int(surface->h));
	return v;
}

var_t* native_image_loadTexture(vm_t* vm, var_t* env, void *data) {
	(void)vm; (void)data;
	const char* fname = get_str(env, "fname");
	SDL_Surface* surface = IMG_Load(fname);
	if(surface == NULL)
		return NULL;
	
	var_t* var = get_obj(env, "canvas");
	if(var == NULL || var->value == NULL)
		return NULL;
	SDL_Renderer* ren = (SDL_Renderer*)var->value;

	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surface);
	if(tex == NULL) {
		SDL_FreeSurface(surface);
		return NULL;
	}

	var_t* v = new_obj(vm, CLS_TEXTURE, 0);
	v->value = tex;
	v->freeFunc = _free_none;
	var_add(v, "w", var_new_int(surface->w));
	var_add(v, "h", var_new_int(surface->h));
	SDL_FreeSurface(surface);
	return v;
}

//Rect
var_t* native_rect_constructor(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	var_t* thisV = get_obj(env, THIS);
	if(thisV == NULL)
		return NULL;

	int x = get_int(env, "x");
	int y = get_int(env, "y");
	int w = get_int(env, "w");
	int h = get_int(env, "h");
	
	var_add(thisV, "x", var_new_int(x));
	var_add(thisV, "y", var_new_int(y));
	var_add(thisV, "w", var_new_int(w));
	var_add(thisV, "h", var_new_int(h));
	return thisV;
}

//Size
var_t* native_size_constructor(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	var_t* thisV = get_obj(env, THIS);
	if(thisV == NULL)
		return NULL;

	int w = get_int(env, "w");
	int h = get_int(env, "h");
	
	var_add(thisV, "w", var_new_int(w));
	var_add(thisV, "h", var_new_int(h));
	return thisV;
}

//Pos
var_t* native_pos_constructor(vm_t* vm, var_t* env, void* data) {
	(void)vm; (void)data;
	var_t* thisV = get_obj(env, THIS);
	if(thisV == NULL)
		return NULL;

	int x = get_int(env, "x");
	int y = get_int(env, "y");
	
	var_add(thisV, "x", var_new_int(x));
	var_add(thisV, "y", var_new_int(y));
	return thisV;
}

#define CLS_SDL "SDL"
#define CLS_RECT "Rect"
#define CLS_POS "Pos"
#define CLS_SIZE "Size"

void reg_native_sdl(vm_t* vm) {
	//SDL
	vm_reg_native(vm, CLS_SDL, "init()", native_sdl_init, NULL);
	vm_reg_native(vm, CLS_SDL, "quit()", native_sdl_quit, NULL);
	vm_reg_native(vm, CLS_SDL, "getDisplayMode()", native_sdl_getDisplayMode, NULL);
	vm_reg_native(vm, CLS_SDL, "createWindow(title, x, y, w, h, fullscreen)", native_sdl_createWindow, NULL);

	//Rect
	vm_reg_native(vm, CLS_RECT, "constructor(x, y, w, h)", native_rect_constructor, NULL);
	vm_reg_native(vm, CLS_RECT, "constructor()", native_rect_constructor, NULL);

	//Pos
	vm_reg_native(vm, CLS_POS, "constructor(x, y)", native_pos_constructor, NULL);
	vm_reg_native(vm, CLS_POS, "constructor()", native_pos_constructor, NULL);

	//Size
	vm_reg_native(vm, CLS_SIZE, "constructor(w, h)", native_size_constructor, NULL);
	vm_reg_native(vm, CLS_SIZE, "constructor()", native_size_constructor, NULL);

	//Event
	vm_reg_var(vm, CLS_EVENT, "QUIT", var_new_int(SDL_QUIT), true);
	vm_reg_var(vm, CLS_EVENT, "KEY_UP", var_new_int(SDL_KEYUP), true);
	vm_reg_var(vm, CLS_EVENT, "KEY_DOWN", var_new_int(SDL_KEYDOWN), true);
	vm_reg_native(vm, CLS_EVENT, "pollEvent()", native_sdl_pollEvent, NULL);

	//Window
	vm_reg_native(vm, CLS_WINDOW, "destroy()", native_window_destroy, NULL);
	vm_reg_native(vm, CLS_WINDOW, "getCanvas()", native_window_getCanvas, NULL);

	//Surface
	vm_reg_native(vm, CLS_SURFACE, "destroy()", native_surface_destroy, NULL);
	vm_reg_native(vm, CLS_SURFACE, "genTexture(canvas)", native_surface_genTexture, NULL);

	//Texture
	vm_reg_native(vm, CLS_TEXTURE, "destroy()", native_texture_destroy, NULL);

	//Image
	vm_reg_native(vm, CLS_IMAGE, "loadSurface(fname)", native_image_loadSurface, NULL);
	vm_reg_native(vm, CLS_IMAGE, "loadTexture(canvas, fname)", native_image_loadTexture, NULL);

	//Font
	vm_reg_native(vm, CLS_FONT, "init()", native_font_init, NULL);
	vm_reg_native(vm, CLS_FONT, "quit()", native_font_quit, NULL);
	vm_reg_native(vm, CLS_FONT, "open(fname, size)", native_font_open, NULL);
	vm_reg_native(vm, CLS_FONT, "close()", native_font_close, NULL);
	vm_reg_native(vm, CLS_FONT, "sizeOf(text)", native_font_sizeOf, NULL);
	vm_reg_native(vm, CLS_FONT, "genSurface(text, color)", native_font_genSurface, NULL);
	vm_reg_native(vm, CLS_FONT, "genTexture(canvas, text, color)", native_font_genTexture, NULL);

	//Canvas
	vm_reg_native(vm, CLS_CANVAS, "setColor(color)", native_canvas_setColor, NULL);
	vm_reg_native(vm, CLS_CANVAS, "destroy()", native_canvas_destroy, NULL);
	vm_reg_native(vm, CLS_CANVAS, "refresh()", native_canvas_refresh, NULL);
	vm_reg_native(vm, CLS_CANVAS, "clear()", native_canvas_clear, NULL);
	vm_reg_native(vm, CLS_CANVAS, "rectangle(x1, y1, x2, y2, color)", native_canvas_rectangle, NULL);
	vm_reg_native(vm, CLS_CANVAS, "filledRectangle(x1, y1, x2, y2, color)", native_canvas_filledRectangle, NULL);
	vm_reg_native(vm, CLS_CANVAS, "roundedRectangle(x1, y1, x2, y2, rad, color)", native_canvas_roundedRectangle, NULL);
	vm_reg_native(vm, CLS_CANVAS, "filledRoundedRectangle(x1, y1, x2, y2, rad, color)", native_canvas_filledRoundedRectangle, NULL);
	vm_reg_native(vm, CLS_CANVAS, "thickLine(x1, y1, x2, y2, w, color)", native_canvas_thickLine, NULL);
	vm_reg_native(vm, CLS_CANVAS, "line(x1, y1, x2, y2, color)", native_canvas_line, NULL);
	vm_reg_native(vm, CLS_CANVAS, "aaline(x1, y1, x2, y2, color)", native_canvas_aaline, NULL);
	vm_reg_native(vm, CLS_CANVAS, "circle(x, y, rad, color)", native_canvas_circle, NULL);
	vm_reg_native(vm, CLS_CANVAS, "aacircle(x, y, rad, color)", native_canvas_aacircle, NULL);
	vm_reg_native(vm, CLS_CANVAS, "filledCircle(x, y, rad, color)", native_canvas_filledCircle, NULL);
	vm_reg_native(vm, CLS_CANVAS, "drawText(text, x, y, font, color)", native_canvas_drawText, NULL);
	vm_reg_native(vm, CLS_CANVAS, "copySurface(surface, srcRect, dstRect)", native_canvas_copySurface, NULL);
	vm_reg_native(vm, CLS_CANVAS, "copyTexture(texture, srcRect, dstRect)", native_canvas_copyTexture, NULL);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

