#ifndef _MAQ_ACTIVE_OBJECT_H_
#define _MAQ_ACTIVE_OBJECT_H_

#include <ostream>
#include <vector>

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/chrono.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/tss.hpp> // thread local storage

#include "maq/error.h"
#include "maq/extractor.h"
#include "maq/ts_container.h"
#include "maq/statistic.h"
#include "maq/plugins/disable.h"


namespace maq {

namespace detail
{
struct tls_holder
{
    template <class AO>
    tls_holder(AO* ao):ptr(ao){}

    void* ptr;
};
extern boost::thread_specific_ptr<tls_holder> active_object_tls;
}

template<class Container, class TimeInQueuePlugin = plugins::disable>
class active_object: public boost::noncopyable
{
    typedef typename TimeInQueuePlugin::template bind<void>::type time_in_queue_t;
    typedef typename time_in_queue_t::collector collector_t;

public:
    typedef active_object<Container, TimeInQueuePlugin> this_type;
    typedef typename Container::value_type value_type;
    typedef boost::function<void(active_object*)> thread_decarator;

    static const std::string& type()
    {
        static std::string name_("ao<" + Container::type_name() + ">");
        return name_;
    }

    static active_object* tls_accessor()
    {
        if(!detail::active_object_tls.get())
            return NULL;
        return reinterpret_cast<active_object*>((*detail::active_object_tls).ptr);
    }

    active_object()
        :stop_(false), wait_until_empty_(false)
        ,container_started_(false), started_(false)
    {
        container_started_ = true;
        // Start io_service in deadline_plugin
        container_.start();

    }
    ~active_object() {stop();}

    void enqueue_method(const value_type& v, boost::system::error_code& err)
    {
        if(is_active(err))
            container_.push(v, err);
    }

    void enqueue_method(const value_type& v)
    {
        boost::system::error_code err;
        enqueue_method(v, err);
        throw_if_error(err);
    }

    void enqueue_method(const value_type& v,
            const boost::chrono::milliseconds& timeout,
            boost::system::error_code& err)
    {
        if(is_active(err))
            container_.push(v, timeout, err);
    }

    void enqueue_method(const value_type& v,
            const boost::chrono::milliseconds& timeout)
    {
        boost::system::error_code err;
        enqueue_method(v, timeout, err);
        throw_if_error(err);
    }

    void try_enqueue_method(const value_type& v, boost::system::error_code& err)
    {
        if(is_active(err))
            container_.try_push(v, err);
    }

    void try_enqueue_method(const value_type& v)
    {
        boost::system::error_code err;
        try_enqueue_method(v, err);
        throw_if_error(err);
    }

    void start(size_t count = 1)
    {
        if(started_ && container_started_)
            throw std::runtime_error("active_object have already started");

        pool_.reset(new boost::thread_group());
        stat_.reset(new statistic::bandwidth_stat(count));
        qtime_plugin_.reset(count);
        stop_ = false;
        wait_until_empty_ = false;

        for(size_t i=0; i<count; i++)
            pool_->add_thread(new boost::thread(&this_type::run, this,
                    boost::ref(stat_->get_counter(i)),
                    boost::ref(qtime_plugin_.get_collector(i))
            ));
        if(!container_started_)
            container_.start();

        started_ = container_started_ = true;
    }

    void stop()
    {
        stop_ = true;
        if(pool_)
            pool_->join_all();
        container_.stop();
        container_.join();

        started_ = container_started_ = false;
    }

    void wait()
    {
        wait_until_empty_ = true;
        if(pool_)
            pool_->join_all();
        container_.stop();
        container_.join();

        started_ = container_started_ = false;
    }

    void add_thread_decarator(thread_decarator decorator)
    {
        decarator_.push_back(decorator);
    }

    void name(const std::string& name)
    {
        name_ = name;
    }

    const std::string& name()
    {
        return name_;
    }

    void capacity(size_t new_capacity)
    {
        container_.capacity(new_capacity);
    }

    size_t capacity() const
    {
        return container_.capacity();
    }

    size_t q_size() const
    {
        return container_.size();
    }

    statistic::la_values la_stat()
    {
        return container_.la_stat();
    }

    const statistic::bandwidth_stat& bandwidth_stat() const
    {
        return *stat_;
    }

    // This method will reset statistics
    boost::chrono::system_clock::duration time_in_queue_avg()
    {
        return qtime_plugin_.average();
    }

private:

    bool is_active(boost::system::error_code& err)
    {
        if(stop_ || wait_until_empty_)
        {
            err = error::make_error_code(error::IS_INACTIVE);
            return false;
        }
        return true;
    }

    void throw_if_error(const boost::system::error_code& err)
    {
        if(err)
            throw boost::system::system_error(err);
    }

    void run(statistic::bandwidth_stat::counter& counter, collector_t& cl)
    {
        detail::active_object_tls.reset(new detail::tls_holder(this));

        for(size_t i=0; i<decarator_.size(); i++)
        {
            try
            { decarator_[i](this); }
            catch(const std::exception&)
            { /* keep silent */ }
        }

        counter.reset();
        try
        {
            bool exit_because_empty = wait_until_empty_ && container_.empty();
            while(!stop_ && !exit_because_empty)
            {
                value_type v;
                if(container_.pop(v, boost::chrono::milliseconds(1000)))
                {
                    cl.collect(v);
                    try{ execute_hook(v); } catch(...) {/*bad user hook*/}
                    counter.inc();
                }
                else
                {/*timeout*/}

                exit_because_empty = wait_until_empty_ && container_.empty();
            }
        }
        catch(...){/*something unexpected*/}

        cancel_all();
    }

    void cancel_all()
    {
        container_.release_all();
        while(!container_.empty())
        {
            value_type v = container_.force_pop();
            try{ cancel_hook(v); } catch(...) {/*bad user hook*/}
        }

    }

private:
    ts_container<Container> container_;
    boost::scoped_ptr<boost::thread_group> pool_;

    boost::atomic<bool> stop_;
    boost::atomic<bool> wait_until_empty_;

    bool container_started_;
    bool started_;

    boost::scoped_ptr<statistic::bandwidth_stat> stat_;
    time_in_queue_t qtime_plugin_;

    std::string name_;
    std::vector<thread_decarator> decarator_;
};

}

#endif /* _MAQ_ACTIVE_OBJECT_H_ */
