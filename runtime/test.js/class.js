class A {
	constructor(name) {
		this.name = name;
	}

	f(a) {
		this.name = a;
	}
}


b = new A('xxx');
console.ln(b);

a = new A();
c = a.f('aaa');

console.ln(c);


function f() {
	this.age = 18;
	this.name = "xx";
}

a = f();
console.ln(a);


class Base {
	constructor() {
		this.b = 1;
		console.ln("Super Base Constructor");
	}

	f() {
		console.ln("Super Base");
	}
}

class Base1 extends Base {
	constructor() {
		super();
		console.ln("Super Base1 Constructor");
	}
	f() {
		super.f();
		console.ln("Super Base1");
	}
}

class Test extends Base1 {
	constructor(cc, bb) {
		super();
		this.c = cc+bb;
	}

	f() {
		super.f();
		console.ln("Test");
	}
}

a = new Test(2, 4);
a.f();

console.ln(a);
