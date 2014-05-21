
#include "maq/container.h"

using namespace maq;
using namespace maq::plugins;

class task{};

int main()
{
    std::cout << container<task, priority>::type::type_name() << std::endl;
    std::cout << container<task, priority, deferred>::type::type_name() << std::endl;
    std::cout << container<task, priority, deadline>::type::type_name() << std::endl;
    std::cout << container<task, priority, deferred, deadline>::type::type_name() << std::endl;
    std::cout << container<task, priority, deadline, deferred>::type::type_name() << std::endl;
    std::cout << container<task, deadline, priority, deferred>::type::type_name() << std::endl;
    std::cout << container<task, deadline, deferred, priority>::type::type_name() << std::endl;

    std::cout << container<task>::type::type_name() << std::endl;
    std::cout << container<task, deferred>::type::type_name() << std::endl;
    std::cout << container<task, deadline>::type::type_name() << std::endl;
    std::cout << container<task, deferred, deadline>::type::type_name() << std::endl;
    std::cout << container<task, deadline, deferred>::type::type_name() << std::endl;
}
