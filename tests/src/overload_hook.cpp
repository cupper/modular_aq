#include "maq/active_object.h"
#include "maq/stat_holder.h"
#include "maq/container.h"

#include "maq/init.h"
INIT_ACTIVE_OBJECT();

using boost::chrono::system_clock;
using namespace maq;
using namespace maq::plugins;

system_clock::time_point MAX_TIME_POINT = system_clock::time_point::max();
system_clock::time_point MIN_TIME_POINT = system_clock::time_point::min();

struct dummy_task
{
	dummy_task(): execute(0), cancel(0), stat(0), priority(0), delay(0), deadline(0){}

	bool execute;
	bool cancel;
	bool stat;
	mutable bool priority;
	mutable bool delay;
	mutable bool deadline;
};

void execute(dummy_task& d)
{
	d.execute = true;
}

void cancel(dummy_task& d)
{
	d.cancel = true;
}

void stat(dummy_task& d, stat_t const&)
{
	d.stat = true;
}

prio_t get_priority(const dummy_task& d)
{
	d.priority = true;
	return 0;
}

system_clock::time_point const& get_delay(const dummy_task& d)
{
	d.delay = true;
	return MIN_TIME_POINT;
}

system_clock::time_point const& get_deadline(const dummy_task& d)
{
	d.deadline = true;
	return MAX_TIME_POINT;
}

typedef cancellable_proxy<dummy_task> c_holder;
typedef stat_holder<c_holder> s_holder;
typedef active_object<container<
				s_holder, priority, deferred, deadline>::type> ao_sc_pfull;

int main()
{
    ao_sc_pfull p;

	s_holder t1((c_holder((dummy_task()))));
	cancellable_holder<dummy_task> t2((dummy_task()));

	p.enqueue_method(t1);
	p.enqueue_method(s_holder(t2.get_proxy()));
	t2.cancel();

	p.start();
	p.wait();

	dummy_task& ref_t1 = t1.get().get();
	assert(ref_t1.execute);
	assert(ref_t1.stat);
	assert(ref_t1.priority);
	assert(ref_t1.delay);
	assert(ref_t1.deadline);

	dummy_task& ref_t2 = t2.get_proxy().get();
	assert(ref_t2.cancel);
	assert(ref_t2.stat);
}


