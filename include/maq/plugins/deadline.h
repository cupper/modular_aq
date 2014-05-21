#ifndef _MAQ_PLUGINS_DEADLINE_H_
#define _MAQ_PLUGINS_DEADLINE_H_

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/chrono.hpp>

#include "maq/extractor.h"
#include "maq/detail/ts_context.h"


namespace maq {
namespace plugins {

namespace detail {

template<class T>
class deadline_plugin
{
public:
    static_type("deadline");

    deadline_plugin():
        deadline_(boost::chrono::system_clock::time_point::max()),
        timer_(service_),
        keep_io_service_(new boost::asio::io_service::work(service_))
    {
    }

    bool preprocessing(const T& t) {
        if(extract_deadline(t) < deadline_)
            set_deadline(extract_deadline(t));

        return false;
    }

    template<class Container>
    bool postprocessing(Container& c)
    {
        if(!c.empty())
        {
            const boost::chrono::system_clock::time_point& deadline = extract_deadline(*c.begin());
            if(deadline != boost::chrono::system_clock::time_point::max())
                set_deadline(deadline);
        }
        return false;
    }

    bool empty() const { return true; }

    template<class F>
    void ts_context(const maq::detail::ts_context& s, const F& f) { strand_ = s; extraxtor_ = f; }

    void run_service() { service_.run(); }

    void set_deadline(const boost::chrono::system_clock::time_point& t)
    {
        deadline_ = t;
        timer_.expires_at(deadline_);
        timer_.async_wait(
                boost::bind(&deadline_plugin::timer_handler, this,
                        boost::asio::placeholders::error));
    }

    void timer_handler(const boost::system::error_code& error)
    {
        typedef typename std::list<T>::iterator iterator;
        if (!error)
        {
            std::list<T> tasks;
            strand_(boost::bind(extraxtor_, boost::ref(tasks)));
            iterator end = tasks.end();
            for(iterator it = tasks.begin(); it != end; it++)
            {
                cancel_hook(*it);
            }
        }
        else if (error != boost::asio::error::operation_aborted)
        {
        }
    }

    bool is_expired(const T& t)
    {
        return extract_deadline(t) < boost::chrono::system_clock::now();
    }

    void start()
    {
        th_ = boost::thread(&deadline_plugin<T>::run_service, this);
    }

    void stop()
    {
        keep_io_service_.reset();
        timer_.cancel();
    }

    void join()
    {
        if(th_.joinable())
            th_.join();
    };

private:
    maq::detail::ts_context strand_;
    boost::function<void (std::list<T>&)> extraxtor_;

    boost::chrono::system_clock::time_point deadline_;
    boost::thread th_;
    boost::asio::io_service service_;
    boost::asio::basic_waitable_timer<boost::chrono::system_clock> timer_;
    std::auto_ptr<boost::asio::io_service::work> keep_io_service_;
};

}

struct deadline
{
    template <class T>
    struct bind
    {
        typedef detail::deadline_plugin<T> type;
    };
};

}}

#endif
