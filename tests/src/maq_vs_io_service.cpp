#include "maq/active_object.h"
#include "maq/container.h"

#include <boost/asio.hpp>
#include <stdlib.h>

#include "maq/init.h"
INIT_ACTIVE_OBJECT();

using namespace maq;
using namespace maq::plugins;

class task
{
public:
    // for aq
    void execute()
    { }

    // for io_service
    void operator()()
    { }
};
typedef active_object<container<task>::type> aq_t;

typedef boost::chrono::milliseconds millisec;
typedef boost::chrono::microseconds microsec;
typedef boost::chrono::nanoseconds nanosec;


class timer
{
public:
    timer():start(boost::chrono::system_clock::system_clock::now()){}

    template <typename Duration>
    Duration get()
    {
        if(end.time_since_epoch() == boost::chrono::system_clock::duration::zero())
        {
            return boost::chrono::duration_cast<Duration>(
                    boost::chrono::system_clock::system_clock::now() - start);
        }

        return boost::chrono::duration_cast<Duration>(end - start);
    }

    void restart()
    {
        end = boost::chrono::system_clock::time_point();
        start = boost::chrono::system_clock::system_clock::now();
    }

    void stop()
    {
        end = boost::chrono::system_clock::system_clock::now();
    }

private:
    boost::chrono::system_clock::time_point start;
    boost::chrono::system_clock::time_point end;
};

struct aq_pusher
{
    aq_pusher()
    {
        aq_.capacity(99999999999);
    }
    void push()
    {
        aq_.enqueue_method(t_);
    }

    aq_t aq_;
    task t_;
};

struct io_service_pusher
{
    void push()
    {
        io_.post(t_);
    }

    boost::asio::io_service io_;
    task t_;
};

struct io_strand_pusher
{
    io_strand_pusher():strand_(io_)
    {}

    void push()
    {
        strand_.post(t_);
    }

    boost::asio::io_service io_;
    boost::asio::io_service::strand strand_;
    task t_;
};


template <typename Queue>
void push(Queue& queue, size_t n, timer& t)
{
    t.restart();
    for(size_t i=0; i<n; i++)
        queue.push();
    t.stop();
}

template <typename Duration, typename Queue>
Duration concurrent_push(size_t th, size_t n)
{
    boost::thread_group group;

    std::vector<timer> timers(th);
    Queue queue;

    for(size_t i=0; i<th; i++)
    {
        group.create_thread(boost::bind(
                push<Queue>, boost::ref(queue), n, boost::ref(timers[i])));
    }

    group.join_all();

    Duration total;
    for(size_t i=0; i<th; i++)
        total += timers[i].get<Duration>();

    return total;
}

void test_push(size_t n)
{
    timer t;
    io_service_pusher io_service;
    push(io_service, n, t);
    millisec ioser_mls = t.get<millisec>();

    sleep(5);
    aq_pusher aq;
    push(aq, n, t);
    millisec aq_mls = t.get<millisec>();

    sleep(5);
    io_strand_pusher io_strand;
    push(io_strand, n, t);
    millisec iostr_mls = t.get<millisec>();

    std::cout << "active_queue = " << aq_mls << std::endl
              << "io_service   = " << ioser_mls << std::endl
              << "io_strand    = " << iostr_mls << std::endl;
}

void test_cuncurrent_push(size_t th, size_t n)
{
    millisec ioser_mls = concurrent_push<millisec, io_service_pusher>(th, n);
    sleep(5);
    millisec aq_mls = concurrent_push<millisec, aq_pusher>(th, n);
    sleep(5);
    millisec iostr_mls = concurrent_push<millisec, io_strand_pusher>(th, n);

    std::cout << "active_queue = " << aq_mls << std::endl
              << "io_service   = " << ioser_mls << std::endl
              << "io_strand    = " << iostr_mls << std::endl;
}


int main(int argc, const char* argv[])
{
    enum TEST {PUSH, CONCURRENT_PUSH, NONE};

    TEST test = NONE;
    size_t th = 0;
    size_t n = 0;

    if(argc >= 3)
    {
        test = CONCURRENT_PUSH;
        th = atoi(argv[1]);
        n = atoi(argv[2]);
    }
    else if(argc >= 2)
    {
        test = PUSH;
        n = atoi(argv[1]);
    }

    switch(test)
    {
    case PUSH:
        std::cout << "Start PUSH test, n = " << n << std::endl;
        test_push(n);
        break;
    case CONCURRENT_PUSH:
        std::cout << "Start CONCURRENT_PUSH test, th = "
                  << th << "n = " << n << std::endl;
        test_cuncurrent_push(th, n);
        break;

    default:
        std::cerr << "You do not specify type of test\n";
    }

}

/* OUTPUT (release):

Start PUSH test, n = 9000000
active_queue = 1113 milliseconds
io_service   = 498 milliseconds
io_strand    = 380 milliseconds

Start CONCURRENT_PUSH test, th = 10n = 1000000
active_queue = 40976 milliseconds
io_service   = 43219 milliseconds
io_strand    = 22766 milliseconds

Start CONCURRENT_PUSH test, th = 10n = 2000000
active_queue = 78602 milliseconds
io_service   = 66920 milliseconds
io_strand    = 42301 milliseconds

if load_average is commented

Start PUSH test, n = 9000000
active_queue = 478 milliseconds
io_service   = 498 milliseconds
io_strand    = 380 milliseconds

Start CONCURRENT_PUSH test, th = 10n = 1000000
active_queue = 28736 milliseconds
io_service   = 20082 milliseconds
io_strand    = 22657 milliseconds

Start CONCURRENT_PUSH test, th = 10n = 2000000
active_queue = 62228 milliseconds
io_service   = 38836 milliseconds
io_strand    = 43335 milliseconds

Start CONCURRENT_PUSH test, th = 10n = 2000000
active_queue = 66485 milliseconds
io_service   = 68901 milliseconds
io_strand    = 44583 milliseconds

 */

