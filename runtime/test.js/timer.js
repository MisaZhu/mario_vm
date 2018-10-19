function f(s) {
	let x = 0;
	setTimer(30000, true, function() { console.log(s + x + "\n"); x++; });
}

f("timer count: ");

marioLoop();
