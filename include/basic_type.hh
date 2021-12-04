#pragma once

namespace illusion {

	template<int N>
	struct integer {
		constexpr static int value = N;
	};

	template<bool B>
	struct boolean {
		constexpr static bool value = B;
	};

	template<char C>
	struct character {
		constexpr static char value = C;
	};

	template<class T>
	struct pack {
		using value = T;
	};

	template<class T>
	using unpack = typename T::value;


	struct none {};


	template<class Int>
	struct is_odd : pack<boolean<Int::value % 2 == 1>> {};

	template<class Int>
	struct square : pack<integer<Int::value * Int::value>> {};

}