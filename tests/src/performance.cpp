#include <fstream>
#include "environment.h"
#include "test_templates.h"

#include "maq/active_object.h"
#include "maq/container.h"

#include "maq/stat_holder.h"
#include "maq/cancellable_holder.h"

#include "maq/plugins.h"

/* measurement 'Bandwidth' and 'Time spent in Queue' for different configurations
 * 1. No plugins
 * 2. Deferred plugin
 * 3. Deadline plugin
 * 4. Deferred + Deadline plugins
 */

#if 0

struct bandwidth_stat_data
{
    size_t bandwidth;
    statistic::la_values la;
    boost::chrono::milliseconds total;
};
struct time_in_q_stat_data
{
    boost::chrono::milliseconds time;
    statistic::la_values la;
    boost::chrono::milliseconds total;
};

typedef std::vector<std::pair<std::string, bandwidth_stat_data> > bandwidth_map;
typedef std::vector<std::pair<std::string, time_in_q_stat_data> > time_in_q_map;

struct bandwidth_extractor
{
    typedef bandwidth_stat_data ret_type;

    template <typename AO>
    ret_type operator()(AO& ao, boost::chrono::system_clock::time_point start)
    {
        using namespace boost::chrono;
        milliseconds total = duration_cast<milliseconds>(system_clock::now() - start);
        return ret_type({ao.bandwidth_stat().bandwidth_per_sec(), ao.la_stat(), total});
    }
};

struct time_in_q_extractor
{
    typedef time_in_q_stat_data ret_type;

    template <typename AO>
    ret_type operator()(AO& ao, boost::chrono::system_clock::time_point start)
    {
        using namespace boost::chrono;
        milliseconds dur = duration_cast<milliseconds>(ao.time_in_queue_avg());
        milliseconds total = duration_cast<milliseconds>(system_clock::now() - start);
        return ret_type({dur, ao.la_stat(), total});
    }
};

class dtask
{};
typedef stat_holder<dtask> dtask_stat;

template <class T>
struct task_creator;

template <>
struct task_creator<dtask>
{
    dtask operator()()
    { return dtask(); }
};

template <>
struct task_creator<dtask_stat>
{
    dtask_stat operator()()
    { return stat_holder<dtask>(dtask()); }
};

template <class AO>
void task_pusher(AO& p, int produce)
{
    typedef typename AO::value_type value_type;
    int i = 0;
    while(i<produce)
        if(p.enqueue_method(task_creator<value_type>()()))
            i++;
}

template <class AO>
void run_bandwidth(AO& ao, size_t produce, size_t producer, size_t consumer)
{
    ao.start(consumer);

    boost::thread_group pool;
    for(size_t i=0; i<producer; i++)
        pool.add_thread(new boost::thread(
                        task_pusher<AO>, boost::ref(ao), produce / producer));

    pool.join_all();
    ao.wait();
}

template <class Container, class AOPlugin, class ResultExtractor>
typename ResultExtractor::ret_type get_bandwidth(size_t produce, size_t producer, size_t consumer)
{
    active_object<Container, AOPlugin> ao;
    boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();
    run_bandwidth(ao, produce, producer, consumer);
    return ResultExtractor()(ao, start);
}

template <class Priority, class AOPlugin, class Task, class ResultExtractor, class ResultList>
bandwidth_map collect_bandwidth(size_t produce, size_t producer, size_t consumer,
        ResultList& list)
{
    bandwidth_map result;

    { typedef typename container<Task, Priority>::type con;
    list.push_back(std::make_pair("non plugin",
            get_bandwidth<con, AOPlugin, ResultExtractor>(produce, producer, consumer))); }
    sleep(1);

    { typedef typename container<Task, Priority, deferred>::type con;
    list.push_back(std::make_pair("defer",
            get_bandwidth<con, AOPlugin, ResultExtractor>(produce, producer, consumer))); }
    sleep(1);

    { typedef typename container<Task, Priority, deadline>::type con;
    list.push_back(std::make_pair("deadline",
            get_bandwidth<con, AOPlugin, ResultExtractor>(produce, producer, consumer))); }
    sleep(1);

    { typedef typename container<Task, Priority, deferred, deferred>::type con;
    list.push_back(std::make_pair("defer+deadline",
            get_bandwidth<con, AOPlugin, ResultExtractor>(produce, producer, consumer))); }
    sleep(1);

    return result;
}

