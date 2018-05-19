#include "function_traits.hpp"

#include <cstdio>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace ex1 {

template <template <class> class Op, class T, class = void>
struct is_detected : std::false_type {};

template <template <class> class Op, class T>
struct is_detected<Op, T, std::void_t<Op<T>>> : std::true_type {};

namespace {
template <template <class> class Op, class T>
constexpr bool is_detected_v = is_detected<Op, T>::value;
}

template <class T>
using uses_metadata = decltype(std::declval<T>()(
		std::declval<const int&>(), std::declval<const std::string&>()));

struct secret_garden final {
	template <class Invokable>
	void visit(Invokable&& invokable) const {
		if constexpr (is_detected_v<uses_metadata, Invokable>) {
			for (size_t i = 0; i < _data.size(); ++i) {
				std::invoke(std::forward<Invokable>(invokable), _data[i],
						_meta_data[i]);
			}
		} else {
			for (const auto& d : _data) {
				std::invoke(std::forward<Invokable>(invokable), d);
			}
		}
	}

private:
	std::vector<int> _data{ 0, 1, 2, 3, 4, 5 };
	std::vector<std::string> _meta_data{ "zero", "one", "two", "three", "four",
		"five" };
};

} // namespace ex1


namespace ex2 {

template <class T>
struct is_tuple : std::false_type {};
template <class... Ts>
struct is_tuple<std::tuple<Ts...>> : std::true_type {};

namespace {
template <class... Ts>
constexpr bool is_tuple_v = is_tuple<Ts...>::value;

// Vittorio Romeo
// https://stackoverflow.com/questions/47511415/checking-if-variadic-template-parameters-are-unique-using-fold-expressions
template <typename...>
constexpr bool is_unique = std::true_type{};

template <typename T, typename... Rest>
constexpr bool is_unique<T, Rest...> = std::bool_constant<
		(!std::is_same_v<T, Rest> && ...) && is_unique<Rest...>>{};

template <class... Ts>
constexpr bool is_tuple_unique(std::tuple<Ts...>) {
	return is_unique<Ts...>;
}
} // namespace

template <class... Args>
struct some_tuple_wrapper final {
	static_assert(is_unique<Args...>, "wrapper requires unique types");

	// Simple version.
	template <class... Ts, class Invokable>
	void execute(Invokable&& invokable) const {
		std::apply(std::forward<Invokable>(invokable),
				std::make_tuple(std::get<Ts>(_data)...));
	}

	template <class T>
	const auto& get() const {
		return std::get<T>(_data);
	}

	template <class... Ts,
			typename std::enable_if_t<sizeof...(Ts) != 1>* dummy = nullptr>
	auto get() const {
		return std::tuple<const Ts&...>{ get<Ts>()... };
	}

	template <class... Ts>
	auto get(std::tuple<Ts...>) const {
		auto ret = get<Ts...>();
		if constexpr (is_tuple_v<std::decay_t<decltype(ret)>>) {
			return ret;
		} else {
			return std::make_tuple(ret);
		}
	}

	template <class Invokable>
	void execute_freedom(Invokable&& invokable) const {
		static_assert(is_tuple_unique(function_traits<Invokable>::args_decay{}),
				"only unique parameters are accepted");

		auto dummy = function_traits<Invokable>::args_decay{};
		if constexpr (std::tuple_size_v<decltype(dummy)> == 0) {
			static_assert(false, "tsk tsk tsk");
		}

		std::apply(std::forward<Invokable>(invokable), get(dummy));
	}

private:
	std::tuple<Args...> _data{ Args{ 65 }... };
};
} // namespace ex2

int main(int, char**) {

	// Simple example, strict API.
	ex1::secret_garden garden{};
	printf("\nExample 1\n");

	garden.visit([](const auto& data) { printf("%d\n", data); });
	printf("\n");

	garden.visit([](const auto& data, const auto& meta_data) {
		printf("%d - %s\n", data, meta_data.c_str());
	});

	// Less simple example, quite free API.
	printf("\nExample 2\n");

	ex2::some_tuple_wrapper<int, double, float, char, short> tuple_burrito{};

	tuple_burrito.execute<char, short>(
			[](char c, short s) { printf("%c, %d\n", c, s); });

	tuple_burrito.execute_freedom(
			[](char c, short s) { printf("%c, %d\n", c, s); });

	tuple_burrito.execute_freedom(
			[](const char& c, const double& d, const int& i) {
				printf("%c, %f, %d\n", c, d, i);
			});

	return 0;
}
