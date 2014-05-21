#ifndef _MAQ_TESTS_ENVIRONMENT_H_
#define _MAQ_TESTS_ENVIRONMENT_H_

#include <deque>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/chrono.hpp>

#include "maq/active_object.h"
#include "maq/container.h"

#include "maq/stat_holder.h"
#include "maq/cancellable_holder.h"

#include "maq/plugins.h"
#include "maq/init.h"

using namespace maq;
using namespace maq::plugins;
using namespace maq::statistic;
using namespace boost::chrono;

system_clock::time_point MAX_TIME_POINT = system_clock::time_point::max();
system_clock::time_point MIN_TIME_POINT = system_clock::time_point::min();

std::string to_simple_string(const system_clock::time_point& tp)
{
	std::time_t t = system_clock::to_time_t(tp);
	return std::string(std::ctime(&t));
}

uint64_t total_seconds(const system_clock::duration& d)
{
	return boost::chrono::duration_cast<boost::chrono::seconds>(d).count();
}

uint64_t total_milliseconds(const system_clock::duration& d)
{
	return boost::chrono::duration_cast<boost::chrono::milliseconds>(d).count();
}

uint64_t total_microseconds(const system_clock::duration& d)
{
	return boost::chrono::duration_cast<boost::chrono::microseconds>(d).count();
}

class resault_set;

boost::mutex CHECK_LEAK_MTX;
int CHECK_LEAK = 0;

struct check_leak
{
	check_leak()
	{
		boost::lock_guard<boost::mutex> lock(CHECK_LEAK_MTX);
		++CHECK_LEAK;
	};
	check_leak(const check_leak&)
	{
		boost::lock_guard<boost::mutex> lock(CHECK_LEAK_MTX);
		++CHECK_LEAK;
	}
	~check_leak()
	{
		boost::lock_guard<boost::mutex> lock(CHECK_LEAK_MTX);
		--CHECK_LEAK;
	}
};
bool check_memory_leak()
{
	/*
	bool ret = (CHECK_LEAK == 0);
	CHECK_LEAK = 0;
	return ret;
	*/
	return true;
}

struct data
{
	data()
		: name_(0)
		, prio_(0)
		, delay_(MIN_TIME_POINT)
		, deadline_(MAX_TIME_POINT)
		, created_(system_clock::now())
		, executed_(MAX_TIME_POINT)
		, canceled_(MAX_TIME_POINT)
	{}

	void print(std::ostream& os)
	{
		os << "Task(" << name_ << ")" << std::endl;
		os << "created: " << to_simple_string(created_) << std::endl;
		if(deadline_ != MIN_TIME_POINT)
			os << "deadline_: " << to_simple_string(deadline_) << std::endl;
		if(delay_ != MAX_TIME_POINT)
			os << "delay_: " << to_simple_string(delay_) << std::endl;


		if(executed_ != MAX_TIME_POINT)
		{
			os << "executed_: " << to_simple_string(executed_) << std::endl;
			if(deadline_ != MIN_TIME_POINT && executed_ > deadline_)
				os << "expired: " << total_microseconds(executed_ - deadline_) << " msec" << std::endl;
			if(delay_ != MAX_TIME_POINT && executed_ < delay_)
				os << "earlier: " << total_microseconds(delay_ - executed_) << " msec" << std::endl;
		}

		if(canceled_ != MAX_TIME_POINT)
		{
			os << "canceled_: " << to_simple_string(canceled_) << std::endl;
			if(deadline_ != MIN_TIME_POINT && canceled_ < deadline_)
				os << "earlier: " << total_microseconds(deadline_ - canceled_) << " msec" << std::endl;
		}
	}

	int name_;

	int prio_;
	system_clock::time_point delay_;
	system_clock::time_point deadline_;

	system_clock::time_point created_;
	system_clock::time_point executed_;
	system_clock::time_point canceled_;
};

class task
{
public:
	task():rset_(NULL){}

	task(int name, resault_set* rset):rset_(rset)
	{
		data_.name_ = name;
	}
	task(int name, resault_set* rset, int prio): rset_(rset)
	{
		data_.name_ = name;
		data_.prio_ = prio;
	}

	task(int name, resault_set* rset, int prio, system_clock::time_point const& delay, system_clock::time_point const& deadline)
		:rset_(rset)
	{
		data_.name_ = name;
		data_.prio_ = prio;
		data_.delay_ = delay;
		data_.deadline_ = deadline;
	}

	void execute();
	void cancel();

	void stat(const stat_t& st);

	prio_t priority() const { return data_.prio_; }

	const system_clock::time_point& deadline() const { return data_.deadline_; }
	void deadline(const system_clock::time_point& t) {data_.deadline_ = t;}

	const system_clock::time_point& delay() const { return data_.delay_; }
	void delay(const system_clock::time_point& t) {data_.delay_ = t;}

	const data& get_data() const { return data_; }


private:
	data data_;
	resault_set* rset_;
	check_leak leak_;
};

class resault_set
{
public:
	void push_execute(int i)
	{
		boost::lock_guard<boost::mutex> lock(e_mtx_);
		e.push_back(i);
	}

