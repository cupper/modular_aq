#ifndef _MAQ_DETAIL_MULTI_INDEX_KEYS_H_
#define _MAQ_DETAIL_MULTI_INDEX_KEYS_H_

#include <boost/chrono.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
//#include <boost/multi_index/identity.hpp>

#include "maq/extractor.h"


namespace maq {
namespace detail {

namespace mi = boost::multi_index;

template <class T>
struct deadline_key
{
        struct tag{};

        typedef mi::global_fun<
                    const T&, boost::chrono::system_clock::time_point const&,
                    extract_deadline<const T> > extractor;
        typedef mi::ordered_non_unique<mi::tag<tag>, extractor> ordered;
};

template <class T>
struct priority_key
{
        struct tag{};

        typedef mi::global_fun<
                    const T&, prio_t, extract_priority<const T> > extractor;
        typedef mi::ordered_non_unique<
                    mi::tag<tag>, extractor, std::greater<prio_t> > ordered;
};


}}


#endif /* MULTI_INDEX_KEYS_H_ */
