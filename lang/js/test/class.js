
function f() {
	this.age = 18;
	this.name = "xx";
}

a = new f();
console.log(a);

/**/
O = {
	name: "Misa",
	age: 20,
	f: function () {
		console.log(this);	
	}
};

o = Object.create(O);
o.f();


/**/
class Base {
	constructor() {
		this.b = 1;
		console.log("Super Base Constructor");
	}

	f() {
		console.log("Super Base");
	}
}

class Base1 extends Base {
	constructor() {
		super();
		console.log("Super Base1 Constructor");
	}
	f() {
		super.f();
		console.log("Super Base1");
	}
}

class Test extends Base1 {
	constructor(cc, bb) {
		super();
		this.c = cc+bb;
	}

	f() {
		super.f();
		console.log("this: " + this);
	}
}

a = new Test(2, 4);
a.f();
