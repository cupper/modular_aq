#ifndef _MAQ_TESTS_TEMPLATES_H_
#define _MAQ_TESTS_TEMPLATES_H_

#include <cassert>
#include <iostream>
#include "environment.h"

template <class Pool, class Maker>
void test_prio()
{
	resault_set rset;
	Pool o;

	int total = 0;
	for(int i=0; i<60; i++)
	{
		o.enqueue_method(Maker::make(i, rset, i));
		++total;
	}

	for(int i=100; i>40; i--)
	{
		o.enqueue_method(Maker::make(i, rset, i));
		++total;
	}

	o.start();
	o.wait();

	assert(rset.e.size() == 120);
	assert(ordered_in<decreasing>(rset.e));
	assert(rset.c.size() == 0);
	assert(check_memory_leak());
}

template <class Pool, class Maker>
void test_later()
{
	resault_set rset;
	Pool o;
	o.start();

	for(int i=100; i>=1; i--)
	{
		/* Input sequence: 100,...,1
		 * **
		 * Task(100, prio = 100) - run last (in 1000 msec)
		 * ...
		 * Task(1, prio = 1) - run first (in 10 msec)
		 * **
		 * Output sequence: 1,...,100
		 */
		o.enqueue_method(Maker::make(i, rset, i, from_now(milliseconds(i*10)), MAX_TIME_POINT));
	}

	sleep(2);
	o.stop();

	assert(rset.e.size() == 100);
	assert(ordered_in<increasing>(rset.e));
	assert(rset.c.size() == 0);
	assert(check_memory_leak());
}

template <class Pool, class Maker>
void test_prio_and_later()
{
	resault_set rset;
	Pool o;
	o.start();

	system_clock::time_point now = system_clock::now();

	/* Input sequence: 10...1, 20...11, ..., 100...91
	 * **
	 * Task(10, prio = 1, delay = 100msec) (run after Task(1))
	 * ...
	 * Task(1, prio = 10, delay = 100msec) (run before Task(10))
	 * Task(20, prio = 10, delay = 200msec) (run before Task(11))
	 * ..
	 * Task(11, prio = 10, delay = 200msec) (run before Task(20))
	 * **
	 * Output sequence: 1 ... 100
	 */
	for(int i=0; i < 10; i++)
	{
		int decade = i * 10;
		for(int j=1; j<=10; j++)
		{
			o.enqueue_method(Maker::make(decade + (10 - j + 1), rset, j,
									now + milliseconds((i+1) * 100),
									MAX_TIME_POINT));
		}
	}

	sleep(2);
	o.stop();

	assert(rset.e.size() == 100);
	assert(ordered_in<increasing>(rset.e));
	assert(rset.c.size() == 0);
	assert(check_memory_leak());
}

template <class Pool, class Maker>
void test_deadline()
{
	resault_set rset;
	Pool o;

	for(int i=1; i<=100; i++)
	{
		// Executed each 10tn task (10, 20, ... 100) = 10
		// Cancelled all other (1,...,9,11,...19,...,99) = 90
		if((i % 10) == 0)
			o.enqueue_method(Maker::make(i, rset, 0,
								MIN_TIME_POINT,
								from_now(milliseconds(500))));
		else
			o.enqueue_method(Maker::make(i, rset, 0,
								MIN_TIME_POINT,
								from_now(milliseconds(100))));

	}

	boost::this_thread::sleep_for(milliseconds(300));

	o.start();
	o.wait();

	assert(rset.e.size() == 10);
	assert(ordered_in<increasing>(rset.e));
	assert(rset.c.size() == 90);
	assert(ordered_in<increasing>(rset.c));
	assert(check_memory_leak());
}

template <class Pool, class Maker>
void test_prio_deadline()
{
	resault_set rset;
	Pool o;

	for(int i=1; i<=100; i++)
	{
		/* Input sequence: 100,...,1
		 * **
		 * Task(1, prio = 1)
		 * ...
		 * Task(100, prio = 100)
		 * **
		 * Output sequence:
		 * Executed each 10tn task (90, 80, ... 10) = 10
		 * Cancelled all other (1,...,9,11,...19,...,99) = 90 (when close ignore prio)
		 */
		if((i % 10) == 0)
			o.enqueue_method(Maker::make(i, rset, i, MIN_TIME_POINT, from_now(milliseconds(500))));
		else
			o.enqueue_method(Maker::make(i, rset, i, MIN_TIME_POINT, from_now(milliseconds(100))));

	}

	boost::this_thread::sleep_for(milliseconds(300));

	o.start();
	o.wait();

	assert(rset.e.size() == 10);
	assert(ordered_in<decreasing>(rset.e));
	assert(rset.c.size() == 90);
	assert(ordered_in<increasing>(rset.c));
	assert(check_memory_leak());
}

