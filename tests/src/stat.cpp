#include "environment.h"

class task_st
{
public:
	task_st(int name, system_clock::time_point delay, system_clock::time_point deadline, resault_set* rset)
		: name_(name)
		, delay_(delay)
		, deadline_(deadline)
		, rset_(rset)
		, hook_(0)
	{}

	void stat(const stat_t& st)
	{
		/*
		std::stringstream buf;
		buf << "task(" << name_ << ")" << std::endl;
		buf << "was pushed in: " << to_simple_string(st.push.first) << std::endl;
		buf << "time spent to push:" << (st.push.second - st.push.first).total_milliseconds() << " msec" << std::endl;
		if(hook_ == 1)
		{
			buf << "start executed in: " << to_simple_string(st.execute.first) << std::endl;
			buf << "executing: " << (st.execute.second - st.execute.first).total_milliseconds() << " msec" << std::endl;
			buf << "total spent: " << (st.execute.second - st.push.first).total_milliseconds() << " msec" << std::endl;
		}
		else if(hook_ == 2)
		{
			buf << "start canceled in: " << to_simple_string(st.cancel.first) << std::endl;
			buf << "canceling: " << (st.cancel.second - st.cancel.first).total_milliseconds() << " msec" << std::endl;
			buf << "total spent: " << (st.cancel.second - st.push.first).total_milliseconds() << " msec" << std::endl;
		}
		std::cout << buf.str();
		*/
		if(rset_)
			rset_->push_stat(name_, st);
	}

	void execute() {
		hook_ = 1;
		boost::this_thread::sleep_for(milliseconds(5));
	}

	void cancel() {}

	const system_clock::time_point& deadline() const { return deadline_; }
	const system_clock::time_point& delay() const { return delay_; }
private:
	int name_;

	system_clock::time_point delay_;
	system_clock::time_point deadline_;

	resault_set* rset_;
	int hook_;
};


typedef cancellable_proxy<task_st> c_holder;
typedef stat_holder<c_holder> s_holder;
typedef active_object<container<s_holder, priority, deferred, deadline>::type> stat_task_pool;

bool check_set_time(const se_time& time)
{
	return time.first.time_since_epoch().count() && time.second.time_since_epoch().count();
}

int main()
{
	resault_set rset;
	stat_task_pool o;
	o.start(2);

	system_clock::time_point delay1 = boost::chrono::system_clock::now() + milliseconds(50);
	system_clock::time_point delay2 = boost::chrono::system_clock::now() + milliseconds(100);


	o.enqueue_method(s_holder(c_holder(task_st(1, MIN_TIME_POINT, MAX_TIME_POINT, &rset))));
	o.enqueue_method(s_holder(c_holder(task_st(2, delay1, MAX_TIME_POINT, &rset))));
	o.enqueue_method(s_holder(c_holder(task_st(3, delay2, MAX_TIME_POINT, &rset))));

	o.wait();

	assert(rset.stat.size() == 3);
	assert(rset.stat[0].first == 1); // task 1
	assert(check_set_time(rset.stat[0].second.push));
	assert(check_set_time(rset.stat[0].second.execute));
	assert(check_set_time(rset.stat[0].second.cancel) == false);

	assert(rset.stat[2].first == 3); // task 3
	assert(check_set_time(rset.stat[2].second.push));
	assert(check_set_time(rset.stat[2].second.execute));
	assert(check_set_time(rset.stat[2].second.cancel) == false);

	assert(rset.stat[1].second.execute.first > delay1);
	assert(rset.stat[1].second.execute.second < (delay1 + milliseconds(10)));
	assert(rset.stat[2].second.execute.first > delay2);
	assert(rset.stat[2].second.execute.second < (delay2 + milliseconds(10)));

	return 0;
}



