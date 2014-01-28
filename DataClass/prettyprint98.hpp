//          Copyright Louis Delacroix 2010 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//changes: (a.b.)
//cut no tuple ifndefs
//gcc 3.4.0 and lower bug 9907: sizeof() considered as non-constant in template arguments.
//unable to compile

#ifndef H_PRETTY_PRINT
#define H_PRETTY_PRINT

#include <ostream>
#include <utility>
#include <iterator>
#include <set>

namespace pretty_print {
//	template <typename T>
//	size_t sizeof_2(const T& dt)
//{ return static_cast<size_t>(static_cast<T*>(0)+1); }
//{T tmp[1]; return (static_cast<char*>(tmp+1) - static_cast<char*>(tmp));}
//#define sizeof_2(type) ((size_t)(((type *)0) + 1))
//#define sizeof_2(var) (size_t)((char *)(&var+1)-(char*)(&var))

//#define sizeof_type( type )  (size_t)((type*)1000 + 1 )-(size_t)((type*)1000)

//#ifdef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
//#define BOOST_STATIC_CONSTANT(type, assignment) enum { assignment }
//#else
//#define BOOST_STATIC_CONSTANT(type, assignment) static const type assignment
//#endif

	template <bool, typename S, typename T> struct conditional { };
	template <typename S, typename T> struct conditional<true,  S, T> { typedef S type; };
	template <typename S, typename T> struct conditional<false, S, T> { typedef T type; };

	template <bool, typename T> struct enable_if { };
	template <typename T> struct enable_if<true, T> { typedef T type; };

//SFINAE type trait to detect whether T::const_iterator exists.
	template<typename T>
	struct has_const_iterator {
	private:
		typedef char                      yes;
		typedef struct { char array[2]; } no;

		template<typename C> static yes test(typename C::const_iterator*);
		template<typename C> static no  test(...);
	//	template<typename C> static bool test(typename C::const_iterator*) { return true; }
	//	template<typename C> static bool test(...) { return false; }
	
	//	template<typename C> static int test(typename C::const_iterator*) { return 1; }	//yes
	//	template<typename C> static int test(...) { return 0; }	//no
	//	typedef char yes;
	//	typedef long no;
	//	template<typename C> static yes test(typename C::const_iterator*);
	//	template<typename C> static no test(...);
	public:
		static const bool value = (sizeof(test<T>(0)) == sizeof(yes));
	//	BOOST_STATIC_CONSTANT(bool, value =(test<T>(0)) );
		static const bool value=(test<T>(0));
	//	static const bool value = (test<T>(0)==1);
	//	static const bool value = false;//(typeid(test<T>(0))==typeid(yes));
		typedef T type;
	};

//SFINAE type trait to detect whether "T::const_iterator T::begin/end() const" exist.
	template <typename T>
	struct has_begin_end {
		struct Dummy { typedef void const_iterator; };
		typedef typename conditional<has_const_iterator<T>::value, T, Dummy>::type TType;
		typedef typename TType::const_iterator iter;

		struct Fallback { iter begin() const; iter end() const; };
		struct Derived : TType, Fallback { };

		template<typename C, C> struct ChT;

	//	template<typename C> static char (&f(ChT<iter (Fallback::*)() const, &C::begin>*))[1];
	//	template<typename C> static char (&f(...))[2];
	//	template<typename C> static char (&g(ChT<iter (Fallback::*)() const, &C::end>*))[1];
	//	template<typename C> static char (&g(...))[2];

		static bool const beg_value = false;//(sizeof(f<Derived>(0)) == 2);
		static bool const end_value = false;//(sizeof(g<Derived>(0)) == 2);
	};

//Basic is_container template; specialize to have value "true" for all desired container types
	template<typename T> struct is_container { static const bool value = has_const_iterator<T>::value && has_begin_end<T>::beg_value && has_begin_end<T>::end_value; };
	
