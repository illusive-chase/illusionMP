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
	struct delay {
		using value = T;
	};

	struct none {
		using value = none;
	};


}