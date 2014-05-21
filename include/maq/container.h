#ifndef _MAQ_CONTAINER_H_
#define _MAQ_CONTAINER_H_

#include "maq/priority_container.h"
#include "maq/queue_container.h"
#include "maq/plugins.h"

#include <boost/mpl/set.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>


namespace maq {

namespace detail {

template <class Plugin, class IsSet>
struct get_plugin_impl;

template <class Plugin>
struct get_plugin_impl<Plugin, boost::mpl::false_>
{
    typedef plugins::disable type;
};

template <class Plugin>
struct get_plugin_impl<Plugin, boost::mpl::true_>
{
    typedef Plugin type;
};

template <class List, class Plugin>
struct get_plugin
{
    typedef typename get_plugin_impl<Plugin,
        typename boost::mpl::has_key<List, Plugin>::type>::type type;
};

template <class List>
struct get_plugins
{
    typedef typename get_plugin<List, plugins::deferred>::type deferred_;
    typedef typename get_plugin<List, plugins::deadline>::type deadline_;
    typedef typename get_plugin<List, plugins::priority>::type priority_;
};

template <class T, class Prio, class Deferred, class Deadline>
struct container_chooser;

template <class T, class Deferred, class Deadline>
struct container_chooser<T, plugins::priority, Deferred, Deadline>
{
    typedef priority_container<T, Deferred, Deadline> type;
};

template <class T, class Deferred, class Deadline>
struct container_chooser<T, plugins::disable, Deferred, Deadline>
{
    typedef queue_container<T, Deferred, Deadline> type;
};
}

template<class T,
         class P1 = plugins::disable,
         class P2 = plugins::disable,
         class P3 = plugins::disable>
struct container
{
    typedef boost::mpl::set<P1, P2, P3> plugin_list;
    typedef detail::get_plugins<plugin_list> plugs;

    // Get free plugins
    typedef typename plugs::priority_ priority_;
    typedef typename plugs::deferred_ deferred_;
    typedef typename plugs::deadline_ deadline_;

    typedef typename detail::container_chooser<T, priority_, deferred_, deadline_>::type type;
};

}

#endif
