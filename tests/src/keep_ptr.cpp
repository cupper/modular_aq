
#include "environment.h"
#include "test_templates.h"


int main()
{
	test_prio<ao_ptr_pfull, make_ptr_task>();
	test_later<ao_ptr_pfull, make_ptr_task>();
	test_prio_and_later<ao_ptr_pfull, make_ptr_task>();
	test_deadline<ao_ptr_pfull, make_ptr_task>();
	test_prio_deadline<ao_ptr_pfull, make_ptr_task>();
	test_deadline_defore_delay<ao_ptr_pfull, make_ptr_task>();
}



