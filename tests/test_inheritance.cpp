#include "test_assert.h"
#include <dukglue/dukglue.h>

#include <iostream>
#include <sstream>

class Shape {
public:
	Shape(int x, int y) : x_(x), y_(y) {}
	virtual ~Shape() {
		std::cout << "Shape deleted" << std::endl;
	}

	virtual std::string describe() = 0;

	inline int x() const { return x_; }
	inline int y() const { return y_; }

protected:
	int x_;
	int y_;
};

class Circle : public Shape {
public:
	Circle(int x, int y, int radius) : Shape(x, y), radius_(radius) {}

	virtual std::string describe() override {
		std::stringstream ss;
		ss << "A lazily-drawn circle, drawn at " << x_ << ", " << y_ << ", with a radius of about " << radius_;
		return ss.str();
	}

	int circleOnlyMethod() {
		return 1;
	}

protected:
	int radius_;
};

class Rectangle : public Shape {
public:
	Rectangle(int x, int y, int w, int h) : Shape(x, y), width_(w), height_(h) {}

	virtual std::string describe() override {
		int hw = width_ / 2;
		int hh = height_ / 2;
		std::stringstream ss;
		ss << "A boxy rectangle, encompassing all from " << x_ - hw << ", " << y_ - hh << " to " << x_ + hw << ", " << y_ + hh;
		return ss.str();
	}

protected:
	int width_;
	int height_;
};

class Triangle : public Shape {
public:
	Triangle(int x, int y) : Shape(x, y) {}
	virtual std::string describe() override {
		std::stringstream ss;
		ss << "A triangle centered at " << x() << ", " << y();
		return ss.str();
	}
};

void praiseRectangle(Rectangle* rect) {
	std::cout << "The lines, they are so straight" << std::endl;
}

void praiseCircle(Circle* circ) {
	std::cout << "Even rounder than my belly" << std::endl;
}

void praiseShape(Shape* shape) {
	std::cout << "This one is going right on the fridge" << std::endl;
}

Shape* makeShape(const std::string& name)
{
	if (name == "rectangle")
		return new Rectangle(0, 0, 1, 1);
	else if (name == "circle")
		return new Circle(0, 0, 1);
	else
		return NULL;
}

void test_inheritance() {
	duk_context* ctx = duk_create_heap_default();

	dukglue_register_method(ctx, &Shape::describe, "describe");
	dukglue_register_delete<Shape>(ctx);

	dukglue_register_constructor<Circle, int, int, int>(ctx, "Circle");
	dukglue_register_constructor<Rectangle, int, int, int, int>(ctx, "Rectangle");

	dukglue_set_base_class<Shape, Circle>(ctx);
	dukglue_set_base_class<Shape, Rectangle>(ctx);

	test_eval(ctx, "var circ = new Circle(2, 3, 4);");
	duk_pop(ctx);
	test_eval_expect(ctx, "circ.describe()", "A lazily-drawn circle, drawn at 2, 3, with a radius of about 4");  // test inherited method

	test_eval(ctx, "var rect = new Rectangle(0, 0, 4, 4)");
	duk_pop(ctx);
	test_eval_expect(ctx, "rect.describe()", "A boxy rectangle, encompassing all from -2, -2 to 2, 2");

	// test type-safety
	dukglue_register_function(ctx, praiseCircle, "praiseCircle");
	dukglue_register_function(ctx, praiseRectangle, "praiseRectangle");
	dukglue_register_function(ctx, praiseShape, "praiseShape");

	test_eval(ctx, "praiseCircle(circ)");
	duk_pop(ctx);
	test_eval_expect_error(ctx, "praiseCircle(rect)");

	test_eval(ctx, "praiseRectangle(rect)");
	duk_pop(ctx);
	test_eval_expect_error(ctx, "praiseRectangle(circ)");

	test_eval(ctx, "praiseShape(circ)");
	test_eval(ctx, "praiseShape(rect)");
	duk_pop_2(ctx);

	test_eval(ctx, "circ.delete()");  // test inherited delete
	test_eval(ctx, "rect.delete()");
	duk_pop_2(ctx);

#ifdef DUKGLUE_INFER_BASE_CLASS
	// test that we pull from the compile-time type if no
	// prototype for the run-time type exists when
	// DUKGLUE_INFER_BASE_CLASS is enabled
	{
		Shape* tri = new Triangle(4, 3);
		std::string str = dukglue_pcall_method<std::string>(ctx, tri, "describe");
		test_assert(str == "A triangle centered at 4, 3");
		delete tri;
	}
#endif

	// test that we are picking the JavaScript prototype based
	// on run-time type, not static type (issue #3)
	{
		dukglue_register_function(ctx, makeShape, "makeShape");
		dukglue_register_method(ctx, &Circle::circleOnlyMethod, "circleOnlyMethod");

		test_eval(ctx, "var rect = makeShape('rectangle'); var circ = makeShape('circle');");
		duk_pop(ctx);

		test_eval_expect_error(ctx, "rect.circleOnlyMethod()");
		test_eval_expect(ctx, "circ.circleOnlyMethod();", 1);

		test_eval(ctx, "rect.delete(); circ.delete();");
		duk_pop(ctx);
	}

	test_assert(duk_get_top(ctx) == 0);
	duk_destroy_heap(ctx);

	std::cout << "Inheritance tested OK" << std::endl;
}
