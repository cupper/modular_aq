#ifndef _MAQ_EXTRACTOR_H_
#define _MAQ_EXTRACTOR_H_

#include <maq/detail/extractor.h>


namespace maq {

// ###########################################################################
// This functions can be overloaded by user

template<typename T>
void execute_hook(T& t)
{
    using namespace detail;
    execute_hook_is_stat(deref(t));
}

template<typename T>
void cancel_hook(T& t)
{
    using namespace detail;
    cancel_hook_is_stat(deref(t));
}

template<typename T>
prio_t extract_priority(T& t)
{
    using namespace detail;
    using namespace detail::no_adl;
    return get_priority(deref(deproxy(deref(destat(deref(t))))));
}

template<typename T>
boost::chrono::system_clock::time_point const& extract_deadline(T& t)
{
    using namespace detail;
    using namespace detail::no_adl;
    return get_deadline(deref(deproxy(deref(destat(deref(t))))));
}

template<typename T>
boost::chrono::system_clock::time_point const& extract_delay(T& t)
{
    using namespace detail;
    using namespace detail::no_adl;
    return get_delay(deref(deproxy(deref(destat(deref(t))))));
}


}


#endif
