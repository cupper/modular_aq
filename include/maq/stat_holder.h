#ifndef _MAQ_STAT_HOLDER_H_
#define _MAQ_STAT_HOLDER_H_

#include <boost/chrono.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/shared_ptr.hpp>


namespace maq {

typedef std::pair<boost::chrono::system_clock::time_point, boost::chrono::system_clock::time_point> se_time;
struct stat_t
{
    se_time push;
    se_time execute;
    se_time cancel;

    boost::chrono::milliseconds full_time() const
    {
        if (!execute.first.time_since_epoch().count())
            return boost::chrono::duration_cast<boost::chrono::milliseconds>(cancel.second - push.first);
        return  boost::chrono::duration_cast<boost::chrono::milliseconds>(execute.second - push.first);
    }
};

class time_guard
{
public:
    time_guard(se_time& t):t_(t) { t_.first = boost::chrono::system_clock::now(); }
    ~time_guard() { t_.second = boost::chrono::system_clock::now(); }
private:
    se_time& t_;
};

template<class T>
class stat_holder
{
public:
    typedef T value_type;

    stat_holder(){};
    stat_holder(const T& t):t_(t), stat_(new stat_t()) {}
    stat_t& stat() { return *stat_; }
    const stat_t& stat() const { return *stat_; }
    T& get() { return t_; }
    T const& get() const { return t_; }

    void enter_push(const boost::chrono::system_clock::time_point& now) const
    { stat_->push.first = now; }
    void exit_push(const boost::chrono::system_clock::time_point& now) const
    { stat_->push.second = now; }

private:
    T t_;
    mutable boost::shared_ptr<stat_t> stat_;
};

template<class T>
class time_gruard_push
{
public:
    time_gruard_push(T const&, const boost::chrono::system_clock::time_point&){};
};

template<class T>
class time_gruard_push<stat_holder<T> const>
{
public:
    time_gruard_push(stat_holder<T> const& stat)
        :stat_(stat)
    { stat_.enter_push(boost::chrono::system_clock::now());}
    time_gruard_push(stat_holder<T> const& stat, const boost::chrono::system_clock::time_point& now)
            :stat_(stat)
    { stat_.enter_push(now);}
    ~time_gruard_push(){ stat_.exit_push(boost::chrono::system_clock::now());}
private:
    const stat_holder<T>& stat_;
};

}

#endif
