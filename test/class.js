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


class Base {
	constructor() {
		this.b = 1;
		Debug.dump("Super Base Constructor");
	}

	f() {
		Debug.dump("Super Base");
	}
}

class Base1 extends Base {
	constructor() {
		super();
		Debug.dump("Super Base1 Constructor");
	}
	f() {
		super.f();
		Debug.dump("Super Base1");
	}
}

class Test extends Base1 {
	constructor(cc, bb) {
		super();
		this.c = cc+bb;
	}

	f() {
		super.f();
		Debug.dump("Test");
	}
}

a = new Test(2, 4);
a.f();

Debug.dump(a);
