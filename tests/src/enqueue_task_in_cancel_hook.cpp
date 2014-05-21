#include "maq/active_object.h"
#include "maq/container.h"
#include "maq/plugins.h"

#include "maq/init.h"
INIT_ACTIVE_OBJECT();

#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace maq;
using namespace maq::plugins;
using namespace boost::chrono;

std::string cancelled_task;
std::string executed_task;

class task
{
public:
    task(){};
    task(const std::string& name,
            const system_clock::time_point& deadline,
            boost::function<void()> f = boost::function<void()>())
    :name_(name), deadline_(deadline), cancel_hook_(f)
    {}

    void execute()
    { executed_task += name_; }

    void cancel()
    {
        cancelled_task += name_;
        if(cancel_hook_)
            cancel_hook_();
    }

    const system_clock::time_point& deadline() const
    { return deadline_; }

private:
    std::string name_;
    system_clock::time_point deadline_;
    boost::function<void()> cancel_hook_;
};

typedef active_object<container<task, deadline, priority>::type > ao_p;
typedef active_object<container<task, deadline>::type > ao_q;

template <class AO>
void enqueue_task(AO& aq)
{
    aq.enqueue_method(task("task_2", system_clock::now() + milliseconds(100)));
}

template <class AO>
bool test()
{
    AO aq;

    boost::function<void()> fn = boost::bind(enqueue_task<AO>, boost::ref(aq));

    // task_1 will expire immediately
    // and add new task_2
    aq.enqueue_method(task("task_1", system_clock::now(), fn));

    boost::this_thread::sleep_for(milliseconds(200));

    aq.start();
    aq.wait();

    return (cancelled_task == "task_1") && (executed_task == "task_2");
}

int main()
{
    assert(true == test<ao_p>());
    cancelled_task.clear();
    executed_task.clear();
    assert(true == test<ao_q>());
}


