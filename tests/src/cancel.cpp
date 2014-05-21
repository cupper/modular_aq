#include "maq/active_object.h"
#include "maq/container.h"
#include "maq/cancellable_holder.h"

#include "maq/init.h"
INIT_ACTIVE_OBJECT();

using namespace maq;
using namespace maq::plugins;


bool was_executed = false;
bool was_cancelled = false;

class task
{
public:
    void execute()
    { was_executed = true; }

    void cancel()
    { was_cancelled = true; }
};


typedef cancellable_holder<task> holder;
typedef active_object<container<holder::proxy, priority>::type> ao_t;

int main()
{
    task t;
    holder h(t);

    ao_t ao;
    ao.enqueue_method(h.get_proxy());
    h.cancel();

    ao.start();

    assert(was_cancelled == true);
    assert(was_executed == false);
}
