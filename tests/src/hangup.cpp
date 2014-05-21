
#include "environment.h"

int repeated = 0;
int executed = 0;

class long_task
{
public:
	long_task(){}
	long_task(const system_clock::time_point& delay):delay_(delay){}

	const system_clock::time_point& delay() const { return delay_; }
private:
	system_clock::time_point delay_;
};

void repeat()
{
	for(int i=0; i<5; i++)
	{
		active_object<container<long_task, priority, deferred>::type> pool;

		pool.enqueue_method(long_task(MIN_TIME_POINT)); // task 1
		pool.enqueue_method(long_task(from_now(milliseconds(20)))); // task 2

		/*
		 * first th get task 1, second th wait until task 2
		 * first th executed task 1, wait until task 2
		 * one of th execute task 2, another must recognize that it's all, and exit
		 */
		pool.start(2);
		pool.wait();

		++repeated;
	}
}


int main()
{
	boost::thread th = boost::thread(repeat);

	boost::this_thread::sleep_for(seconds(7));

	assert(repeated == 5);
}


