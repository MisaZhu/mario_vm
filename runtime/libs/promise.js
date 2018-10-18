class Promise {
	constructor(func) {
		this.doStart = func;
	}

	then(resolved, error) {
		this.doResolved = resolved;
		this.doError = error;
		doStart(doResolv, doReject);
	}

	doResolv(msg) {
		this.doResolved(msg);
	}

	doReject(msg) {
		this.doError(msg);
	}
}

