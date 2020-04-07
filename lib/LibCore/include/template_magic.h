#ifndef TEMPLATE_MAGIC
#define TEMPLATE_MAGIC

struct true_type{ static constexpr bool value = true;};
struct false_type{ static constexpr bool value = false;};

template <class T>
struct is_lvalue_ref : false_type{};

template <class T>
struct is_lvalue_ref<T&> : true_type{}; 

template <class T>
struct remove_reference{
	typedef T type;
};

template <class T>
struct remove_reference<T&>{
	typedef T type;
};

template <class T>
struct remove_reference<T&&>{
	typedef T type;
};

template <class T>
T&& forward(class remove_reference<T>::type&& t){
	static_assert(!is_lvalue_ref<T>::value, "Cannot forward lvalues");
	return static_cast<T&&>(t);
}

template <class T>
remove_reference<T>::type&& move(T&& t){
	return static_cast<remove_reference<T>::type&&>(t);
}

#endif
