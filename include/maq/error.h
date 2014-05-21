#ifndef _MAQ_ERROR_H_
#define _MAQ_ERROR_H_

#include <boost/system/error_code.hpp>

#ifdef BOOST_SYSTEM_NOEXCEPT
# define AO_NOEXCEPT BOOST_SYSTEM_NOEXCEPT
#else
# define AO_NOEXCEPT
#endif//BOOST_SYSTEM_NOEXCEPT


namespace maq {
namespace error {

enum ao_errors
{
    SUCCESS,
    IS_FULL,
    IS_BUSY,
    IS_TIMEDOUT,
    IS_INACTIVE
};

namespace detail
{
class ao_error_category_impl: public boost::system::error_category
{
public:

    const char * name() const AO_NOEXCEPT
    {
        return "active_object";
    }

    std::string message(int ev) const AO_NOEXCEPT
    {
        if (ev == SUCCESS)
            return std::string("success");
        if (ev == IS_FULL)
            return std::string("queue of active object is full");
        if (ev == IS_BUSY)
            return std::string("active object cannot acquire mutex");
        if (ev == IS_TIMEDOUT)
            return std::string("active object is full and timeout is exhausted");
        if (ev == IS_INACTIVE)
            return std::string("active object is inactive");
        return std::string("unknown error");
    }
};
}

extern const boost::system::error_category& get_ao_category();
static const boost::system::error_category& ao_category = get_ao_category();

}}

namespace boost {
namespace system {

template<>
struct is_error_code_enum<maq::error::ao_errors>
{
    static const bool value = true;
};

}}


namespace maq {
namespace error {

inline boost::system::error_code make_error_code(ao_errors e)
{
    return boost::system::error_code(
            static_cast<int>(e), ao_category);
}

}}

#define INIT_AO_ERROR() \
        \
        namespace maq {  \
        namespace error {    \
            const boost::system::error_category& get_ao_category()\
            {\
                static const detail::ao_error_category_impl aocat;\
                return aocat;\
            }\
        }}\

#endif
