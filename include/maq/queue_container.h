#ifndef _MAQ_QUEUE_CONTAINER_H_
#define _MAQ_QUEUE_CONTAINER_H_

#include <queue>
#include <list>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>

#include "maq/plugins/disable.h"
#include "maq/detail/multi_index_keys.h"


namespace maq {

namespace mi = boost::multi_index;
template<class Type, class Deferred = plugins::disable, class Deadline = plugins::disable>
class queue_container
{
    typedef typename Deferred::template bind<Type>::type deferred_t;
    typedef typename Deadline::template bind<Type>::type deadline_t;

    struct sequenced_tag {};

    typedef typename detail::deadline_key<Type>::tag deadline_tag;
    typedef typename detail::deadline_key<Type>::ordered deadline_ordered;

    typedef mi::multi_index_container<Type,
                mi::indexed_by<mi::sequenced<>, deadline_ordered> > container_t;

    typedef typename container_t::template index<deadline_tag>::type deadline_container_t;

    typedef typename container_t::iterator iterator;
    typedef typename deadline_container_t::iterator diterator;

public:
    typedef queue_container<Type, Deferred, Deadline> this_type;
    typedef Type value_type;

    // Provide compatible interface for container passed to plugins
    class wrap_container
    {
    public:
        wrap_container(container_t& c):container_(c){}

        template <class Iterator>
        void insert(const Iterator& begin, const Iterator& end)
        {
            /*Iterator it = begin;
            for(; it != end; it++)
                container_.push(*it);
                */
            container_.insert(container_.end(), begin, end);
        }
    private:
        container_t& container_;
    };

public:
    static std::string type_name()
    {
        return std::string(std::string("queue_container<") +
                deferred_t::type_name() + ", " +
                deadline_t::type_name() + ">");
    }

    queue_container();

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

    // Release all task from deferred plugin
    void release_all();

    size_t size() const { return container_.size(); }
    bool empty() const { return container_.empty(); }

    // Get time for nearest deferred task
    void nearest_delay_time(boost::chrono::system_clock::time_point& tp) const;

    // Direct access to the operation postprocessing in deferred plugin
    void check_delayed_queue();

    // Move all expired task to arg 'tasks'
    void get_expired_tasks(std::list<Type>& tasks);

private:
    container_t container_;
    wrap_container sequeced_container_;
    deferred_t deffered_plugin_;
    deadline_t deadline_plugin_;
};

/*
 * Specialization for disabled deadline plugin
 */
template <class Type, class Deferred>
class queue_container<Type, Deferred, plugins::disable>
{
    typedef typename Deferred::template bind<Type>::type deferred_t;
    typedef typename plugins::disable::template bind<Type>::type deadline_t;

    typedef std::queue<Type> container_t;
    typedef container_t iterator;

public:
    typedef queue_container<Type, Deferred, plugins::disable> this_type;
    typedef Type value_type;

    // Provide compatible interface for container passed to plugins
    class wrap_container
    {
    public:
        wrap_container(container_t& c):container_(c){}

        template <class Iterator>
        void insert(const Iterator& begin, const Iterator& end)
        {
            Iterator it = begin;
            for(; it != end; it++)
                container_.push(*it);
        }
    private:
        container_t& container_;
    };

public:
    static std::string type_name()
    {
        return std::string(std::string("queue_container<") + deferred_t::type_name() + ">");
    }

    queue_container();

    void start();
    void stop();
    void join();

    void push(const value_type& v);

    value_type pop();

    // Fast access to queue, avoid any additional options
    // It can be used for fast cancel all tasks
    value_type force_pop();

    // Release all task from deferred plugin
    void release_all();

    size_t size() const { return container_.size(); }
    bool empty() const { return container_.empty(); }

    // Get time for nearest deferred task
    void nearest_delay_time(boost::chrono::system_clock::time_point& tp) const;

    // Direct access to the operation postprocessing in deferred plugin
    void check_delayed_queue();

