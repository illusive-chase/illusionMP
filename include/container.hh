#pragma once
#include "basic_type.hh"
#include <type_traits>

namespace illusion {

	template<class ... T>
	struct list { using value = list<T...>; };

	namespace _impl_basic {

		// O(1)
		template<class Container> struct size : none {};
		template<template<class ...> class Container, class ... T>
		struct size<Container<T...>> {
			static constexpr size_t value = sizeof...(T);
		};

		// O(1)
		template<class Container, class Val> struct push_back : none {};
		template<template<class ...> class Container, class Val, class ... T>
		struct push_back<Container<T...>, Val> : Container<T..., Val> {};

		// O(1)
		template<class Container, class Val> struct push_front : none {};
		template<template<class ...> class Container, class Val, class ... T>
		struct push_front<Container<T...>, Val> : Container<Val, T...> {};

		// O(1)
		template<class Container> struct pop_front : none {};
		template<template<class ...> class Container, class Val, class ... T>
		struct pop_front<Container<Val, T...>> : Container<T...> {};

		// O(1)
		template<class Container> struct pop_back : none {};
		template<template<class ...> class Container, class Val, class ... T>
		struct pop_back<Container<T..., Val>> : Container<T...> {};

		// O(1)
		template<class Container> struct front : none {};
		template<template<class ...> class Container, class Val, class ... T>
		struct front<Container<Val, T...>> {
			using value = Val;
		};

		// O(1)
		template<class Container> struct back : none {};
		template<template<class ...> class Container, class Val, class ... T>
		struct back<Container<T..., Val>> {
			using value = Val;
		};

		// O(1)
		template<class Dest, class Src> struct copy : none {};
		template<template<class ...> class Dest, template<class ...> class Src, class ... T>
		struct copy<Dest<>, Src<T...>> : Dest<T...> {};

		// O(N)
		template<class ...C> struct append : none {};
		template<>
		struct append<> : list<> {};
		template<template<class ...> class Container, class ... T1>
		struct append<Container<T1...>> : Container<T1...> {};
		template<template<class ...> class Container, class ... T1, class ... T2>
		struct append<Container<T1...>, Container<T2...>> : Container<T1..., T2...> {};
		template<template<class ...> class Container, class ... T1, class ... T2, class ...C>
		struct append<Container<T1...>, Container<T2...>, C...> : 
			append<Container<T1..., T2...>, C...> {};

		// O(1)
		template<class Container, class Val> struct contain : none {};
		template<template<class ...> class Container, class Val, class ... T>
		struct contain<Container<T...>, Val> {
			constexpr static bool value = (... || std::is_same_v<Val, T>);
		};


	}


	template<class Container> static constexpr size_t size = _impl_basic::size<Container>::value;
	template<class Container> static constexpr bool empty = _impl_basic::size<Container>::value == 0U;
	template<class Container, class Val> using push_back = typename _impl_basic::push_back<Container, Val>::value;
	template<class Container, class Val> using push_front = typename _impl_basic::push_front<Container, Val>::value;
	template<class Container> using pop_front = typename _impl_basic::pop_front<Container>::value;
	template<class Container> using pop_back = typename _impl_basic::pop_back<Container>::value;
	template<class Container> using front = typename _impl_basic::front<Container>::value;
	template<class Container> using back = typename _impl_basic::back<Container>::value;
	template<class Dest, class Src> using copy = typename _impl_basic::copy<Dest, Src>::value;
	template<class ...C> using append = typename _impl_basic::append<C...>::value;
	template<class Container, class Val> static constexpr bool contain = _impl_basic::contain<Container, Val>::value;


	namespace _impl_bitset {
		template<uint8_t ... Vals> struct bitset {
			using value = bitset<Vals...>;
		};

		// O(1)
		template<bool ...Bits> struct byte;
		template<> struct byte<> { static constexpr uint8_t value = 0; };
		template<bool Val, bool ...Bits>
		struct byte<Val, Bits...> {
			static_assert(sizeof...(Bits) < 8, "ERROR @ byte: executed with more than 8 bits.");
			static constexpr uint8_t value = uint8_t{ Val } << (7 - sizeof...(Bits)) | byte<Bits...>::value;
		};


		// O(1)
		template<bool ...Bits> struct bytes : bitset<> { using first = byte<Bits...>; };
		template<bool B0, bool B1, bool B2, bool B3, bool B4, bool B5, bool B6, bool B7, bool ...Bits>
		struct bytes<B0, B1, B2, B3, B4, B5, B6, B7, Bits...> : bitset<Bits...> {
			using first = byte<B0, B1, B2, B3, B4, B5, B6, B7>;
		};


		// O(N)
		template<class T8, class T1> struct collect;
		template<uint8_t ... Ints>
		struct collect<bitset<Ints...>, bitset<>> : bitset<Ints...> {};
		template<uint8_t ... Ints, bool ... Bools>
		struct collect<bitset<Ints...>, bitset<Bools...>> : collect<
			bitset<Ints..., bytes<Bools...>::first::value>,
			typename bytes<Bools...>::value
		> {};

	}

	template<bool ...Bools>
	using bitset = typename _impl_bitset::collect<_impl_bitset::bitset<>, _impl_bitset::bitset<Bools...>>::value;

	namespace _impl_range {

		// O(N)
		template<class Container, bool ... Value> struct mask : none {};
		template<template<class ...> class Container, bool Val, bool ... Value, class Type, class ... T>
		struct mask<Container<Type, T...>, Val, Value...> : std::conditional_t <
			Val,
			_impl_basic::push_front<typename mask<Container<T...>, Value...>::value, Type>,
			mask<Container<T...>, Value...>
		>::value {
			static_assert(sizeof...(Value) == sizeof...(T), "ERROR @ mask: the lengths do not match.");
		};
		template<template<class ...> class Container> struct mask<Container<>> : Container<> {};

		// O(N)
		template<template<class> class Func, class Container> struct filter : none {};
		template<template<class ...> class Container, template<class> class Func, class ... T>
		struct filter<Func, Container<T...>> : mask<Container<T...>, (Func<T>::value)...> {};

		// O(N)
		template<template<class> class Func, class Container> struct map : none {};
		template<template<class ...> class Container, template<class> class Func, class ... T>
		struct map<Func, Container<T...>> : Container<Func<T>...> {};

		// O(N)
		template<class Container> struct unique : none {};
		template<template<class ...> class Container, class Val, class ...T>
		struct unique<Container<Val, T...>> : std::conditional_t<
			contain<Container<T...>, Val>,
			unique<Container<T...>>,
			_impl_basic::push_front<typename unique<Container<T...>>::value, Val>
		>::value {};
		template<template<class ...> class Container>
		struct unique<Container<>> : Container<> {};

		// O(1)
		template<template<class ...> class Func, class Container> struct apply : none {};
		template<template<class ...> class Container, template<class ...> class Func, class ... T>
		struct apply<Func, Container<T...>> : Func<T...> {};

	}

	template<template<class ...> class Func, class Container>
	using apply = typename _impl_range::apply<Func, Container>::value;
	template<template<class> class Func, class Container>
	using map = typename _impl_range::map<Func, Container>::value;
	template<template<class> class Func, class Container>
	using flat_map = apply<append, map<Func, Container>>;
	template<template<class> class Func, class Container>
	using filter = typename _impl_range::filter<Func, Container>::value;
	template<class Container, bool ... Value>
	using mask = typename _impl_range::mask<Container, Value...>::value;
	template<class Container>
	using unique = typename _impl_range::unique<Container>::value;


	template<char ...C>
	using string = list<character<C>...>;
	

}

