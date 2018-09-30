class A {
	constructor(name) {
		this.name = name;
	}

	f(a) {
		this.name = a;
	}
}


b = new A('xxx');
dump(b);

a = new A();
c = a.f('aaa');

dump(c);


function f() {
	this.age = 18;
	this.name = "xx";
}

a = f();
dump(a);


class Base {
	constructor() {
		this.b = 1;
		dump("Super Base Constructor");
	}

	f() {
		dump("Super Base");
	}
}

class Base1 extends Base {
	constructor() {
		super();
		dump("Super Base1 Constructor");
	}
	f() {
		super.f();
		dump("Super Base1");
	}
}

class Test extends Base1 {
	constructor(cc, bb) {
		super();
		this.c = cc+bb;
	}

	f() {
		super.f();
		dump("Test");
	}
}

a = new Test(2, 4);
a.f();

dump(a);
