#include "maq/active_object.h"
#include "maq/container.h"
#include "maq/plugins.h"

#include "maq/init.h"
INIT_ACTIVE_OBJECT();

using namespace maq;
using namespace maq::plugins;
using namespace boost::chrono;

system_clock::time_point was_executed_in = system_clock::time_point::min();

class task
{
public:
    task(){};
    task(const system_clock::time_point& t):t_(t) {}

    void execute()
    { was_executed_in = system_clock::now(); }

    const system_clock::time_point& delay() const
    { return t_; }

private:
    system_clock::time_point t_;
};

typedef active_object<container<task, deferred, priority>::type > ao_p;
typedef active_object<container<task, deferred>::type > ao_q;

template <class AO>
bool test()
{
    system_clock::time_point start_in = system_clock::now();
    task t(start_in + milliseconds(500));

    AO ao;
    ao.start();
    ao.enqueue_method(t);
    ao.wait();

    milliseconds dur = duration_cast<boost::chrono::milliseconds>(was_executed_in - start_in);
    return dur >= milliseconds(500);
}

int main()
{
    assert(true == test<ao_p>());
    was_executed_in = system_clock::time_point::min();
    assert(true == test<ao_q>());
}



