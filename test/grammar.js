var i = 0;
while(true) {
	if(i == 10)
		break;
	else
		Debug.dump("loop: " + i);
	i++;
}

a = {
	"name" : "misa"
};

a.name = "xx";
a.age = 24;
Debug.dump(a);

arr = [0, 1, 2];
arr[10] = "hhh";
Debug.dump(arr);

s = new Bytes();
s.fromString("aaa"); 
Debug.dump(s.toString());
