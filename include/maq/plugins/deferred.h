#ifndef _MAQ_PLUGINS_DEFERRED_H_
#define _MAQ_PLUGINS_DEFERRED_H_

#include <set>
#include <boost/chrono.hpp>
#include "maq/extractor.h"


namespace maq {
namespace plugins {

namespace  detail {

template<class T>
class deferred_plugin
{
public:
    template<class U>
    struct less_comp
    {
        bool operator()(const U& lhs, const U& rhs) const
        {
            return extract_delay(lhs) < extract_delay(rhs);
        }
    };
    typedef std::multiset<T, less_comp<T> > set_t;

public:
    static_type("deferred");

    bool preprocessing(const T& t)
    {
        if(extract_delay(t) < boost::chrono::system_clock::now())
            return false;

        set_.insert(t);
        return true;
    }

    template<class Container>
    bool postprocessing(Container& c)
    {
        if(!set_.empty())
        {
            boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
            typename set_t::iterator it = set_.begin();
            while((it != set_.end()) && (extract_delay(*it) <= now))
                ++it;

            c.insert(set_.begin(), it);
            set_.erase(set_.begin(), it);
        }
        // not used
        return false;
    }

    template<class Container>
    void release_all(Container& c)
    {
        if(!set_.empty())
        {
            c.insert(set_.begin(), set_.end());
            set_.clear();
        }
    }

    bool empty() const { return set_.empty(); }

    void nearest_time(boost::chrono::system_clock::time_point& tp) const
    {
        if(!set_.empty())
        {
            typename set_t::iterator it = set_.begin();
            tp = extract_delay(*it);
        }
    }

    // Unused methods
    template<class S, class F>
    void ts_context(S&, const F&) {}

    bool is_expired(const T&) {return false;}
    void start(){};
    void stop(){};
    void join(){};

private:
    set_t set_;
};

}

struct deferred
{
    template <class T>
    struct bind
    {
        typedef detail::deferred_plugin<T> type;
    };
};

}}

#endif
