#ifndef _MAQ_PLUGINS_PLUGIN_H_
#define _MAQ_PLUGINS_PLUGIN_H_


namespace maq {
namespace plugin {

#if __cplusplus >= 201103L
# define _constexpr constexpr
#else
# define _constexpr
#endif

#define static_type(name) static _constexpr const char* type_name() { return name; }


}}

#endif

