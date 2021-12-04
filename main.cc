#include "include/regex.hh"
#include <iostream>
using namespace illusion::regex;


int main() {
	auto reg = make_default_regex<float>();

	while (true) {
		std::string s;
		std::cin >> s;
		std::cout << std::boolalpha << reg.match(s.c_str()) << std::endl;
	}
}