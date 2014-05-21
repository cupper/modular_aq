
#include "environment.h"
#include "test_templates.h"


int main()
{
	test_prio<ao_cancel_ptr_pfull, make_holder_task_ptr>();
	test_later<ao_cancel_ptr_pfull, make_holder_task_ptr>();
	test_prio_and_later<ao_cancel_ptr_pfull, make_holder_task_ptr>();
	test_deadline<ao_cancel_ptr_pfull, make_holder_task_ptr>();
	test_prio_deadline<ao_cancel_ptr_pfull, make_holder_task_ptr>();
	test_deadline_defore_delay<ao_cancel_ptr_pfull, make_holder_task_ptr>();
}


