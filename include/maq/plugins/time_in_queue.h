#ifndef _MAQ_PLUGINS_TIME_IN_QUEUE_H_
#define _MAQ_PLUGINS_TIME_IN_QUEUE_H_

#include <list>
#include <vector>
#include <numeric>
#include <boost/chrono.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/mutex.hpp>
#include "maq/extractor.h"


namespace maq {
namespace plugins {

namespace detail {

// Bad implementation only for tests
template <typename T>
const boost::chrono::system_clock::duration& time_in_queue (const T&,
        typename boost::disable_if<maq::detail::is_a_stat<T> >::type* _ = 0)
{
    assert(!_);
    static boost::chrono::system_clock::duration dur;
    return dur;
}

template<typename T>
boost::chrono::system_clock::duration time_in_queue(const T& t,
        typename boost::enable_if<maq::detail::is_a_stat<T> >::type* _ = 0)
{
    assert(!_);
    return boost::chrono::system_clock::now() - t.stat().push.first;
}

template <class U>
class time_in_queue_plugin
{
public:
    struct collector
    {
        collector(): mtx(boost::make_shared<boost::mutex>())
        {}

        template <typename T>
        void collect(const T& t)
        {
            static boost::chrono::system_clock::duration zero =
                    boost::chrono::system_clock::duration::zero();

            boost::chrono::system_clock::duration dur;
            dur = time_in_queue(t);
            if(dur != zero)
            {
                // do some logic
                boost::mutex::scoped_lock lock(*mtx);
                durs_.push_back(dur);
            }
        };

        boost::shared_ptr<boost::mutex> mtx;
        std::list<boost::chrono::system_clock::duration> durs_;
    };
    typedef std::vector<collector> collectors;

public:

    void reset(size_t size)
    {
        collectors_.clear();
        collectors tmp;
        tmp.resize(size);
        collectors_.swap(tmp);
    };

    collector& get_collector(size_t indx)
    {
        assert(indx < collectors_.size());
        return collectors_[indx];
    }

    boost::chrono::system_clock::duration average()
    {
        boost::chrono::system_clock::duration dur;
        if(!collectors_.size())
            return dur;

        for(size_t i=0; i<collectors_.size(); i++)
        {
            std::list<boost::chrono::system_clock::duration> durs;
            {
                boost::mutex::scoped_lock lock(*collectors_[i].mtx);
                durs.swap(collectors_[i].durs_);
            }

            if( durs.size())
            {
                boost::chrono::system_clock::duration dur_l;
                dur += (std::accumulate(durs.begin(), durs.end(), dur_l) / durs.size());
            }
        }

        return dur / collectors_.size();
    }

private:
    collectors collectors_;
};

}

#if 0

struct time_in_queue
{
    template <class T>
    struct bind
    {
        typedef detail::time_in_queue_plugin<T> type;
    };
};

#endif

}}

#endif /* TIME_IN_QUEUE_PLUGIN_H_ */
