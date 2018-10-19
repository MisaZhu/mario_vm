let promise = new Promise(function(resolve, reject) {
	setTimeout(function() { 
		resolve("ok.\n"); 
		//reject("error!\n"); 
	}, 1000);
});


promise.then(function(value) {
		console.log("resolve: " + value);
		marioQuit();
	}, 
	function(value) {
		console.log("reject: " + value);
		marioQuit();
	});

console.log("main task going.\n");

marioLoop();