    // Method not used. Needed for compatibility.
    void ts_context(const maq::detail::ts_context&){};

private:
    container_t container_;
    wrap_container wrap_container_;
    deferred_t deffered_plugin_;
};

/*
 * template<class Type, class Deferred = plugins::disable, class Deadline = plugins::disable>
 * class queue_container
 */

template<class T, class Deferred, class Deadline>
queue_container<T, Deferred, Deadline>::queue_container()
    : container_()
    , sequeced_container_(container_)
{
}

template<class T, class Deferred, class Deadline>
void queue_container<T, Deferred, Deadline>::start()
{
    deffered_plugin_.start();
    deadline_plugin_.start();
}

template<class T, class Deferred, class Deadline>
void queue_container<T, Deferred, Deadline>::stop()
{
    deffered_plugin_.stop();
    deadline_plugin_.stop();
}

template<class T, class Deferred, class Deadline>
void queue_container<T, Deferred, Deadline>::join()
{
    deffered_plugin_.join();
    deadline_plugin_.join();
}

template<class T, class Deferred, class Deadline>
void queue_container<T, Deferred, Deadline>::ts_context(const maq::detail::ts_context& s)
{
    deadline_plugin_.ts_context(s, boost::bind(&this_type::get_expired_tasks, this, _1));
}

template<class T, class Deferred, class Deadline>
void queue_container<T, Deferred, Deadline>::push(const value_type& v)
{
    if(!deffered_plugin_.preprocessing(v))
        container_.push_back(v);

    deadline_plugin_.preprocessing(v);
}

template<class T, class Deferred, class Deadline>
T queue_container<T, Deferred, Deadline>::pop()
{
    deffered_plugin_.postprocessing(sequeced_container_);

    iterator it = container_.begin();
    value_type v = *it;
    container_.erase(it);

    deadline_plugin_.postprocessing(container_.template get<deadline_tag>());
    return v;
}

template<class T, class Deferred, class Deadline>
T queue_container<T, Deferred, Deadline>::force_pop()
{
    iterator it = container_.begin();
    value_type v = *it;
    container_.erase(it);
    return v;
}

template<class T, class Deferred, class Deadline>
void queue_container<T, Deferred, Deadline>::release_all()
{
    deffered_plugin_.release_all(sequeced_container_);
}

template<class T, class Deferred, class Deadline>
void queue_container<T, Deferred, Deadline>::get_expired_tasks(std::list<T>& tasks)
{
    deadline_container_t& con = container_.template get<deadline_tag>();
    diterator it = con.begin();
    diterator end = con.end();

    while((it != end) && deadline_plugin_.is_expired(*it))
    {
        it++;
    }

    tasks.insert(tasks.end(), con.begin(), it);
    con.erase(con.begin(), it);
    deadline_plugin_.postprocessing(container_.template get<deadline_tag>());
}

template<class T, class Deferred, class Deadline>
void queue_container<T, Deferred, Deadline>::nearest_delay_time(boost::chrono::system_clock::time_point& tp) const
{
    deffered_plugin_.nearest_time(tp);
}

template<class T, class Deferred, class Deadline>
void queue_container<T, Deferred, Deadline>::check_delayed_queue()
{
    deffered_plugin_.postprocessing(sequeced_container_);
}

/*
 * template <class Type, class Deferred>
 * class queue_container<Type, Deferred, plugins::disable>
 */

template<class T, class Deferred>
queue_container<T, Deferred, plugins::disable>::queue_container()
    : container_()
    , wrap_container_(container_)
{
}

template<class T, class Deferred>
void queue_container<T, Deferred, plugins::disable>::start()
{
    deffered_plugin_.start();
}

template<class T, class Deferred>
void queue_container<T, Deferred, plugins::disable>::stop()
{
    deffered_plugin_.stop();
}

template<class T, class Deferred>
void queue_container<T, Deferred, plugins::disable>::join()
{
    deffered_plugin_.join();
}

template<class T, class Deferred>
void queue_container<T, Deferred, plugins::disable>::push(const value_type& v)
{
    if(!deffered_plugin_.preprocessing(v))
        container_.push(v);
}

template<class T, class Deferred>
T queue_container<T, Deferred, plugins::disable>::pop()
{
    deffered_plugin_.postprocessing(wrap_container_);

    value_type v = container_.front();
    container_.pop();
    return v;
}

template<class T, class Deferred>
T queue_container<T, Deferred, plugins::disable>::force_pop()
{
    value_type v = container_.front();
    container_.pop();
    return v;
}

template<class T, class Deferred>
void queue_container<T, Deferred, plugins::disable>::release_all()
{
    deffered_plugin_.release_all(wrap_container_);
}

template<class T, class Deferred>
void queue_container<T, Deferred, plugins::disable>::nearest_delay_time(boost::chrono::system_clock::time_point& tp) const
{
    deffered_plugin_.nearest_time(tp);
}

template<class T, class Deferred>
void queue_container<T, Deferred, plugins::disable>::check_delayed_queue()
{
    deffered_plugin_.postprocessing(wrap_container_);
}

}

#endif /* QUEUE_CONTAINER_H_ */
