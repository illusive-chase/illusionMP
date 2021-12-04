#pragma once
#include "basic_type.hh"
#include <type_traits>

namespace illusion {

	namespace container {

		// struct
		template<class ... T>
		struct list { using value = list<T...>; };

		// O(1)
		// is_list[object] : boolean
		template<class Obj>
		struct is_list : pack<boolean<false>> {};
		template<class ...T>
		struct is_list<list<T...>> : pack<boolean<true>> {};

		// O(1)
		// size[list[element...]] : integer
		template<class Container> struct size : none {};
		template<class ... T>
		struct size<list<T...>> : pack<integer<sizeof...(T)>> {};
		// test
		static_assert(size<list<int, int, bool>>::value::value == 3);

		// O(1)
		// push_back[list[element...]] : list[element...]
		template<class Container, class Val> struct push_back : none {};
		template<class Val, class ... T>
		struct push_back<list<T...>, Val> : list<T..., Val> {};
		// test
		static_assert(std::is_same_v<typename push_back<list<int, int, bool>, int>::value, list<int, int, bool, int>>);

		// O(1)
		// push_front[list[element...]] : list[element...]
		template<class Container, class Val> struct push_front : none {};
		template<class Val, class ... T>
		struct push_front<list<T...>, Val> : list<Val, T...> {};
		// test
		static_assert(std::is_same_v<typename push_front<list<int, char, bool>, int>::value, list<int, int, char, bool>>);

		// O(1)
		// pop_front[list[element...]] : list[element...]
		template<class Container> struct pop_front : none {};
		template<class Val, class ... T>
		struct pop_front<list<Val, T...>> : list<T...> {};
		// test
		static_assert(std::is_same_v<typename pop_front<list<int, char, bool>>::value, list<char, bool>>);

		// O(N)
		// append[list[element1...], list[element2...], ...] : list[element1..., element2..., ...]
		// append[list[]]: list[]
		template<class ...Lsts> struct append : none {};
		template<> struct append<> : list<> {};
		template<class ... T1>
		struct append<list<T1...>> : list<T1...> {};
		template<class ... T1, class ... T2>
		struct append<list<T1...>, list<T2...>> : list<T1..., T2...> {};
		template<template<class ...> class Container, class ... T1, class ... T2, class ...C>
		struct append<Container<T1...>, Container<T2...>, C...> :
			append<Container<T1..., T2...>, C...> {};
		// test
		static_assert(std::is_same_v<typename 
					  append<list<int, char, bool>, list<double, float>, list<list<int>>>::value, 
					  list<int, char, bool, double, float, list<int>>>);

		// O(1)
		// contain[list[element...], element] : boolean
		template<class Container, class Val> struct contain : none {};
		template<template<class ...> class Container, class Val, class ... T>
		struct contain<Container<T...>, Val> : pack<boolean<(... || std::is_same_v<Val, T>)>> {};
		// test
		static_assert(std::is_same_v<typename contain<list<char, int>, int>::value, boolean<true>>);

		// O(1)
		// apply[callable[argument...], list[element...]] : callable[element...]::value
		template<template<class ...> class Func, class Container> struct apply : none {};
		template<template<class ...> class Func, class ... T>
		struct apply<Func, list<T...>> : Func<T...> {};

		// O(1)
		// delay_apply[callable[argument...], list[callable[]...]] : callable[callable[]::value...]::value
		template<template<class ...> class Func, class Container> struct delay_apply : none {};
		template<template<class ...> class Func, class ... T>
		struct delay_apply<Func, list<T...>> : Func<unpack<T>...> {};

		// O(N)
		// mask[list[element...], list[boolean...]] : list[element...]
		template<class Container, class Mask> struct mask {};
		template<> struct mask<list<>, list<>> : pack<list<>> {};
		template<bool Val, bool ... Vals, class Elem, class ... Elems>
		struct mask<list<Elem, Elems...>, list<boolean<Val>, boolean<Vals>...>> :
			std::conditional_t<
			    Val,
			    delay_apply<push_front, list<mask<list<Elems...>, list<boolean<Vals>...>>, pack<Elem>>>,
			    mask<list<Elems...>, list<boolean<Vals>...>>
			>
		{};
		// test
		static_assert(std::is_same_v<typename
					  mask<list<char, int>, list<boolean<true>, boolean<false>>>::value, 
					  list<char>>);

		// O(N)
		// filter[callable[argument], list[element...]] : list[element...]
		template<template<class> class Func, class Container> struct filter : none {};
		template<template<class> class Func, class ... T>
		struct filter<Func, list<T...>> : mask<list<T...>, list<unpack<Func<T>>...>> {};
		// test
		static_assert(std::is_same_v<typename
					  filter<is_odd, list<integer<1>, integer<2>, integer<3>>>::value,
					  list<integer<1>, integer<3>>>);

		// O(N)
		// map[callable[argument], list[element...]] : list[element...]
		template<template<class> class Func, class Container> struct map : none {};
		template<template<class> class Func, class ... T>
		struct map<Func, list<T...>> : list<unpack<Func<T>>...> {};
		// test
		static_assert(std::is_same_v<typename
					  map<square, list<integer<1>, integer<2>, integer<3>>>::value,
					  list<integer<1>, integer<4>, integer<9>>>);


		// O(N)
		// flat_map[callable[argument]->list[element...], list[element...]] : list[element...]
		template<template<class> class Func, class Container>
		using flat_map = apply<append, unpack<map<Func, Container>>>;
		// test
		static_assert(std::is_same_v<typename
					  flat_map<list, list<integer<1>, integer<2>, integer<3>>>::value,
					  list<integer<1>, integer<2>, integer<3>>>);

		

		// O(N)
		template<class Container> struct unique : none {};
		template<class Val, class ...T>
		struct unique<list<Val, T...>> :
		std::conditional_t<
			contain<list<T...>, Val>::value::value,
			unique<list<T...>>,
			delay_apply<push_front, list<unique<list<T...>>, pack<Val>>>
		> {};
		template<template<class ...> class Container>
		struct unique<Container<>> : Container<> {};
		// test
		static_assert(std::is_same_v<typename
					  unique<list<integer<6>, integer<2>, integer<3>, integer<6>>>::value,
					  list<integer<2>, integer<3>, integer<6>>>);



	}

	template<char ...C>
	using string = container::list<character<C>...>;

}

