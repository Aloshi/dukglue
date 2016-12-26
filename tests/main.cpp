#include <iostream>

void test_framework();
void test_classes();
void test_primitives();
void test_inheritance();
void test_multiple_contexts();
void test_properties();
void test_dukvalue();

int main() {
	test_framework();
	test_primitives();
	test_classes();
	test_inheritance();
	test_multiple_contexts();
	test_properties();
	test_dukvalue();

	std::cout << "All tests passed!" << std::endl;

	return 0;
}