#include "maq/active_object.h"
#include "maq/container.h"

#include "maq/init.h"
INIT_ACTIVE_OBJECT();

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

using namespace maq;
using namespace maq::plugins;

bool is_rigth = false;

class task;
typedef active_object<container<task>::type> ao_t;

struct ao_ptr_holder
{
    ao_ptr_holder(ao_t* p):ptr(p){}
    ao_t* ptr;
};
boost::thread_specific_ptr<ao_ptr_holder> ao_tls_accessor;

struct task
{
    task():ptr_(NULL){}
    task(ao_t* ptr):ptr_(ptr){}

    void execute()
    {
        if(!ptr_)
        {
            ao_t* ao = (*ao_tls_accessor).ptr;
            ao->enqueue_method(task((*ao_tls_accessor).ptr));
        }
        else
        {
            assert(ptr_ == (*ao_tls_accessor).ptr);
        }
    }

    ao_t* ptr_;
};

void th_decorator(ao_t* ptr)
{
    //std::cout << "decorator was called: " << ptr << std::endl;
    ao_tls_accessor.reset(new ao_ptr_holder(ptr));
}

int main()
{
    ao_t ao;

    ao_t::thread_decarator decor(boost::bind(th_decorator, _1));
    ao.add_thread_decarator(decor);
    ao.start(5);

    for(size_t i=0; i<1000; i++)
    {
        ao.enqueue_method(task());
    }

    sleep(1);
    ao.wait();
}




