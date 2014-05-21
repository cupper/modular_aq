
#include "environment.h"
#include "test_templates.h"


int main()
{
	test_prio<ao_pfull, make_task>();
	test_later<ao_pfull, make_task>();
	test_prio_and_later<ao_pfull, make_task>();
	test_deadline<ao_pfull, make_task>();
	test_prio_deadline<ao_pfull, make_task>();
	test_deadline_defore_delay<ao_pfull, make_task>();

	// queue_container
    test_later<ao_qfull, make_task>();
    test_deadline<ao_qfull, make_task>();
    test_deadline_defore_delay<ao_qfull, make_task>();
}
