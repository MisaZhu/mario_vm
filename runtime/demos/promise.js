let promise = new Promise(function(resolv, reject) {
	setTimeout(function() { 
		resolv("ok.\n"); 
		//reject("error!\n"); 
	}, 1000);
});


promise.then(function(value) {
		console.log("resolv: " + value);
		marioQuit();
	}, 
	function(value) {
		console.log("reject: " + value);
		marioQuit();
	});

console.log("main task going.\n");

marioLoop();
