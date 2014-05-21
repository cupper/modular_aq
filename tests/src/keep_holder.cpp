
#include "environment.h"
#include "test_templates.h"


int main()
{
	test_prio<ao_cancel_pfull, make_holder_task>();
	test_later<ao_cancel_pfull, make_holder_task>();
	test_prio_and_later<ao_cancel_pfull, make_holder_task>();
	test_deadline<ao_cancel_pfull, make_holder_task>();
	test_prio_deadline<ao_cancel_pfull, make_holder_task>();
	test_deadline_defore_delay<ao_cancel_pfull, make_holder_task>();
	test_cancel_and_prio();
	test_cancel_and_later();
	test_cancel_first_obj_in_after();
}


