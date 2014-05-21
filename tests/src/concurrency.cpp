
#include "environment.h"
#include "test_templates.h"


int main()
{
	test_simple_concurrency<ao_pfull, make_task>(100000, 2, 2);
	test_simple_concurrency<ao_cancel_pfull, make_holder_task>(100000, 2, 2);

	test_deep_concurrency<ao_pfull, make_task>(1000000, 2, 2);
	test_deep_concurrency<ao_cancel_pfull, make_holder_task>(100000, 2, 2);
}



