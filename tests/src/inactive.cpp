#include "environment.h"

bool expect_exception(ao_pfull& pool)
{
    boost::system::error_code err;
    pool.enqueue_method(task(), err);
    return err == error::IS_INACTIVE;
}

int main()
{
    ao_pfull pool;

    pool.start();
    assert(!expect_exception(pool));
    pool.stop();
    assert(expect_exception(pool));

    pool.start();
    assert(!expect_exception(pool));
    pool.wait();
    assert(expect_exception(pool));
}





