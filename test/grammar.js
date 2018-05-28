var i = 0;
while(i<10) {
	print("loop: " + i + "\n");
	i++;
}

a = {
	"name" : "misa"
};

a.name = "xx";
a.age = 24;
Debug.dump(a);

b = [0, 1, 2];
b[10] = "hhh";

Debug.dump(b);

