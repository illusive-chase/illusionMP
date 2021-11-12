#include "include/trie.hh"
#include "include/fixed_string.hh"
#include "include/display.hh"
#include <iostream>
using namespace illusion;


using p = list<
	string<'a', 'b', 'c'>,
	string<'a', 'b'>,
	string<'b', 'c'>
>;

int main() {

	using trie = Trie<
		str<"dictionary">,
		str<"today">,
		str<"I am happy">
	>;
	constexpr bool b = trie::contain("today");
	std::cout << b << std::endl;
}