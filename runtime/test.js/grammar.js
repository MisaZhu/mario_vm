
var i = 0;
while(true) {
	if(i == 10) {
		break;
	}
	else {
		console.ln("while loop: " + i);
	}
	i++;
}

for(i=0;i<10;i++) {
	console.ln("for loop: " + i);
}

//Object.
var a = {
	"name": "misa",
	'naxx': 'misa',
	'age': 18,
	'working': { on: 'mario' },
};

a.name = "xx";
a.age = 24;
console.ln(a);

arr = [1];
arr[10] = "hhh";
arr[11] = {
  foobar: 10
};
console.ln(arr);

//var and let
cc1 = "cc1";
cc2 = "cc2";
{
	let cc1 = 1;
	var cc2 = 2;
	console.ln(cc1);
	console.ln(cc2);
}
console.ln(cc1);
console.ln(cc2);

//callback
function f(callback, s) {
	callback(s);
}
f(function(x) { console.ln(x); }, "callback test");

const x = "aaa";
x = "bbb";
