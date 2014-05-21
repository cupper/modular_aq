#ifndef _MAQ_TS_CONTAINER_H_
#define _MAQ_TS_CONTAINER_H_

#include <stdexcept>

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/chrono.hpp>

#include "maq/error.h"
#include "maq/stat_holder.h"
#include "maq/statistic.h"
#include "maq/detail/ts_context.h"


namespace maq {

template<class Container>
class ts_container: public boost::noncopyable
{
public:
    typedef ts_container<Container> this_type;
    typedef typename Container::value_type value_type;

    ts_container(size_t capacity = 2048);

    void start();
    void stop();
    void join();

    void capacity(size_t new_capacity);
    size_t capacity() const;

    template <class Rep, class Period>
    void push(const value_type& v,
            const boost::chrono::duration<Rep, Period>& timeout,
            boost::system::error_code& err);

    void push(const value_type& v, boost::system::error_code& err);
    void try_push(const value_type& v, boost::system::error_code& err);

    template <class Rep, class Period>
    bool pop(value_type& v, const boost::chrono::duration<Rep, Period>& timeout);

    void release_all();
    value_type force_pop();

    bool empty() const;
    size_t size() const;

    const statistic::la_values la_stat();

private:
    bool full_i() const { return container_.size() >= capacity_; }
    bool empty_i() const { return container_.empty(); }
private:
    Container container_;
    size_t capacity_;

    statistic::load_average la_stat_;

    mutable boost::mutex mtx_;
    boost::condition_variable r_cond_;
    boost::condition_variable w_cond_;
};

template<class Container>
ts_container<Container>::ts_container(size_t capacity):capacity_(capacity)
{
    container_.ts_context(maq::detail::ts_context(&mtx_));
}
template<class Container>
void ts_container<Container>::start()
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    container_.start();
}

template<class Container>
void ts_container<Container>::stop()
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    container_.stop();
}

template<class Container>
void ts_container<Container>::join()
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    container_.join();
}

template<class Container>
void ts_container<Container>::capacity(size_t new_capacity)
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    capacity_ = new_capacity;
    w_cond_.notify_all();
}

template<class Container>
size_t ts_container<Container>::capacity() const
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    return capacity_;
}

template<class Container>
template <class Rep, class Period>
void ts_container<Container>::push(const value_type& v,
        const boost::chrono::duration<Rep, Period>& timeout,
        boost::system::error_code& err)
{
    boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
    time_gruard_push<const value_type> tgp(v, now);
    boost::chrono::system_clock::time_point tp = now + timeout;

    boost::unique_lock<boost::mutex> lock(mtx_);
    while (full_i())
    {
        if (w_cond_.wait_until(lock, tp) == boost::cv_status::timeout)
        {
            err = error::make_error_code(error::IS_TIMEDOUT);
            return;
        }
    }

    container_.push(v);
    r_cond_.notify_one();
    la_stat_.collect(container_.size(), boost::chrono::system_clock::now());
}

template<class Container>
void ts_container<Container>::push(const value_type& v,
        boost::system::error_code& err)
{
    boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
    time_gruard_push<const value_type> tgp(v, now);

    boost::unique_lock<boost::mutex> lock(mtx_);
    if(full_i())
    {
        err = error::make_error_code(error::IS_FULL);
        return;
    }

    container_.push(v);
    r_cond_.notify_one();
    la_stat_.collect(container_.size(), boost::chrono::system_clock::now());
}

template<class Container>
void ts_container<Container>::try_push(const value_type& v,
        boost::system::error_code& err)
{
    boost::unique_lock<boost::mutex> lock(mtx_, boost::try_to_lock);

    if (!lock.owns_lock())
    {
        err = error::make_error_code(error::IS_BUSY);
        return;
    }
    else if(full_i())
    {
        err = error::make_error_code(error::IS_FULL);
        return;
    }

    boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
    time_gruard_push<const value_type> tgp(v, now);

    container_.push(v);
    r_cond_.notify_one();
    la_stat_.collect(container_.size(), now);
}

template<class Container>
template <class Rep, class Period>
bool ts_container<Container>::pop(value_type& v, const boost::chrono::duration<Rep, Period>& timeout)
{
    boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
    boost::chrono::system_clock::time_point tp = now + timeout;
    boost::unique_lock<boost::mutex> lock(mtx_);
    while (empty_i())
    {
        boost::chrono::system_clock::time_point wait_delay_q =
                boost::chrono::system_clock::time_point::max();

        container_.nearest_delay_time(wait_delay_q);

        if (tp < wait_delay_q)
        {
            if (r_cond_.wait_until(lock, tp) == boost::cv_status::timeout)
                return false;
        }
        else
        {
            r_cond_.wait_until(lock, wait_delay_q);
            container_.check_delayed_queue();
        }
    }

    v = container_.pop();
    w_cond_.notify_one();
    la_stat_.collect(container_.size(), boost::chrono::system_clock::now());
    return true;
}

template<class Container>
void ts_container<Container>::release_all()
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    container_.release_all();
}

template<class Container>
typename ts_container<Container>::value_type ts_container<Container>::force_pop()
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    return container_.pop();
}

template<class Container>
bool ts_container<Container>::empty() const
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    boost::chrono::system_clock::time_point wait_delay_q;
    container_.nearest_delay_time(wait_delay_q);
    return empty_i() && !wait_delay_q.time_since_epoch().count();
}

template<class Container>
size_t ts_container<Container>::size() const
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    return container_.size();
}

template<class Container>
const statistic::la_values ts_container<Container>::la_stat()
{
    boost::unique_lock<boost::mutex> lock(mtx_);
    la_stat_.collect(container_.size());
    return la_stat_.get();
}

}

#endif
