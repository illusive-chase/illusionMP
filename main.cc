#include "include/parser.hh"
#include <iostream>
using namespace illusion::regex;
using namespace illusion::parser;


int main() {
	/*auto reg = make_default_regex<float>();

	while (true) {
		std::string s;
		std::cin >> s;
		std::cout << reg.match(s.c_str()) << std::endl;
	}*/

	constexpr auto parser = sequence 
		<< parse_to_int()
		<< parse_to_bool() 
		<< parse_to_string(make_regex<"<>">())
		<< parse_to_vector();

	constexpr auto result = parser.parse(illusion::str<"123true<>[1,2,3]">());
	return 0;
}