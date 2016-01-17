#include <iostream>

void test_framework();
void test_classes();
void test_primitives();
void test_inheritance();
void test_multiple_contexts();

int main() {
	test_framework();
	test_primitives();
	test_classes();
	test_inheritance();
	test_multiple_contexts();

	std::cout << "All tests passed!" << std::endl;

	return 0;
}