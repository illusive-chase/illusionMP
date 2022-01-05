#include "include/parser.hh"
#include <iostream>
using namespace illusion::regex;
using namespace illusion::parser;


int main() {

	constexpr auto parser = sequence 
		<< parse_to_int()
		<< parse_to_bool() 
		<< parse_to_string(make_regex<"<>">())
		<< parse_to_vector();

	// result: tuple<int, bool, char const*, std::array<int, 3>>
	constexpr auto result = parser.parse(illusion::str<"123true<>[1,2,3]">());
	static_assert(std::is_same_v<decltype(result), const std::tuple<int, bool, char const*, std::array<int, 3>>>, "error");
	
	return 0;
}