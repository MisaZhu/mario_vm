i = 0;

while(true) {
	if(i == 10)
		break;
	else {
		Debug.dump("while loop: " + i);
	}
	i++;
}

for(i=0;i<10;i++) {
	Debug.dump("for loop: " + i);
}

var a = {
	"name": "misa",
	'naxx': 'misa',
	'age': 18,
	'working': { on: 'mario' },
};

a.name = "xx";
a.age = 24;
Debug.dump(a);

arr = [1];
arr[10] = "hhh";
arr[11] = {
  foobar: 10
};
Debug.dump(arr);
