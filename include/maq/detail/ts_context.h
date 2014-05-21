#ifndef _MAQ_DETAIL_TS_CONTEXT_H_
#define _MAQ_DETAIL_TS_CONTEXT_H_

#include <boost/thread/mutex.hpp>


namespace maq {
namespace detail {

class ts_context
{
public:
	ts_context():mtx_(NULL){}
	ts_context(boost::mutex* mtx):mtx_(mtx)
	{}

	template<class F>
	void operator()(F f)
	{
		assert(mtx_);
		if(mtx_)
		{
			boost::unique_lock<boost::mutex> lock(*mtx_);
			f();
		}
	}

private:
	boost::mutex* mtx_;
};

}}


#endif
