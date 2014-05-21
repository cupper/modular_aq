#ifndef _MAQ_PRIORITY_CONTAINER_H_
#define _MAQ_PRIORITY_CONTAINER_H_

#include <functional>
#include <list>

#include <boost/chrono.hpp>
#include <boost/multi_index_container.hpp>

#include "maq/plugins/disable.h"
#include "maq/detail/ts_context.h"
#include "maq/detail/multi_index_keys.h"


namespace maq {

namespace mi = boost::multi_index;

template<class Type, class Deferred = plugins::disable, class Deadline = plugins::disable>
class priority_container
{
    typedef typename Deferred::template bind<Type>::type deferred_t;
    typedef typename Deadline::template bind<Type>::type deadline_t;
public:
    typedef priority_container<Type, Deferred, Deadline> this_type;
    typedef Type value_type;

private:

    typedef typename detail::deadline_key<Type>::tag deadline_tag;
    typedef typename detail::deadline_key<Type>::ordered deadline_ordered;

    typedef typename detail::priority_key<Type>::tag priority_tag;
    typedef typename detail::priority_key<Type>::ordered priority_ordered;

    typedef mi::multi_index_container<value_type,
            mi::indexed_by<deadline_ordered, priority_ordered> > container_t;

    typedef typename container_t::template index<priority_tag>::type prio_container_t;
    typedef typename container_t::template index<deadline_tag>::type deadline_container_t;

    typedef typename prio_container_t::iterator Iterator;
    typedef typename deadline_container_t::iterator DIterator;

public:
    static std::string type_name()
    {
        return std::string(std::string("priotiry_container<") +
                deferred_t::type_name() + ", " +
                deadline_t::type_name() + ">");
    }

    void start();
    void stop();
    void join();

    // Set thread-safe  context
    // through it can be called any method with high level synchronization
    // it useful when you need pass some callback methods to plugins
    void ts_context(const maq::detail::ts_context& s);

    void push(const value_type& v);

    value_type pop();

    // Fast access to queue, avoid any additional options
    // It can be used for fast cancel all tasks
    value_type force_pop();

    // It release all task from deferred plugin
    void release_all();

    // Move all expired task from queue to arg 'tasks'
    void get_expired_tasks(std::list<Type>& tasks);

    size_t size() const { return container_.size(); }
    bool empty() const { return container_.empty(); }

    // Get time for nearest deferred task
    void nearest_delay_time(boost::chrono::system_clock::time_point& tp) const;

    // Direct access to the operation postprocessing in deferred plugin
    void check_delayed_queue();

private:
    container_t container_;

    deferred_t deffered_plugin_;
    deadline_t deadline_plugin_;
};

template<class T, class Deferred, class Deadline>
void priority_container<T, Deferred, Deadline>::start()
{
    deffered_plugin_.start();
    deadline_plugin_.start();
}

template<class T, class Deferred, class Deadline>
void priority_container<T, Deferred, Deadline>::stop()
{
    deffered_plugin_.stop();
    deadline_plugin_.stop();
}

template<class T, class Deferred, class Deadline>
void priority_container<T, Deferred, Deadline>::join()
{
    deffered_plugin_.join();
    deadline_plugin_.join();
}

template<class T, class Deferred, class Deadline>
void priority_container<T, Deferred, Deadline>::ts_context(const maq::detail::ts_context& s)
{
    deadline_plugin_.ts_context(s, boost::bind(&this_type::get_expired_tasks, this, _1));
}

template<class T, class Deferred, class Deadline>
void priority_container<T, Deferred, Deadline>::push(const value_type& v)
{
    if(!deffered_plugin_.preprocessing(v))
        container_.insert(v);

    deadline_plugin_.preprocessing(v);
}

template<class T, class Deferred, class Deadline>
T priority_container<T, Deferred, Deadline>::pop()
{
    deffered_plugin_.postprocessing(container_.template get<priority_tag>());

    Iterator it = container_.template get<priority_tag>().begin();
    value_type v = *it;
    container_.template get<priority_tag>().erase(it);

    deadline_plugin_.postprocessing(container_.template get<deadline_tag>());
    return v;
}

template<class T, class Deferred, class Deadline>
T priority_container<T, Deferred, Deadline>::force_pop()
{
    Iterator it = container_.template get<priority_tag>().begin();
    value_type v = *it;
    container_.template get<priority_tag>().erase(it);
    return v;
}

template<class T, class Deferred, class Deadline>
void priority_container<T, Deferred, Deadline>::release_all()
{
    deffered_plugin_.release_all(container_.template get<priority_tag>());
}

template<class T, class Deferred, class Deadline>
void priority_container<T, Deferred, Deadline>::get_expired_tasks(std::list<T>& tasks)
{
    deadline_container_t& con = container_.template get<deadline_tag>();
    DIterator it = con.begin();
    DIterator end = con.end();

    while((it != end) && deadline_plugin_.is_expired(*it))
    {
        it++;
    }

    tasks.insert(tasks.end(), con.begin(), it);
    con.erase(con.begin(), it);
    deadline_plugin_.postprocessing(container_.template get<deadline_tag>());
}

template<class T, class Deferred, class Deadline>
void priority_container<T, Deferred, Deadline>::nearest_delay_time(boost::chrono::system_clock::time_point& tp) const
{
    deffered_plugin_.nearest_time(tp);
}

template<class T, class Deferred, class Deadline>
void priority_container<T, Deferred, Deadline>::check_delayed_queue()
{
    deffered_plugin_.postprocessing(container_.template get<priority_tag>());
}

}


#endif
