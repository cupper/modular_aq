#include "maq/active_object.h"
#include "maq/container.h"
#include "maq/plugins.h"

#include "maq/init.h"
INIT_ACTIVE_OBJECT();

using namespace maq;
using namespace maq::plugins;
using namespace boost::chrono;

std::string cancelled_task;

class task
{
public:
    task(){};
    task(const std::string& name,
            const milliseconds& work_for,
            const system_clock::time_point& deadline)
    :name_(name), work_for_(work_for), deadline_(deadline)
    {}

    void execute()
    { boost::this_thread::sleep_for(work_for_); }

    void cancel()
    { cancelled_task += name_; }

    const system_clock::time_point& deadline() const
    { return deadline_; }

private:
    std::string name_;
    milliseconds work_for_;
    system_clock::time_point deadline_;
};


typedef active_object<container<task, deadline, priority>::type > ao_p;
typedef active_object<container<task, deadline>::type > ao_q;

template <class AO>
bool test()
{
    AO ao;
    ao.enqueue_method(task("task_1", milliseconds(200), system_clock::time_point::max()));
    ao.enqueue_method(task("task_2", milliseconds(0), system_clock::now() + milliseconds(100)));

    ao.start();
    ao.wait();

    return cancelled_task == "task_2";
}

int main()
{
    assert(true == test<ao_p>());
    cancelled_task.clear();
    assert(true == test<ao_q>());
}