template <class Pool, class Maker>
void test_deadline_defore_delay()
{
	resault_set rset;
	Pool o;

	for(int i=1; i<=10000; i++)
	{
		// Unpredictable behavior
		// expired tasks either might be executed or cancelled
		if((i % 10) == 0)
			o.enqueue_method(Maker::make(i, rset, 10000 - i,
								from_now(milliseconds(100)),
								from_now(milliseconds(200))));
		else
			o.enqueue_method(Maker::make(i, rset, 10000 - i,
								from_now(milliseconds(200)),
								from_now(milliseconds(100))));

	}

	o.start();
	o.wait();

	std::cout << "executed: " << rset.e.size() << std::endl;
	std::cout << "canceled: " << rset.c.size() << std::endl;

	assert((rset.e.size() + rset.c.size()) == 10000);
	assert(ordered_in<increasing>(rset.c));
	assert(check_memory_leak());
}

template <class Pool, class Maker>
void pusher(Pool& p, resault_set& rset, int produce)
{
	int i = 0;
	while(i<produce)
	{
	    boost::system::error_code err;
		p.enqueue_method(Maker::make(i, rset, i), err);
		if(!err)
			i++;
	}
}

template <class Pool, class Maker>
void test_simple_concurrency(int produce, int producer, int consumer)
{
	Pool o;
	resault_set rset;

	o.start(consumer);

	boost::thread_group pool;
	for(int i=0; i<producer; i++)
		pool.add_thread(new boost::thread(
						pusher<Pool, Maker>, boost::ref(o),
						boost::ref(rset), produce / producer));

	pool.join_all();
	o.wait();

	assert((rset.e.size() + rset.c.size()) == static_cast<size_t>(produce));
	assert(rset.twice.size() == 0);
	assert(rset.untimely.size() == 0);
	assert(rset.expired.size() == 0);
	assert(check_memory_leak());
}

template <class Pool, class Maker>
void test_simple_concurrency(Pool& o, size_t produce, int producer, int consumer)
{
	resault_set rset;

	o.start(consumer);

	boost::thread_group pool;
	for(int i=0; i<producer; i++)
		pool.add_thread(new boost::thread(
						pusher<Pool, Maker>, boost::ref(o),
						boost::ref(rset), produce / producer));

	pool.join_all();
	o.wait();

	assert((rset.e.size() + rset.c.size()) == static_cast<size_t>(produce));
	assert(rset.twice.size() == 0);
	assert(rset.untimely.size() == 0);
	assert(rset.expired.size() == 0);
	assert(check_memory_leak());

	/*
	o.show_stat(std::cout);
	typedef std::vector<std::pair<int, stat_t> > stat;
	stat st = rset.stat;
	if(st.size())
	{
		//analize_stat
		uint64_t time = 0;
		for(stat::iterator it = st.begin(); it != st.end(); it++)
			time += (*it).second.full_time().total_microseconds();
		std::cout << "general time: " << time / st.size() << " mcsec\n";

		std::cout << "10 serial choices:\n";
		for(size_t i=1; i<=10; i++)
			std::cout << st[(st.size()/11) * i].second.full_time().total_microseconds() << " mcsec\n";
	}
	*/
}

template<class Pool, class Maker>
void random_pusher(Pool& p, resault_set& rset, int produce)
{
	int i = 0;
	while (i < produce)
	{
		int delay = rand() % 1000;
		int deadline = delay + 50 + rand() % 1000;
		typename Pool::value_type h = Maker::make(i, rset, i, from_now(milliseconds(delay)),from_now(milliseconds(deadline)));
        boost::system::error_code err;
        p.enqueue_method(h, err);
        if(!err)
            i++;
	}
}

