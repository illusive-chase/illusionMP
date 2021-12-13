#pragma once
#include "regex.hh"
#include <array>


namespace illusion {

	namespace parser {

		namespace impl {

			

			template<fixed_string Str, unsigned Base, unsigned End>
			struct make_substr {
				template<unsigned ... S>
				static constexpr illusion::string<Str[S + Base]...> impl(std::index_sequence<S...>) {
					return {};
				}

				using value = decltype(impl(std::make_index_sequence<End - Base>()));
			};

			

			template<typename Ps, typename Ts, fixed_string, unsigned> struct parse;
			template<typename P, typename T, typename ...Ps, typename ...Ts, fixed_string Str, unsigned N>
			struct parse<container::list<P, Ps...>, container::list<T, Ts...>, Str, N> {
				static constexpr const char* Next = P().match(Str.begin() + N);
				using Substr = typename make_substr<Str, N, Next - Str.begin()>::value;
				using value = unpack<container::push_front<unpack<parse<container::list<Ps...>, container::list<Ts...>, Str, Next - Str.begin()>>, unpack<decltype(T::Parser(Substr{}))>>>;
			};

			template<typename P, typename ...Ps, typename ...Ts, fixed_string Str, unsigned N>
			struct parse<container::list<P, Ps...>, container::list<none, Ts...>, Str, N> {
				static constexpr const char* Next = P().match(Str.begin() + N);
				using value = unpack<parse<container::list<Ps...>, container::list<Ts...>, Str, Next - Str.begin()>>;
			};

			template<fixed_string Str, unsigned N>
			struct parse<container::list<>, container::list<>, Str, N> {
				using value = container::list<>;
			};


			template<typename ...T>
			constexpr auto forward_as_tuple(container::list<T...>) {
				return std::make_tuple((T::value)...);
			}

		}


		template<typename Ps, typename Ts>
		struct Parser {

			template<char ...C>
			constexpr Parser<unpack<container::push_back<Ps, decltype(regex::_regex_impl::make_regex_impl<illusion::string<C...>>())>>, Ts>
				operator <<(illusion::string<C...>) const {
				return {};
			}

			template<template <typename> typename Builder>
			constexpr Parser<unpack<container::push_back<Ps, regex::Regex<Builder>>>, unpack<container::push_back<Ts, none>>>
				operator <<(regex::Regex<Builder>) const {
				return {};
			}

			template<typename P>
			constexpr Parser<unpack<container::push_back<Ps, typename P::Regex>>, unpack<container::push_back<Ts, P>>>
				operator <<(P) const {
				return {};
			}

			template<char ...C>
			constexpr auto parse(illusion::string<C...>) const {
				constexpr char str[] = { C..., 0, 0 };
				return impl::forward_as_tuple(typename impl::parse<Ps, Ts, fixed_string{ str }, 0> ::value{});
			}

		};

		namespace impl {

			
			template<typename S>
			struct parse_to_int;

			template<char...C>
			struct parse_to_int<illusion::string<C...>> {
				constexpr static int parse_impl() {
					char str[] = { C..., 0 };
					int s = 0;
					int x = 0;
					const char* c = str;
					if (c[0] == '+' || c[0] == '-') s = c[0] == '-';
					for (; *c; ++c) {
						x = x * 10 + c[0] - '0';
					}
					return s ? -x : x;
				}

				using value = integer<parse_impl()>;
			};

			template<typename S>
			struct parse_to_bool;

			template<>
			struct parse_to_bool<illusion::string<'t', 'r', 'u', 'e'>> {
				using value = boolean<true>;
			};

			template<>
			struct parse_to_bool<illusion::string<'f', 'a', 'l', 's', 'e'>> {
				using value = boolean<false>;
			};


			template<char ...C>
			struct stored_string {
				constexpr static char value[] = { C..., 0 };
			};

			template<unsigned Len, std::array<int, Len> Array>
			struct stored_vector {
				constexpr static std::array<int, Len> value = Array;
			};

			template<typename S>
			struct parse_to_string;

			template<char...C>
			struct parse_to_string<illusion::string<C...>> {
				using value = stored_string<C...>;
			};

			template<typename S>
			struct parse_to_vector;

			template<char...C>
			struct parse_to_vector<illusion::string<C...>> {
				constexpr static unsigned Len = (... + (C == ',')) + 1;
				constexpr static std::array<int, Len> parse_impl() {
					char str[] = { C..., 0 };
					const char* c = str + 1;
					int i = 0;
					std::array<int, Len> ret;
					while (*c != ']') {
						int s = 0;
						int x = 0;
						if (c[0] == '+' || c[0] == '-') s = c[0] == '-';
						for (; *c != ']' && *c != ','; ++c) {
							x = x * 10 + c[0] - '0';
						}
						ret[i++] = s ? -x : x;
						if (*c != ']') ++c;
					}
					return ret;
				}

				using value = stored_vector<Len, parse_impl()>;
			};

			struct parse_to_int_t {
				template<typename S>
				inline constexpr static parse_to_int<S> Parser(S) {
					return {};
				}

				using Regex = decltype(regex::make_default_regex<int>());
			};

			struct parse_to_bool_t {
				template<typename S>
				inline constexpr static parse_to_bool<S> Parser(S) {
					return {};
				}
				using Regex = decltype(regex::make_regex<"true">() | regex::make_regex<"false">());
			};

			template<typename R>
			struct parse_to_string_t {
				template<typename S>
				inline constexpr static parse_to_string<S> Parser(S) {
					return {};
				}
				using Regex = R;
			};

			

			struct parse_to_vector_t {
				template<typename S>
				inline constexpr static parse_to_vector<S> Parser(S) {
					return {};
				}
				using Regex = decltype(regex::make_regex<"[">() << (regex::digit() << regex::make_regex<",">()).many() << regex::digit() << regex::make_regex<"]">());
			};
		}

		template<typename R>
		constexpr auto parse_to_string(R) { return impl::parse_to_string_t<R>{}; }
		constexpr auto parse_to_int() { return impl::parse_to_int_t{}; }
		constexpr auto parse_to_bool() { return impl::parse_to_bool_t{}; }
		constexpr auto parse_to_vector() { return impl::parse_to_vector_t{}; }


		

		constexpr auto sequence = Parser<container::list<>, container::list<>>();
	}


}