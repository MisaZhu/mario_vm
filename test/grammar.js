if(x) {
	Debug.dump("error!");
}

var i = 0;

while(true) {
	if(i == 10)
		break;
	else
		Debug.dump("loop: " + i);
	i++;
}

function f() {
	return "hello";
}

i = f();
Debug.dump(i);

a = {
	"name" : "misa"
};

a.name = "xx";
a.age = 24;
Debug.dump(a);

b = [0, 1, 2];
b[10] = "hhh";

Debug.dump(b);
