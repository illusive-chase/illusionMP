#pragma once
#include "basic_type.hh"
#include "container.hh"
#include "fixed_string.hh"

namespace illusion {

	namespace _impl_display {
		using namespace illusion::container;

		template<class T> struct instance;
		template<char ...C> struct instance<string<C...>> {
			static constexpr char value[] = { C...,'\0' };
		};

		template<class T> struct to_string;

		template<char ...Extra> struct prefix {
			template<class T>
			using value = append<string<Extra...>, typename to_string<T>::value>;
		};

		template<char ...Extra> struct suffix {
			template<class T>
			using value = append<typename to_string<T>::value, string<Extra...>>;
		};

		template<int N> struct to_string<integer<N>> :
			std::conditional_t<
			bool(N < 0),
		 	_impl_basic::push_front<typename to_string<integer<-N>>::value, character<'-'>>,
			_impl_basic::push_front<typename to_string<integer<N / 10>>::value, character<(N % 10) + '0'>>
			>::value {};
		template<> struct to_string<integer<0>> : string<'0'> {};

		template<> struct to_string<boolean<true>> : string<'t', 'r', 'u', 'e'> {};
		template<> struct to_string<boolean<false>> : string<'f', 'a', 'l', 's', 'e'> {};
		template<> struct to_string<list<>> : string<'[', ']'> {};

		template<class T, class = void> struct list_to_string;
		template<class T>
		struct list_to_string<T, std::enable_if_t<is_string<T>>>
			: push_front<push_back<T, character<'\"'>>, character<'\"'>> {};
		template<class T>
		struct list_to_string<T, std::enable_if_t<!is_string<T>>>
			: append<string<'[', ' '>, push_back<flat_map<typename suffix<' '>::value, T>, character<']'>>> {};

		template<class ...T>
		struct to_string<list<T...>> : list_to_string<list<T...>> {};
		
		
	}

	template<class T>
	constexpr auto& to_string = _impl_display::instance<typename _impl_display::to_string<T>::value>::value;
}