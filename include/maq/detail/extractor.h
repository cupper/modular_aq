#ifndef _MAQ_DETAIL_EXTRACTOR_H_
#define _MAQ_DETAIL_EXTRACTOR_H_

#include <boost/chrono.hpp>
#include <boost/type_traits/has_dereference.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/tti/has_member_function.hpp>

#include "maq/stat_holder.h"
#include "maq/cancellable_holder.h"

namespace maq {

typedef unsigned int prio_t;

namespace detail {

using boost::chrono::system_clock;

namespace no_adl
{

namespace {
system_clock::time_point const MAX_TIME_POINT = system_clock::time_point::max();
system_clock::time_point const MIN_TIME_POINT = system_clock::time_point::min();
}

// определяем тип-темплейт has_member_function_stat
BOOST_TTI_HAS_MEMBER_FUNCTION(stat)

template <typename T>
inline typename boost::enable_if< // определяем функцию если ...
    // тип T содержит mem-fun c такой сигнатурой ...
    has_member_function_stat<void (T::*) (stat_t const&) const>
  , void
>::type
stat (T const& t, stat_t const& stat)
{
  return t.stat (stat);
}

template <typename T>
inline typename boost::enable_if< // определяем функцию если ...
    boost::mpl::or_<
    	  has_member_function_stat<void (T::*) (stat_t const&)>
      , has_member_function_stat<void (T::*) (stat_t const&) const>
    >
  , void
>::type
stat (T& t, stat_t const& stat)
{
  return t.stat (stat);
}

// fallback для случая, если не определена mem-fun 'stat'.
// stat hook, который ничего не делает.

template <typename T>
inline typename boost::disable_if<
	has_member_function_stat<void (T::*) (stat_t const&) const>
  , void
>::type
stat (T const&, stat_t const&)
{
  // do nothing
}

template <typename T>
inline typename boost::disable_if<
	boost::mpl::or_<
		  has_member_function_stat<void (T::*) (stat_t const&)>
		, has_member_function_stat<void (T::*) (stat_t const&) const>
	>
  , void
>::type
stat (T&, stat_t const&)
{
  // do nothing
}

// определяем тип-темплейт has_member_function_cancel
BOOST_TTI_HAS_MEMBER_FUNCTION(execute)

template <typename T>
inline typename boost::enable_if< // определяем функцию если ...
    // тип T содержит mem-fun c такой сигнатурой ...
    has_member_function_execute<void (T::*) () const>
  , void
>::type
execute (T const& t)
{
  return t.execute ();
}

template <typename T>
inline typename boost::enable_if< // определяем функцию если ...
    boost::mpl::or_<
      	has_member_function_execute<void (T::*) ()>
      , has_member_function_execute<void (T::*) () const>
    >
  , void
>::type
execute (T& t)
{
  return t.execute ();
}

// fallback для случая, если не определена mem-fun 'execute'.
// execute hook, который ничего не делает.

template <typename T>
inline typename boost::disable_if<
	  has_member_function_execute<void (T::*) () const>
  , void
>::type
execute (T const&)
{
  // do nothing
}

template <typename T>
inline typename boost::disable_if< // определяем функцию если ...
    boost::mpl::or_<
    	has_member_function_execute<void (T::*) ()>
      , has_member_function_execute<void (T::*) () const>
    >
  , void
>::type
execute (T&)
{
  // do nothing
}

// определяем тип-темплейт has_member_function_cancel
BOOST_TTI_HAS_MEMBER_FUNCTION(cancel)

template <typename T>
inline typename boost::enable_if< // определяем функцию если ...
    // тип T содержит mem-fun c такой сигнатурой ...
    has_member_function_cancel<void (T::*) () const>
  , void
>::type
cancel (T const& t)
{
  return t.cancel ();
}

template <typename T>
inline typename boost::enable_if< // определяем функцию если ...
    boost::mpl::or_<
        has_member_function_cancel<void (T::*) ()>
      , has_member_function_cancel<void (T::*) () const>
    >
  , void
>::type
cancel (T& t)
{
	return t.cancel ();
}

// fallback для случая, если не определена mem-fun 'cancel'.
// cancel hook, который ничего не делает.

template <typename T>
inline typename boost::disable_if<
    has_member_function_cancel<void (T::*) () const>
  , void
>::type
cancel (T const&)
{
	// do nothing
}

template <typename T>
inline typename boost::disable_if< // определяем функцию если ...
    boost::mpl::or_<
        has_member_function_cancel<void (T::*) ()>
      , has_member_function_cancel<void (T::*) () const>
    >
  , void
>::type
cancel (T&)
{
	// do nothing
}

// определяем тип-темплейт has_member_function_priority
BOOST_TTI_HAS_MEMBER_FUNCTION(priority)

// Реализация для случая, когда определена "priority () const"
template <typename T>
inline typename boost::enable_if< // определяем функцию если ...
    // тип T содержит mem-fun c такой сигнатурой ...
    has_member_function_priority<prio_t (T::*) () const>
  , prio_t // тип возврата, если функия будет вообще определена
>::type
get_priority (T const& t)
{
	return t.priority ();
}

// Реализация для случая, когда определена "priority ()" const или nonconst
template <typename T>
inline typename boost::enable_if<
    boost::mpl::or_<
        has_member_function_priority<prio_t (T::*) ()>
      , has_member_function_priority<prio_t (T::*) () const>
    >
  , prio_t
>::type
get_priority (T& t)
{
	return t.priority ();
}

// Реализация для случая, когда нет member function. Всегда возвращает 0.
template <typename T>
inline typename boost::disable_if<
    has_member_function_priority<prio_t (T::*) () const>
  , prio_t
>::type
get_priority (T const&)
{
	return 0;
}

// Реализация для случая, когда нет member function. Всегда возвращает 0.
template <typename T>
inline typename boost::disable_if<
    boost::mpl::or_<
        has_member_function_priority<prio_t (T::*) ()>
      , has_member_function_priority<prio_t (T::*) () const>
    >
  , prio_t
>::type
get_priority (T&)
{
 	return 0;
}

// определяем тип-темплейт has_member_function_deadline
BOOST_TTI_HAS_MEMBER_FUNCTION(deadline)

// Реализация для случая, когда определена "deadline () const"
template <typename T>
inline typename boost::enable_if< // определяем функцию если ...
    // тип T содержит mem-fun c такой сигнатурой ...
    has_member_function_deadline<system_clock::time_point const& (T::*) () const>
  , system_clock::time_point const& // тип возврата, если функия будет вообще определена
>::type
get_deadline (T const& t)
{
	return t.deadline ();
}

// Реализация для случая, когда определена "deadline ()" const или nonconst
template <typename T>
inline typename boost::enable_if<
    boost::mpl::or_<
    	  has_member_function_deadline<system_clock::time_point const& (T::*) ()>
      , has_member_function_deadline<system_clock::time_point const& (T::*) () const>
    >
  , system_clock::time_point const&
>::type
get_deadline (T& t)
{
	return t.deadline ();
}

// Реализация для случая, когда нет member function. Всегда возвращает 0.
template <typename T>
inline typename boost::disable_if<
	  has_member_function_deadline<system_clock::time_point const& (T::*) () const>
  , system_clock::time_point const&
>::type
get_deadline (T const&)
{
	return MAX_TIME_POINT;
}

// Реализация для случая, когда нет member function. Всегда возвращает 0.
template <typename T>
inline typename boost::disable_if<
    boost::mpl::or_<
    	  has_member_function_deadline<system_clock::time_point const& (T::*) ()>
      , has_member_function_deadline<system_clock::time_point const& (T::*) () const>
    >
  , system_clock::time_point const&
>::type
get_deadline (T&)
{
	return MAX_TIME_POINT;
}

// определяем тип-темплейт has_member_function_delay
BOOST_TTI_HAS_MEMBER_FUNCTION(delay)

// Реализация для случая, когда определена "delay () const"
template <typename T>
inline typename boost::enable_if< // определяем функцию если ...
    // тип T содержит mem-fun c такой сигнатурой ...
    has_member_function_delay<system_clock::time_point const& (T::*) () const>
  , system_clock::time_point const& // тип возврата, если функия будет вообще определена
>::type
get_delay (T const& t)
{
	return t.delay ();
}

// Реализация для случая, когда определена "delay ()" const или nonconst
template <typename T>
inline typename boost::enable_if<
    boost::mpl::or_<
    	  has_member_function_delay<system_clock::time_point const& (T::*) ()>
      , has_member_function_delay<system_clock::time_point const& (T::*) () const>
    >
  , system_clock::time_point const&
>::type
get_delay (T& t)
{
	return t.delay ();
}

// Реализация для случая, когда нет member function. Всегда возвращает 0.
template <typename T>
inline typename boost::disable_if<
    has_member_function_delay<system_clock::time_point const& (T::*) () const>
  , system_clock::time_point const&
>::type
get_delay (T const&)
{
	return MIN_TIME_POINT;
}

// Реализация для случая, когда нет member function. Всегда возвращает 0.
template <typename T>
inline typename boost::disable_if<
    boost::mpl::or_<
    	  has_member_function_delay<system_clock::time_point const& (T::*) ()>
      , has_member_function_delay<system_clock::time_point const& (T::*) () const>
    >
  , system_clock::time_point const&
>::type
get_delay (T&)
{
	return MIN_TIME_POINT;
}

}

//################################ DEREFER ###################################
template <typename T>
struct is_a_pointer : boost::mpl::false_ {};

template <typename T>
struct is_a_pointer<T*> : boost::mpl::true_
{
	typedef T value_type;
};

template <typename T>
struct is_a_pointer<T* const> : boost::mpl::true_
{
	typedef T value_type;
};

template <typename T>
struct is_a_pointer<boost::shared_ptr<T> > : boost::mpl::true_
{
	typedef T value_type;
};

template <typename T>
struct is_a_pointer<boost::shared_ptr<T>const> : boost::mpl::true_
{
	typedef T value_type;
};

template <typename T, class Enable = void>
struct deref_helper;

template <typename T>
struct deref_helper<T, typename boost::enable_if<is_a_pointer<T> >::type>
{
	typedef typename is_a_pointer<T>::value_type result_type;
	static result_type& deref (T& t) { return *t; }
};

template <typename T>
struct deref_helper<T, typename boost::disable_if<is_a_pointer<T> >::type>
{
	typedef T result_type;
	static result_type& deref (T& t) { return t; }
};

template <typename T>
typename deref_helper<T>::result_type& deref (T& t)
{
	return deref_helper<T>::deref(t);
}

template <typename T>
struct is_a_proxy : boost::mpl::false_ {};

template <typename T>
struct is_a_proxy<cancellable_proxy<T> > : boost::mpl::true_
{
	typedef T value_type;
};

template <typename T>
struct is_a_proxy<cancellable_proxy<T>const> : boost::mpl::true_
{
	typedef const T value_type;
};

template <typename T>
T& deproxy(T& t, typename boost::disable_if<is_a_proxy<T> >::type* _ = 0)
{
	assert(!_);
	return t;
}

template <typename T>
typename is_a_proxy<T>::value_type& deproxy(T& t, typename boost::enable_if<is_a_proxy<T> >::type* _ = 0)
{
	assert(!_);
	return t.get();
}

template <typename T>
struct is_a_stat : boost::mpl::false_ {};

template <typename T>
struct is_a_stat<stat_holder<T> > : boost::mpl::true_
{
	typedef T value_type;
};

template <typename T>
struct is_a_stat<stat_holder<T> const> : boost::mpl::true_
{
	typedef const T value_type;
};

template <typename T>
T& destat(T& t, typename boost::disable_if<is_a_stat<T> >::type* _ = 0)
{
	assert(!_);
	return t;
}

template <typename T>
typename is_a_stat<T>::value_type& destat(T& t, typename boost::enable_if<is_a_stat<T> >::type* _ = 0)
{
	assert(!_);
	return t.get();
}

//############################ HOOK MACROS ###################################

#define DEFINE_HOOK_DISABLE_PROXY(NAME) \
	template <typename T> \
	void NAME##_hook_is_proxy (T& t, typename boost::disable_if<is_a_proxy<T> >::type* _ = 0) \
	{ \
		assert(!_); \
		using namespace no_adl; \
		NAME(t); \
	}

#define DEFINE_HOOK_ENABLE_PROXY(NAME) \
	template <typename T> \
	void NAME##_hook_is_proxy (T& t, typename boost::enable_if<is_a_proxy<T> >::type* _ = 0) \
	{ \
		assert(!_); \
		using namespace no_adl; \
		if(t.lock()) \
			NAME(deref(t.get())); \
	}

#define DEFINE_HOOK_DISABLE_STAT(NAME) \
	template <typename T> \
	void NAME##_hook_is_stat (T& t, typename boost::disable_if<is_a_stat<T> >::type* _ = 0) \
	{ \
		assert(!_); \
		NAME##_hook_is_proxy(t); \
	}

#define DEFINE_HOOK_ENABLE_STAT(NAME) \
	template <typename T> \
	void NAME##_hook_is_stat (T& t, typename boost::enable_if<is_a_stat<T> >::type* _ = 0) \
	{ \
		assert(!_); \
		using namespace no_adl; \
		{ \
			time_guard tg(t.stat().NAME); \
			NAME##_hook_is_proxy(deref(t.get())); \
		} \
		stat(deref(deproxy(deref(t.get()))), t.stat()); \
	}

//############################################################################

DEFINE_HOOK_DISABLE_PROXY(execute)
DEFINE_HOOK_ENABLE_PROXY(execute)
DEFINE_HOOK_DISABLE_STAT(execute)
DEFINE_HOOK_ENABLE_STAT(execute)

DEFINE_HOOK_DISABLE_PROXY(cancel)
DEFINE_HOOK_ENABLE_PROXY(cancel)
DEFINE_HOOK_DISABLE_STAT(cancel)
DEFINE_HOOK_ENABLE_STAT(cancel)

}}

#endif