template <class TimePlugin>
void measurement_bandwidth(size_t produce, size_t producer, size_t consumer)
{
    bandwidth_map q_res, prio_res;

    collect_bandwidth<disable, TimePlugin, dtask, bandwidth_extractor>(produce, producer, consumer, q_res);
    collect_bandwidth<priority, TimePlugin, dtask, bandwidth_extractor>(produce, producer, consumer, prio_res);

    std::stringstream out;
    out << std::setw(15) << "Configuration"
        << std::setw(10) << "Queue"
        << std::setw(10) << "Prio"
        << std::setw(10) << "Q 5sec"
        << std::setw(10) << "P 5sec"
        << std::setw(10) << "Q 10sec"
        << std::setw(10) << "P 10sec"
        << std::setw(15) << "Q total(ms)"
        << std::setw(15) << "P total(ms)"
        << std::endl;
    for(size_t i=0; i<q_res.size(); i++)
    {
        out << std::setw(15) << q_res[i].first
            << std::setw(10) << q_res[i].second.bandwidth
            << std::setw(10) << prio_res[i].second.bandwidth
            << std::setw(10) << q_res[i].second.la[0].value
            << std::setw(10) << prio_res[i].second.la[0].value
            << std::setw(10) << q_res[i].second.la[1].value
            << std::setw(10) << prio_res[i].second.la[1].value
            << std::setw(15) << q_res[i].second.total.count()
            << std::setw(15) << prio_res[i].second.total.count()
            << std::endl;
    }

    std::cout << out.str();
}

void measurement_time_in_q(size_t produce, size_t producer, size_t consumer)
{
    time_in_q_map q_res, prio_res;

    collect_bandwidth<disable, time_in_queue, dtask_stat, time_in_q_extractor>(produce, producer, consumer, q_res);
    collect_bandwidth<priority, time_in_queue, dtask_stat, time_in_q_extractor>(produce, producer, consumer, prio_res);

    std::stringstream out;
    out << std::setw(15) << "Configuration"
        << std::setw(10) << "Queue"
        << std::setw(10) << "Prio"
        << std::setw(10) << "Q 5sec"
        << std::setw(10) << "P 5sec"
        << std::setw(10) << "Q 10sec"
        << std::setw(10) << "P 10sec"
        << std::setw(15) << "Q total(ms)"
        << std::setw(15) << "P total(ms)"
        << std::endl;
    for(size_t i=0; i<q_res.size(); i++)
    {
        out << std::setw(15) << q_res[i].first
            << std::setw(10) << q_res[i].second.time.count()
            << std::setw(10) << prio_res[i].second.time.count()
            << std::setw(10) << q_res[i].second.la[0].value
            << std::setw(10) << prio_res[i].second.la[0].value
            << std::setw(10) << q_res[i].second.la[1].value
            << std::setw(10) << prio_res[i].second.la[1].value
            << std::setw(15) << q_res[i].second.total.count()
            << std::setw(15) << prio_res[i].second.total.count()
            << std::endl;
    }

    std::cout << out.str();
}

int main()
{
    std::cout << "Not stat:\n";
    measurement_bandwidth<disable>(2000000, 2, 2);

    std::cout << "\nWith stat:\n";
    measurement_bandwidth<time_in_queue>(2000000, 2, 2);

    std::cout << "\nTime in q (milisec):\n";
    measurement_time_in_q(2000000, 2, 2);
}

#else
int main()
{
}
#endif




