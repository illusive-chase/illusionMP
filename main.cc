#include "include/regex.hh"
#include <iostream>
using namespace illusion::regex;


int main() {
	auto reg = any_of<"+-">().optional() << any_of<"0123456789">().more();

	while (true) {
		std::string s;
		std::cin >> s;
		std::cout << std::boolalpha << reg.match(s.c_str()) << std::endl;
	}
}