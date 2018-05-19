#pragma once
#include <tuple>
#include <type_traits>

// Based off of Functional C++ blog post :
// https://functionalcpp.wordpress.com/2013/08/05/function-traits/

// Required for the functor overload, as the object type gets stored in args.
template <class...>
struct drop_first;

// Drop the first type from a parameter pack.
template <class T, class... Args>
struct drop_first<std::tuple<T, Args...>> {
	using type = std::tuple<Args...>;
};


template <class F>
struct function_traits;

// function pointer
template <class R, class... Args>
struct function_traits<R (*)(Args...)> : public function_traits<R(Args...)> {};

template <class R, class... Args>
struct function_traits<R(Args...)> {
	using args = std::tuple<Args...>;
	using args_decay = std::tuple<std::decay_t<Args>...>;
};

// member function pointer
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...)>
		: public function_traits<R(C&, Args...)> {};

// const member function pointer
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) const>
		: public function_traits<R(C&, Args...)> {};

// member object pointer
template <class C, class R>
struct function_traits<R(C::*)> : public function_traits<R(C&)> {};

// functor
template <class F>
struct function_traits {
private:
	using call_type = function_traits<decltype(&F::operator())>;

public:
	using args = typename drop_first<typename call_type::args>::type;
	using args_decay =
			typename drop_first<typename call_type::args_decay>::type;
};

template <class F>
struct function_traits<F&> : public function_traits<F> {};

template <class F>
struct function_traits<F&&> : public function_traits<F> {};
