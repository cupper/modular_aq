#ifndef _MAQ_PLUGINS_DISABLE_H_
#define _MAQ_PLUGINS_DISABLE_H_

#include "maq/plugins/plugin.h"


namespace maq {
namespace plugins {

namespace detail {

class disable_plugin
{
public:
    static_type("disable");

    // Generic part
    template <class T>
    bool preprocessing(const T&) {return false; }

    void start(){};
    void stop(){};
    void join(){};

    template<class Skip>
    bool postprocessing(const Skip&) { return false;}

    // Deadline part
    template<class Skip1, class Skip2>
    void ts_context(const Skip1&, const Skip2&) {}

    template <class T>
    bool is_expired(const T&) {return false;}

    // Later part
    template <class Skip>
    void nearest_time(const Skip&) const {}

    template <class Skip>
    void release_all(const Skip&){};

    //active_object part
    template <typename T>
    void reset(const T&){};

    struct collector
    {
        template <typename T>
        void collect(const T&){};
    };

    template <typename T>
    collector& get_collector(const T&)
    {
        static collector dummy;
        return dummy;
    }

    boost::chrono::system_clock::duration average()
    {
        static boost::chrono::system_clock::duration tmp;
        return tmp;
    }
};
}

struct disable
{
    template <class T>
    struct bind
    {
        typedef detail::disable_plugin type;
    };
};

}}

#endif
