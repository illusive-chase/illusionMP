#pragma once
#include "container.hh"
#include "fixed_string.hh"


namespace illusion {

	template<char C>
	struct has_char_value {
		template<class N> struct closure : boolean<N::value::charvalue::value == C> {};
	};

	template<class N> using get_char_value = N::value::charvalue;
	template<class N> using get_next_list = N::value::next;

	template<char C, class NextLst, class EqualLst> struct Node : none {};
	template<char C, class ...NextNodes, class ...EqualNodes> struct Node<C, list<NextNodes...>, list<EqualNodes...>> {
		using value = Node<C, list<NextNodes...>, list<EqualNodes...>>;
		using charvalue = character<C>;
		using next = list<NextNodes...>;
	};

	using FinalNode = Node<0, list<>, list<>>;

	template<class N> struct charset_impl;
	template<char C, class NextLst, class EqualLst>
	struct charset_impl<Node<C, NextLst, EqualLst>> :map<get_char_value, NextLst> {};
	template<class N> using charset = typename charset_impl<typename N::value>::value;

	template<class N> struct equalset_impl;
	template<class N> using equalset = typename equalset_impl<typename N::value>::value;
	template<char C, class NextLst, class EqualLst>
	struct equalset_impl<Node<C, NextLst, EqualLst>>
		: push_front<flat_map<equalset, EqualLst>, Node<C, NextLst, EqualLst>> {};

	template<char C, class NodeLst>
	struct direct_trans {
		using valid = unique<flat_map<charset, NodeLst>>;
		using has_char_C = has_char_value<C>;
		using value = filter<typename has_char_C::closure, NodeLst>;
	};

	template<char C, class NodeLst>
	struct full_trans {
		using value = unique<flat_map<equalset, typename direct_trans<C, NodeLst>::value>>;
	};

	template<class K, class V>
	struct table_item {
		using key = K;
		using value = V;
	};
	

	template<class NodeLst>
	struct matcher {
		template<class C>
		using transfer = table_item<C, typename full_trans<C::value, flat_map<get_next_list, NodeLst>>::value>;
		using is_final_node = boolean<contain<NodeLst, FinalNode>>;
		using transfer_table = map<transfer, unique<flat_map<charset, NodeLst>>>;

		template<class ...Items>
		constexpr static bool match_impl(const char* curr, const char* next, list<Items...>) {
			return (... || (Items::key::value == *curr && matcher<Items::value>::match(next)));
		}

		constexpr static bool match(const char* p) {
			if (p[0] == 0) return is_final_node::value;
			return match_impl(p, p + 1, transfer_table{});
		}
	};


	template<class EndLst, char ... Cs>
	struct build_str_impl;

	template<class EndLst, char C>
	struct build_str_impl<EndLst, C> : Node<C, list<>, EndLst> {};

	template<class EndLst, char C, char ... Cs>
	struct build_str_impl<EndLst, C, Cs...> :
		Node<C, list<typename build_str_impl<EndLst, Cs...>::value>, list<>> {};

	template<char ... Cs>
	struct build_str {
		template<class EndLst>
		using value = Node<0, list<typename build_str_impl<EndLst, Cs...>::value>, list<>>;
	};

	template<char ... Cs>
	struct build_any {
		template<class EndLst>
		using value = Node<0, list<Node<Cs, list<>, EndLst>...>, list<>>;
	};

	struct build_equal {
		template<class EndLst>
		using value = Node<0, list<>, EndLst>;
	};

	template<template<class> class A, template<class> class B>
	struct build_link {
		template<class EndLst>
		using value = A<list<B<EndLst>>>;
	};

	template<template<class> class ... Options>
	struct build_options {
		template<class EndLst>
		using value = Node<0, list<>, list<Options<EndLst>...>>;
	};

	template<template<class> class R>
	using build_optional = build_options<R, typename build_equal::value>;

	template<template<class> class R, class EndLst>
	struct self_equal_node : Node<0, list<>, list<R<push_front<EndLst, self_equal_node<R, EndLst>>>>> {};

	template<class Name>
	using make_final = Node<static_cast<char>(0xff), list<>, list<Name>>;

	template<template<class> class R>
	struct build_more {
		template<class EndLst>
		using value = self_equal_node<R, EndLst>;
	};

	template<template<class> class R>
	using build_many = build_optional<typename build_more<R>::value>;

	template<template<class> class Builder>
	struct Regex {

		using NFA = typename Builder<list<FinalNode>>::value;

		constexpr Regex<typename build_more<Builder>::value> more() const { return {}; }
		constexpr Regex<typename build_many<Builder>::value> many() const { return {}; }
		constexpr Regex<typename build_optional<Builder>::value> optional() const { return {}; }

		constexpr bool match(const char* p) const {
			return matcher<equalset<NFA>>::match(p);
		}
	};

	template<char ... Cs>
	constexpr Regex<typename build_str<Cs...>::value> make_regex_impl(string<Cs...>) {
		return {};
	}

	template<fixed_string Str>
	constexpr auto make_regex() {
		return make_regex_impl(str<Str>{});
	}

	template<char ... Cs>
	constexpr Regex<typename build_any<Cs...>::value> any_of_impl(string<Cs...>) {
		return {};
	}

	template<fixed_string Str>
	constexpr auto any_of() {
		return any_of_impl(str<Str>{});
	}

	template<template<class> class Builder1, template<class> class Builder2>
	constexpr Regex<typename build_options<Builder1, Builder2>::value> operator | (Regex<Builder1>, Regex<Builder2>) { return {}; }

	template<template<class> class Builder1, template<class> class Builder2>
	constexpr Regex<typename build_link<Builder1, Builder2>::value> operator << (Regex<Builder1>, Regex<Builder2>) { return {}; }

}