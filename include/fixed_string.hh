#pragma once
#include <string_view>
#include <type_traits>
#include "basic_type.hh"
#include "container.hh"

namespace illusion {

	template <size_t N> struct fixed_string {
		char content[N] = {};
		size_t real_size{ 0 };
		constexpr fixed_string() noexcept {}
		constexpr fixed_string(const char(input)[N + 1]) noexcept {
			for (size_t i{ 0 }; i < N; ++i) {
				content[i] = static_cast<uint8_t>(input[i]);
				if ((i == (N - 1)) && (input[i] == 0)) break;
				real_size++;
			}
		}
		constexpr fixed_string(const fixed_string& other) noexcept {
			for (size_t i{ 0 }; i < N; ++i) {
				content[i] = other.content[i];
			}
			real_size = other.real_size;
		}
		constexpr size_t size() const noexcept {
			return real_size;
		}
		constexpr const char* begin() const noexcept {
			return content;
		}
		constexpr const char* end() const noexcept {
			return content + size();
		}
		constexpr char operator[](size_t i) const noexcept {
			return content[i];
		}
		template <size_t M> constexpr bool is_same_as(const fixed_string<M>& rhs) const noexcept {
			if (real_size != rhs.size()) return false;
			for (size_t i{ 0 }; i != real_size; ++i) {
				if (content[i] != rhs[i]) return false;
			}
			return true;
		}
		constexpr operator std::basic_string_view<char>() const noexcept {
			return std::basic_string_view<char>{content, size()};
		}
	};

	template <typename CharT, size_t N> fixed_string(const CharT(&)[N])->fixed_string<N - 1>;
	template <size_t N> fixed_string(fixed_string<N>)->fixed_string<N>;


	namespace _impl_string {

		template<fixed_string Str, class Seq> struct make_string;
		template<fixed_string Str, size_t ...Ns>
		struct make_string<Str, std::index_sequence<Ns...>> : string<Str[Ns]...> {};

		// O(1)
		template<class T> struct is_string { static constexpr bool value = false; };
		template<char ...C> struct is_string<string<C...>> { static constexpr bool value = true; };

	}
	

	template<fixed_string Str>
	using str = typename _impl_string::make_string<Str, std::make_index_sequence<Str.size()>>::value;

	template<class T>
	static constexpr bool is_string = _impl_string::is_string<T>::value;

}