	template<typename T, std::size_t N> struct is_container<T[N]> { static const bool value = true; };
	template<std::size_t N> struct is_container<char[N]> { static const bool value = false; };

//Holds the delimiter values for a specific character type
	template<typename TChar>
	struct delimiters_values {
		typedef TChar char_type;
		const TChar * prefix;
		const TChar * delimiter;
		const TChar * postfix;
	};

//Defines the delimiter values for a specific container and character type
	template<typename T, typename TChar>
	struct delimiters {
		typedef delimiters_values<TChar> type;
		static const type values;
	};

//Default delimiters
	template<typename T> struct delimiters<T, char> { static const delimiters_values<char> values; };
	template<typename T> const delimiters_values<char> delimiters<T, char>::values = { "[", ", ", "]" };
	template<typename T> struct delimiters<T, wchar_t> { static const delimiters_values<wchar_t> values; };
	template<typename T> const delimiters_values<wchar_t> delimiters<T, wchar_t>::values = { L"[", L", ", L"]" };

//Delimiters for (multi)set and unordered_(multi)set
	template<typename T, typename TComp, typename TAllocator>
	struct delimiters< ::std::set<T, TComp, TAllocator>, char> { static const delimiters_values<char> values; };
	template<typename T, typename TComp, typename TAllocator>
	const delimiters_values<char> delimiters< ::std::set<T, TComp, TAllocator>, char>::values = { "{", ", ", "}" };
	template<typename T, typename TComp, typename TAllocator>
	struct delimiters< ::std::set<T, TComp, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };
	template<typename T, typename TComp, typename TAllocator>
	const delimiters_values<wchar_t> delimiters< ::std::set<T, TComp, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };
	template<typename T, typename TComp, typename TAllocator>
	struct delimiters< ::std::multiset<T, TComp, TAllocator>, char> { static const delimiters_values<char> values; };
	template<typename T, typename TComp, typename TAllocator>
	const delimiters_values<char> delimiters< ::std::multiset<T, TComp, TAllocator>, char>::values = { "{", ", ", "}" };
	template<typename T, typename TComp, typename TAllocator>
	struct delimiters< ::std::multiset<T, TComp, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };
	template<typename T, typename TComp, typename TAllocator>
	const delimiters_values<wchar_t> delimiters< ::std::multiset<T, TComp, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };

//Delimiters for pair (reused for tuple, see below (cut))
	template<typename T1, typename T2> struct delimiters< ::std::pair<T1, T2>, char> { static const delimiters_values<char> values; };
	template<typename T1, typename T2> const delimiters_values<char> delimiters< ::std::pair<T1, T2>, char>::values = { "(", ", ", ")" };
	template<typename T1, typename T2> struct delimiters< ::std::pair<T1, T2>, wchar_t> { static const delimiters_values<wchar_t> values; };
	template<typename T1, typename T2> const delimiters_values<wchar_t> delimiters< ::std::pair<T1, T2>, wchar_t>::values = { L"(", L", ", L")" };

//Iterator microtrait class to handle C arrays uniformly
	template <typename T> struct get_iterator { typedef typename T::const_iterator iter; };
	template <typename T, std::size_t N> struct get_iterator<T[N]> { typedef const T * iter; };

	template <typename T> typename enable_if<has_const_iterator<T>::value, typename T::const_iterator>::type begin(const T & c) { return c.begin(); }
	template <typename T> typename enable_if<has_const_iterator<T>::value, typename T::const_iterator>::type end(const T & c) { return c.end(); }
	template <typename T, size_t N> const T * begin(const T(&x)[N]) { return &x[0];     }
	template <typename T, size_t N> const T * end  (const T(&x)[N]) { return &x[0] + N; }

//Functor to print containers. You can use this directly if you want to specificy a non-default delimiters type.
	template<typename T, typename TChar = char, typename TCharTraits = ::std::char_traits<TChar>, typename TDelimiters = delimiters<T, TChar> >
	struct print_container_helper {
		typedef TChar char_type;
		typedef TDelimiters delimiters_type;
		typedef std::basic_ostream<TChar, TCharTraits> ostream_type;
		typedef typename get_iterator<T>::iter TIter;

		print_container_helper(const T & container) : _container(container) { }

		inline void operator()(ostream_type & stream) const {
			if (delimiters_type::values.prefix != NULL)
			stream << delimiters_type::values.prefix;

			if (begin(_container) != end(_container))
				for (TIter it = begin(_container), it_end = end(_container); ; ) {
					stream << *it;
					if (++it == it_end) break;
					if (delimiters_type::values.delimiter != NULL)
						stream << delimiters_type::values.delimiter;
				}
			if (delimiters_type::values.postfix != NULL)
				stream << delimiters_type::values.postfix;
		}
	private:
		const T & _container;
	};

