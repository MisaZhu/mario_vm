Debug.dump({'age': 19, name: 'misa'});

var i = 0;
while(true) {
	if(i == 10) {
		break;
	}
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


cc1 = "cc1";
cc2 = "cc2";
{
	let cc1 = 1;
	var cc2 = 2;
	Debug.dump(cc1);
	Debug.dump(cc2);
}
Debug.dump(cc1);
Debug.dump(cc2);


