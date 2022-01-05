#pragma once
#include "container.hh"
#include "fixed_string.hh"


namespace illusion {

	namespace regex {
		namespace _regex_impl {

			using illusion::container::list;

			template<char C>
			struct has_char_value {
				template<class N> struct closure : pack<boolean<N::value::charvalue::value == C>> {};
			};

			template<class N> using get_char_value = pack<typename N::value::charvalue>;
			template<class N> using get_next_list = pack<typename N::value::next>;

			template<char C, class NextLst, class EqualLst> struct Node : none {};
			template<char C, class ...NextNodes, class ...EqualNodes> struct Node<C, list<NextNodes...>, list<EqualNodes...>> {
				using value = Node<C, list<NextNodes...>, list<EqualNodes...>>;
				using charvalue = character<C>;
				using next = list<NextNodes...>;
				using equal = list<EqualNodes...>;

			};

			using FinalNode = Node<0, list<>, list<>>;

			template<class N>
			struct get_charset : container::map<get_char_value, typename N::next> {};

			template<class N>
			struct get_equalset
				: container::push_front<unpack<container::flat_map<get_equalset, typename N::equal>>, N> {};

			template<char C, class NodeLst>
			struct direct_trans : container::filter<typename has_char_value<C>::closure, NodeLst> {};

			template<char C, class NodeLst>
			struct full_trans : container::unique<unpack<container::flat_map<get_equalset, unpack<direct_trans<C, NodeLst>>>>> {};

			template<class K, class V>
			struct table_item : V {
				using key = K;
			};


			template<class NodeLst>
			struct matcher {


				template<class C>
				using transfer = pack<table_item<C,
					typename full_trans<
					C::value,
					typename container::flat_map<get_next_list, NodeLst>::value
					>::value
				>>;
				using is_final_node = typename container::contain<NodeLst, FinalNode>::value;
				using transfer_table = typename container::map<
					transfer,
					typename container::unique<
					typename container::flat_map<get_charset, NodeLst>::value
					>::value
				>::value;

				template<class ...Items>
				inline constexpr static const char* match_impl(const char* curr, const char* next, list<Items...>) {
					const char* res = nullptr;
					(... || (Items::key::value == *curr && ((res = matcher<typename Items::value>::match(next)), true)));
					return res;
				}

				constexpr static const char* match(const char* p) {
					if (*p == 0) return is_final_node::value ? p : nullptr;
					const char* result = match_impl(p, p + 1, transfer_table{});
					if (is_final_node::value && !result) return p;
					return result;
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
			struct self_equal_node : Node<0, list<>, list<R<
				unpack<container::push_front<EndLst, self_equal_node<R, EndLst>>>>>> {};

			template<template<class> class R>
			struct build_more {
				template<class EndLst>
				using value = self_equal_node<R, EndLst>;
			};

			template<template<class> class R>
			using build_many = build_optional<typename build_more<R>::value>;
		}

		template<template<class> class Builder>
		struct Regex {

			constexpr Regex<typename _regex_impl::build_more<Builder>::value> more() const { return {}; }
			constexpr Regex<typename _regex_impl::build_many<Builder>::value> many() const { return {}; }
			constexpr Regex<typename _regex_impl::build_optional<Builder>::value> optional() const { return {}; }

			constexpr const char* match(const char* p) const {
				using NFA = typename Builder<container::list<_regex_impl::FinalNode>>::value;
				return _regex_impl::matcher<typename _regex_impl::get_equalset<NFA>::value>::match(p);
			}
		};

		namespace _regex_impl {
			template<char ... Cs>
			constexpr Regex<typename build_str<Cs...>::value> make_regex_impl(string<Cs...>) {
				return {};
			}
			template<char ... Cs>
			constexpr Regex<typename build_any<Cs...>::value> any_of_impl(string<Cs...>) {
				return {};
			}
		}

		

		template<fixed_string Str>
		constexpr auto make_regex() {
			return _regex_impl::make_regex_impl(str<Str>{});
		}

		template<fixed_string Str>
		constexpr auto any_of() {
			return _regex_impl::any_of_impl(str<Str>{});
		}

		template<template<class> class Builder1, template<class> class Builder2>
		constexpr Regex<typename _regex_impl::build_options<Builder1, Builder2>::value> operator | (Regex<Builder1>, Regex<Builder2>) { return {}; }

		template<template<class> class Builder1, template<class> class Builder2>
		constexpr Regex<typename _regex_impl::build_link<Builder1, Builder2>::value> operator << (Regex<Builder1>, Regex<Builder2>) { return {}; }

		constexpr auto digit() {
			return any_of<"123456789">();
		}

		constexpr auto space() {
			return any_of<" \n\t">();
		}

		constexpr auto lower_case() {
			return any_of<"abcedfghijklmnopqrstuvwxyz">();
		}

		constexpr auto upper_case() {
			return any_of<"ABCDEFGHIJKLMNOPQRSTUVWXYZ">();
		}

		constexpr auto letter() {
			return any_of<"abcedfghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ">();
		}

		template<class T>
		constexpr auto make_default_regex();

		template<>
		constexpr auto make_default_regex<int>() {
			return 
				make_regex<"+">().optional() << any_of<"123456789">() << any_of<"0123456789">().many() |
				make_regex<"0">() |
				make_regex<"-">() << any_of<"123456789">() << any_of<"0123456789">().many();
		}

		template<>
		constexpr auto make_default_regex<unsigned>() {
			return
				any_of<"123456789">() << any_of<"0123456789">().many() |
				make_regex<"0">() | 
				make_regex<"0x">() << any_of<"0123456789ABCDEFabcdef">().more();
		}


		template<>
		constexpr auto make_default_regex<float>() {
			//(+-)?((0|[1-9]\d*)(.\d*|.)?|0.?)
			return
				any_of<"+-">().optional() <<
				(((make_regex<"0">() | any_of<"123456789">() << any_of<"0123456789">().many()) <<
				  (make_regex<".">() << any_of<"0123456789">().many() | make_regex<".">()).optional()) |
				 make_regex<"0">() << make_regex<".">().optional());
		}

	}
}