//Type-erasing helper class for easy use of custom delimiters.
//Requires TCharTraits = std::char_traits<TChar> and TChar = char or wchar_t, and MyDelims needs to be defined for TChar.
//Usage: "cout << pretty_print::custom_delims<MyDelims>(x)".
	struct custom_delims_base {
		virtual ~custom_delims_base() { }
		virtual ::std::ostream & stream(::std::ostream &) = 0;
		virtual ::std::wostream & stream(::std::wostream &) = 0;
	};
	template<typename T, typename Delims>
	struct custom_delims_wrapper : public custom_delims_base {
		custom_delims_wrapper(const T & t_) : t(t_) { }
		::std::ostream & stream(::std::ostream & stream) {
			return stream << ::pretty_print::print_container_helper<T, char, ::std::char_traits<char>, Delims>(t);
		}
		::std::wostream & stream(::std::wostream & stream) {
			return stream << ::pretty_print::print_container_helper<T, wchar_t, ::std::char_traits<wchar_t>, Delims>(t);
		}
	private:
		const T & t;
	};
	template<typename Delims>
	struct custom_delims {
		template<typename Container> custom_delims(const Container & c) : base(new custom_delims_wrapper<Container, Delims>(c)) { }
		~custom_delims() { delete base; }
		custom_delims_base * base;
	};
}//namespace pretty_print

template<typename TChar, typename TCharTraits, typename Delims>
inline std::basic_ostream<TChar, TCharTraits> & operator<<(std::basic_ostream<TChar, TCharTraits> & stream, const pretty_print::custom_delims<Delims> & p) {
	return p.base->stream(stream);
}

//Template aliases for char and wchar_t delimiters
//Enable these if you have compiler support
//
//Implement as "template<T, C, A> const sdelims::type sdelims<std::set<T,C,A>>::values = { ... }."

//template<typename T> using pp_sdelims = pretty_print::delimiters<T, char>;
//template<typename T> using pp_wsdelims = pretty_print::delimiters<T, wchar_t>;


namespace std {
//Prints a print_container_helper to the specified stream.
	template<typename T, typename TChar, typename TCharTraits, typename TDelimiters>
	inline basic_ostream<TChar, TCharTraits> & operator<<(basic_ostream<TChar, TCharTraits> & stream,
			const ::pretty_print::print_container_helper<T, TChar, TCharTraits, TDelimiters> & helper) {
		helper(stream);
		return stream;
	}

//Prints a container to the stream using default delimiters
	template<typename T, typename TChar, typename TCharTraits>
	inline typename ::pretty_print::enable_if< ::pretty_print::is_container<T>::value, basic_ostream<TChar, TCharTraits>&>::type
	operator<<(basic_ostream<TChar, TCharTraits> & stream, const T & container) {
		return stream << ::pretty_print::print_container_helper<T, TChar, TCharTraits>(container);
	}

//Prints a pair to the stream using delimiters from delimiters<std::pair<T1, T2>>.
	template<typename T1, typename T2, typename TChar, typename TCharTraits>
	inline basic_ostream<TChar, TCharTraits> & operator<<(basic_ostream<TChar, TCharTraits> & stream, const pair<T1, T2> & value) {
		if (::pretty_print::delimiters<pair<T1, T2>, TChar>::values.prefix != NULL)
			stream << ::pretty_print::delimiters<pair<T1, T2>, TChar>::values.prefix;
		stream << value.first;
		if (::pretty_print::delimiters<pair<T1, T2>, TChar>::values.delimiter != NULL)
			stream << ::pretty_print::delimiters<pair<T1, T2>, TChar>::values.delimiter;
		stream << value.second;
		if (::pretty_print::delimiters<pair<T1, T2>, TChar>::values.postfix != NULL)
			stream << ::pretty_print::delimiters<pair<T1, T2>, TChar>::values.postfix;
		return stream;
	}
}//namespace std


//A wrapper for raw C-style arrays. Usage: int arr[] = { 1, 2, 4, 8, 16 };  std::cout << wrap_array(arr) << ...
namespace pretty_print {
	template<typename T>
	struct array_wrapper_n {
		typedef const T * const_iterator;
		typedef T value_type;

		array_wrapper_n(const T * const a, size_t n) : _array(a), _n(n) { }
		inline const_iterator begin() const { return _array; }
		inline const_iterator end() const { return _array + _n; }
	private:
		const T * const _array;
		size_t _n;
	};
}//namespace pretty_print

template<typename T>
inline pretty_print::array_wrapper_n<T> pretty_print_array(const T * const a, size_t n) {
	return pretty_print::array_wrapper_n<T>(a, n);
}

#endif