template <class Pool, class Maker>
void test_deep_concurrency(int produce, int producer, int consumer)
{
	Pool o;
	resault_set rset;

	o.start(consumer);

	boost::thread_group pool;
	for(int i=0; i<producer; i++)
		pool.add_thread(new boost::thread(
						random_pusher<Pool, Maker>, boost::ref(o),
						boost::ref(rset), produce / producer));

	pool.join_all();
	o.wait();

	assert((rset.e.size() + rset.c.size()) == static_cast<size_t>(produce));
	assert(check_memory_leak());

	/*
	for(int i=0; i<10; i++)
	{
		rset.untimely[rand() % rset.untimely.size()].print(std::cout);
		std::cout << std::endl;
	}

	assert(rset.twice.size() == 0);
	assert(rset.untimely.size() == 0);
	assert(rset.expired.size() == 0);
	*/
}

void test_cancel_and_prio()
{
	resault_set rset;
	ao_cancel_pfull o;

	std::vector<cancellable_holder<task> > vec;

	for(int i=1; i<=100; i++)
	{
		/* Input sequence: 1,...,100
		 * **
		 * Task(1, prio = 1)
		 * ...
		 * Task(100, prio = 100)
		 * **
		 * Output sequence:
		 * Cancelled each 10tn (10,...,100) = 10
		 * Executed all other (99,...,81,79,...,1) = 90
		 */
		cancellable_holder<task> h(task(i, &rset, i));
		o.enqueue_method(h.get_proxy());
		if((i%10) == 0)
			vec.push_back(h);
	}

	for(std::vector<cancellable_holder<task> >::iterator it = vec.begin(); it != vec.end(); it++)
		(*it).cancel();

	o.start();
	o.wait();

	vec.clear();

	assert(rset.e.size() == 90);
	assert(ordered_in<decreasing>(rset.e));
	assert(rset.c.size() == 10);
	assert(ordered_in<increasing>(rset.c));
	assert(check_memory_leak());
}

void test_cancel_and_later()
{
	resault_set rset;
	ao_cancel_pfull o;

	std::vector<cancellable_holder<task> > vec;

	for(int i=1; i<=100; i++)
	{
		/* Input sequence: 1,...,100
		 * **
		 * Task(1, prio = 1, delay = 10)
		 * ...
		 * Task(100, prio = 100, delay = 1000)
		 * **
		 * Output sequence:
		 * Cancelled each 10tn (10,...,100) = 10
		 * Executed all other (1,...,79,81,...,99) = 90
		 */
		cancellable_holder<task> h(task(i, &rset, i, from_now(milliseconds(i*10)), MAX_TIME_POINT));
		o.enqueue_method(h.get_proxy());
		if((i%10) == 0)
			vec.push_back(h);
	}

	for(std::vector<cancellable_holder<task> >::iterator it = vec.begin(); it != vec.end(); it++)
		(*it).cancel();

	o.start();
	sleep(1);
	o.wait();

	vec.clear();

	assert(rset.e.size() == 90);
	assert(ordered_in<increasing>(rset.e));
	assert(rset.c.size() == 10);
	assert(ordered_in<increasing>(rset.c));
	assert(check_memory_leak());
}

void test_cancel_first_obj_in_after()
{
	resault_set rset;
	ao_cancel_pfull o;


	{
		cancellable_holder<task> h1(task(1, &rset, 0, from_now(milliseconds(100)), MAX_TIME_POINT));
		cancellable_holder<task> h2(task(2, &rset, 0, from_now(milliseconds(110)), MAX_TIME_POINT));
		cancellable_holder<task> h3(task(3, &rset, 0, from_now(milliseconds(120)), MAX_TIME_POINT));

		o.enqueue_method(h1.get_proxy());
		o.enqueue_method(h2.get_proxy());
		o.enqueue_method(h3.get_proxy());

		// shit happens here, because it's the first object in Later Queue
		h1.cancel();
	}

	o.start();
	boost::this_thread::sleep_for(milliseconds(200));
	o.wait();

	assert(rset.e.size() == 2);
	assert(ordered_in<increasing>(rset.e));
	assert(rset.c.size() == 1);
	assert(check_memory_leak());
}

#endif /* TEST_TEMPLATES_HPP_ */
