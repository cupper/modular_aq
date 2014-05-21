#include "maq/active_object.h"
#include "maq/container.h"

#include "maq/init.h"
INIT_ACTIVE_OBJECT();

class task
{
public:
    task(){};
};

using namespace maq;
using namespace maq::plugins;

typedef active_object<container<task, priority>::type> ao_t;

int main()
{
    ao_t aq;

    aq.capacity(5);
    for(int i=0; i<5; i++)
        aq.enqueue_method(task());

    // full
    boost::system::error_code err;
    aq.enqueue_method(task(), boost::chrono::milliseconds(1), err);
    assert(err);

    aq.capacity(6);
    // and now can
    err.clear();
    aq.enqueue_method(task(), boost::chrono::milliseconds(1), err);
    assert(!err);
}
