#ifndef _MAQ_INIT_H_
#define _MAQ_INIT_H_

#include <maq/error.h>

#define INIT_AO_TLS_ACCESSOR() \
    boost::thread_specific_ptr<maq::detail::tls_holder> \
        maq::detail::active_object_tls;

#define INIT_ACTIVE_OBJECT() \
        INIT_AO_TLS_ACCESSOR();\
        INIT_AO_ERROR();


#endif
