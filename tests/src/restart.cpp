
#include "environment.h"
#include "test_templates.h"

int main()
{
	ao_pfull p;
	// start - work - stop
	test_simple_concurrency<ao_pfull, make_task>(p, 100000, 1, 1);
	// start - work - stop
	test_simple_concurrency<ao_pfull, make_task>(p, 100000, 2, 2);
	// start - work - stop
	test_simple_concurrency<ao_pfull, make_task>(p, 100000, 2, 2);
}




