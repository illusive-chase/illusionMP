#pragma once
#include "container.hh"

namespace illusion {

	template<char C, bool Final, class Next> struct TrieNode : none {};
	template<char C, bool Final, class ...NextNode>
	struct TrieNode<C, Final, list<NextNode...>> {
		static constexpr bool trans(const char* p) {
			if (*p++ != C) return false;
			return p && ((!*p && Final) || ... || NextNode::trans(p));
		}
	};

	template<class C>
	struct start_with {
		template<class Str>
		using value = boolean<!empty<Str> && std::is_same_v<front<Str>, C>>;
	};

	template<class T, class C> using choose_start_with  = map<pop_front, filter<typename start_with<C>::value, T>>;
	template<class T> using non_final = boolean<!empty<T>>;

	template<class T> struct any_empty : none {};
	template<class ...T> struct any_empty<list<T...>> : boolean<(... || empty<T>)> {};

	template<class T>
	struct make_node {
		template<class C>
		using value = TrieNode<C::value, any_empty<choose_start_with<T, C>>::value,
			map<typename make_node<choose_start_with<T, C>>::value, unique<map<front, filter<non_final, choose_start_with<T, C>>>>>>;
	};

	template<class ...NextNode>
	static constexpr bool trans_impl(const char* p, list<NextNode...>) {
		if (!p) return false;
		return (... || NextNode::trans(p));
	}

	
	template<class ...Strings>
	struct Trie {
		using NextNodes = map<typename make_node<list<Strings...>>::value, unique<map<front, filter<non_final, list<Strings...>>>>>;

		static constexpr bool contain(const char* p) {
			return trans_impl(p, NextNodes{});
		}
	};




}