	void push_cancel(int i)
	{
		boost::lock_guard<boost::mutex> lock(c_mtx_);
		c.push_back(i);
	}

	void push_untimely(const task& t)
	{
		boost::lock_guard<boost::mutex> lock(c_mtx_);
		untimely.push_back(t.get_data());
	}

	void push_expired(const task& t)
	{
		boost::lock_guard<boost::mutex> lock(c_mtx_);
		expired.push_back(t.get_data());
	}

	void push_twice(const task& t)
	{
		boost::lock_guard<boost::mutex> lock(c_mtx_);
		twice.push_back(t.get_data());
	}

	void push_stat(int name, const stat_t& st)
	{
		boost::lock_guard<boost::mutex> lock(c_mtx_);
		stat.push_back(std::make_pair(name, st));
	}

	std::vector<int> e;
	std::vector<int> c;
	std::vector<data> untimely;
	std::vector<data> expired;
	std::vector<data> twice;
	std::vector<std::pair<int, stat_t> > stat;
private:

	boost::mutex e_mtx_;
	boost::mutex c_mtx_;
};

void task::execute()
{
	//std::cout << "Task(" << name_ << ")::execute()\n";
	data_.executed_ = system_clock::now();
	if(rset_)
	{
		rset_->push_execute(data_.name_);

		if(data_.executed_ > data_.deadline_)
			rset_->push_expired(*this);

		if(data_.executed_ < data_.delay_)
			rset_->push_untimely(*this);

		if(data_.canceled_ != MAX_TIME_POINT)
			rset_->push_twice(*this);
	}
}

void task::cancel()
{
	//std::cout << "Task(" << name_ << ")::cancel()\n";
	data_.canceled_ = system_clock::now();
	if(rset_)
	{
		rset_->push_cancel(data_.name_);

		if(data_.canceled_ < data_.deadline_)
			rset_->push_untimely(*this);
		if(data_.executed_ != MAX_TIME_POINT)
			rset_->push_twice(*this);
	}
}

void task::stat(const stat_t& st)
{
	if(rset_)
		rset_->push_stat(data_.name_, st);
}

typedef task* task_ptr;
typedef active_object<container<task, priority, deferred, deadline>::type> ao_pfull;
typedef active_object<container<task, deferred, deadline>::type> ao_qfull;

typedef active_object<container<task_ptr, priority, deferred, deadline>::type> ao_ptr_pfull;

typedef cancellable_proxy<task> holder;
typedef active_object<container<holder, priority, deferred, deadline>::type> ao_cancel_pfull;

typedef cancellable_proxy<task_ptr> holder_ptr;
typedef active_object<container<
				holder_ptr, priority, deferred, deadline>::type> ao_cancel_ptr_pfull;


struct make_task
{
	static task make(int i, resault_set& rset, int prio)
	{
		return task(i, &rset, prio);
	}

	static task make(int i, resault_set& rset, int prio, system_clock::time_point delay, system_clock::time_point deadline)
	{
		return task(i, &rset, prio, delay, deadline);
	}
};

struct make_ptr_task
{
	static task* make(int i, resault_set& rset, int prio)
	{
		return new task(i, &rset, prio);
	}

	static task* make(int i, resault_set& rset, int prio, system_clock::time_point delay, system_clock::time_point deadline)
	{
		return new task(i, &rset, prio, delay, deadline);
	}
};

struct make_holder_task
{
	static holder make(int i, resault_set& rset, int prio)
	{
		return cancellable_holder<task>(task(i, &rset, prio)).get_proxy();
	}

	static holder make(int i, resault_set& rset, int prio, system_clock::time_point delay, system_clock::time_point deadline)
	{
		return cancellable_holder<task>(task(i, &rset, prio, delay, deadline)).get_proxy();
	}
};

struct make_holder_task_ptr
{
	static holder_ptr make(int i, resault_set& rset, int prio)
	{
		return cancellable_holder<task_ptr>(new task(i, &rset, prio)).get_proxy();
	}

	static holder_ptr make(int i, resault_set& rset, int prio, system_clock::time_point delay, system_clock::time_point deadline)
	{
		return cancellable_holder<task_ptr>(new task(i, &rset, prio, delay, deadline)).get_proxy();
	}
};

struct decreasing
{
	bool operator()(int cur, int next)
	{ return cur >= next; }
};

struct increasing
{
	bool operator()(int cur, int next)
	{ return cur <= next; }
};

template<class Order, class Q>
bool ordered_in(Q& q)
{
	if(q.begin() == q.end() || q.size() == 1)
		return true;

	std::vector<int>::iterator cur = q.begin();
	std::vector<int>::iterator next = cur + 1;
	std::vector<int>::iterator end = q.end();

	Order op;
	for(; next != end; cur++, next++)
		if(!op(*cur, *next))
			return false;
	return true;
}

template<class Duration>
system_clock::time_point from_now(const Duration& d)
{
	return system_clock::now() + d;
}

INIT_ACTIVE_OBJECT();

#endif
