#ifndef _MAQ_CANCELLABLE_HOLDER_H_
#define _MAQ_CANCELLABLE_HOLDER_H_

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>

#include "maq/extractor.h"


namespace maq {

template<class T>
class cancellable_proxy
{
    template<class U>
    struct impl : public boost::noncopyable
    {
        impl(T const& t): t_(t), processed_(0) {}

        U t_;
        boost::atomic<bool> processed_;
    };

public:
    typedef T value_type;

    cancellable_proxy(){}
    cancellable_proxy(T const& t):impl_(new impl<T>(t)){}

    bool lock()
    {
        bool _lock = 0;
        return impl_->processed_.compare_exchange_strong(_lock, 1);
    }

    T& get() {return impl_->t_;}
    T const& get() const {return impl_->t_;}

private:
    boost::shared_ptr<impl<T> > impl_;
};

template<class T>
class cancellable_holder
{
public:
    typedef cancellable_proxy<T> proxy;

public:
    cancellable_holder(T const& t):proxy_(t){}
    cancellable_holder(cancellable_proxy<T> const& p):proxy_(p){}

    cancellable_proxy<T> get_proxy() { return proxy_; }

    void cancel()
    {
        cancel_hook(proxy_);
    }

private:
    cancellable_proxy<T> proxy_;
};

}

#endif
