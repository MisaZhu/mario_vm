class A {
	constructor(name) {
		this.name = name;
	}

	f(a) {
		this.name = a;
	}
}


b = new A('xxx');
Debug.dump(b);

a = new A();
c = a.f('aaa');

Debug.dump(c);


function f() {
	this.age = 18;
	this.name = "xx";
}

a = f();
Debug.dump(a);
