#ifndef _MAQ_STATISTIC_H_
#define _MAQ_STATISTIC_H_

#include <vector>
#include <deque>
#include <numeric>
#include <boost/thread/mutex.hpp>
#include <boost/atomic.hpp>
#include <boost/chrono.hpp>


namespace maq {
namespace statistic {

class bandwidth_stat
{
public:
    struct counter
    {
        // boost::atomic<T> does not allow copy constructor
        counter() {/* fake for vector */}
        counter(const counter&) {/* fake for vector */}

        void reset()
        {
            count = 0;
            started = boost::chrono::system_clock::now();
        }

        void inc() { ++count; }

        boost::chrono::system_clock::time_point started;
        boost::atomic<uint64_t> count;
    };
    typedef std::vector<counter> counters;

    struct measure
    {
        measure(const counter& c)
        {
            count = c.count;
            duration = boost::chrono::system_clock::now() - c.started;
        }

        boost::chrono::system_clock::duration duration;
        uint64_t count;
    };
    typedef std::vector<measure> measures;

public:
    bandwidth_stat(size_t num_th):counters_(num_th)
    {}

    measures get() const
    {
        return measures(counters_.begin(), counters_.end());
    }

    std::vector<size_t> bandwidth_per_thread_per_sec() const
    {
        std::vector<size_t> res;

        measures mas = get();
        measures::iterator it = mas.begin();
        for(; it!= mas.end(); it++)
        {
            measure& m = *it;
            boost::chrono::milliseconds dur =
                    boost::chrono::duration_cast<boost::chrono::milliseconds>(m.duration);
            res.push_back(static_cast<size_t>(
                    static_cast<double>(m.count) / (static_cast<double>(dur.count()) / 1000.0)));
        }
        return res;
    }

    size_t bandwidth_per_sec() const
    {
        std::vector<size_t> mas = bandwidth_per_thread_per_sec();
        return std::accumulate(mas.begin(), mas.end(), 0) / mas.size();
    }

    counter& get_counter(size_t i) { return counters_[i]; }

private:
    counters counters_;
};

struct ma_value
{
    ma_value():value(0){}
    ma_value(const double& v, boost::chrono::system_clock::time_point& t)
        : value(v), time(t) {}

    double value;
    boost::chrono::system_clock::time_point time;
};
typedef std::deque<ma_value> ma_values;

class exponential_moving_average
{
public:
    exponential_moving_average(
            const size_t& s = 900, // 15 minutes if interval is 1 second
            const boost::chrono::milliseconds& i = boost::chrono::milliseconds(1000),
            const double& al = 0)
        : size_(s)
        , alpha_( al ? al : 2 / (static_cast<double>(s) + 1))
        , interval_ (i)
    {}

    void collect(const size_t& s)
    { collect(s, boost::chrono::system_clock::now()); }

    void collect(const size_t& s, const boost::chrono::system_clock::time_point& now)
    {
        last_.value = static_cast<double>(s) * alpha_ + (1 - alpha_) * last_.value;
        if((now - last_.time) >= interval_)
        {
            last_.time = now;
            stat_.push_back(last_);
            if(stat_.size() > size_)
                stat_.pop_front();
        }
    }

    const ma_values get() const { return stat_; };

private:
    ma_values stat_;
    ma_value last_;

    const size_t size_;
    const double alpha_;
    const boost::chrono::milliseconds interval_;
};

struct la_value
{
    la_value(const boost::chrono::milliseconds& i):value(0), interval(i){}

    double value;
    boost::chrono::milliseconds interval;
};
typedef std::vector<la_value> la_values;

class load_average
{
public:
    load_average()
    {
        //add_interval(boost::chrono::milliseconds(1000));
        add_interval(boost::chrono::milliseconds(5000));
        add_interval(boost::chrono::milliseconds(10000));
    }

    void add_interval(const boost::chrono::milliseconds& interval)
    {
        stat_.push_back(la_value(interval));
    }

    void collect(const size_t& s)
    { collect(s, boost::chrono::system_clock::now()); }

    void collect(const size_t& s, const boost::chrono::system_clock::time_point& now)
    {
        typedef boost::chrono::duration<double, boost::milli> dmilliseconds;

        dmilliseconds rhs_durr = now - last_time_;

        la_values::iterator it = stat_.begin();
        la_values::iterator end = stat_.end();
        for(; it != end; it++)
        {
            la_value& la = *it;
            if(rhs_durr > la.interval)
                rhs_durr = la.interval;
            dmilliseconds lhs_durr = la.interval - rhs_durr;

            la.value = (lhs_durr.count() * la.value + static_cast<double>(s) * rhs_durr.count()) / static_cast<double>(la.interval.count());
            last_time_ = now;
        }
    }

    const la_values get() const { return stat_; };

private:
    la_values stat_;

    boost::chrono::system_clock::time_point last_time_;
};

}}

#endif /* _MAQ_STATISTIC_H_ */
