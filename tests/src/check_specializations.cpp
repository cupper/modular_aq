#ifndef CHECK_SPECIALIZATIONS_CPP_
#define CHECK_SPECIALIZATIONS_CPP_

#include "maq/active_object.h"

#include "maq/priority_container.h"
#include "maq/queue_container.h"

#include "maq/stat_holder.h"
#include "maq/cancellable_holder.h"

#include "maq/plugins.h"

#include "maq/init.h"
INIT_ACTIVE_OBJECT();


using namespace maq;
using namespace maq::plugins;
using namespace boost::chrono;

class Task
{
public:
    void execute(){};
    void cancel(){};

    void stat(const stat_t&){};

    prio_t priority() const { return 0; }

    const system_clock::time_point& deadline() const
    { static system_clock::time_point tp; return tp; }

    const system_clock::time_point& delay() const
    { static system_clock::time_point tp; return tp; }
};
typedef stat_holder<Task> STask;
typedef cancellable_holder<Task>::proxy CTask;
typedef stat_holder<CTask> SCTask;

template <class Type>
struct make_task;


template <>
struct make_task<Task>
{
    Task operator()() { return Task(); }
};

template <>
struct make_task<STask>
{
    STask operator()()
    { return STask(Task()); }
};

template <>
struct make_task<CTask>
{
    CTask operator()()
    { return cancellable_holder<Task>(Task()).get_proxy(); }
};

template <>
struct make_task<SCTask>
{
    SCTask operator()()
    { return SCTask(cancellable_holder<Task>(Task()).get_proxy()); }
};


template<class Container>
void check()
{
    //std::cout << active_object<Container>::type() << std::endl;
    active_object<Container> ao;
    ao.start();

    ao.capacity();

    make_task<typename Container::value_type> maker;
    ao.enqueue_method(maker());
    ao.wait();
    ao.stop();
};

int main()
{
    // Disable all plugins
    {
        typedef priority_container<Task> PrioContainer;
        typedef priority_container<STask> StatPrioContainer;
        typedef priority_container<CTask> CancellablePrioContainer;
        typedef priority_container<SCTask> StatCancellablePrioContainer;

        typedef queue_container<Task> Container;
        typedef queue_container<STask> StatContainer;
        typedef queue_container<CTask> CancellableContainer;
        typedef queue_container<SCTask> StatCancellableContainer;

        check<PrioContainer>();
        check<StatPrioContainer>();
        check<CancellablePrioContainer>();
        check<StatCancellablePrioContainer>();

        check<Container>();
        check<StatContainer>();
        check<CancellableContainer>();
        check<StatCancellableContainer>();
    }

    // enable deferred_plugin
    {
        typedef priority_container<Task, deferred>PrioContainerDeferred;
        typedef priority_container<STask, deferred>StatPrioContainerDeferred;
        typedef priority_container<CTask, deferred>CancellablePrioContainerDeferred;
        typedef priority_container<SCTask, deferred>StatCancellablePrioContainerDeferred;

        typedef queue_container<Task, deferred>ContainerDeferred;
        typedef queue_container<STask, deferred>StatContainerDeferred;
        typedef queue_container<CTask, deferred>CancellableContainerDeferred;
        typedef queue_container<SCTask, deferred>StatCancellableContainerDeferred;

        check<PrioContainerDeferred>();
        check<StatPrioContainerDeferred>();
        check<CancellablePrioContainerDeferred>();
        check<StatCancellablePrioContainerDeferred>();

        check<ContainerDeferred>();
        check<StatContainerDeferred>();
        check<CancellableContainerDeferred>();
        check<StatCancellableContainerDeferred>();
    }

    // enable deadline_plugin
    {
        typedef priority_container<Task, disable, deadline> PrioContainerDeadline;
        typedef priority_container<STask, disable, deadline> StatPrioContainerDeadline;
        typedef priority_container<CTask, disable, deadline> CancellablePrioContainerDeadline;
        typedef priority_container<SCTask, disable, deadline> StatCancellablePrioContainerDeadline;

        typedef queue_container<Task, disable, deadline> ContainerDeadline;
        typedef queue_container<STask, disable, deadline> StatContainerDeadline;
        typedef queue_container<CTask, disable, deadline> CancellableContainerDeadline;
        typedef queue_container<SCTask, disable, deadline> StatCancellableContainerDeadline;

        check<PrioContainerDeadline>();
        check<StatPrioContainerDeadline>();
        check<CancellablePrioContainerDeadline>();
        check<StatCancellablePrioContainerDeadline>();

        check<ContainerDeadline>();
        check<StatContainerDeadline>();
        check<CancellableContainerDeadline>();
        check<StatCancellableContainerDeadline>();
    }

    // enable deferred_plugin, deadline_plugin
    {
        typedef priority_container<Task, deferred>PrioContainerDeadlineDeffered;
        typedef priority_container<STask, deferred>StatPrioContainerDeadlineDeffered;
        typedef priority_container<CTask, deferred>CancellablePrioContainerDeadlineDeffered;
        typedef priority_container<SCTask, deferred>StatCancellablePrioContainerDeadlineDeffered;

        typedef queue_container<Task, deferred>ContainerDeadlineDeffered;
        typedef queue_container<STask, deferred>StatContainerDeadlineDeffered;
        typedef queue_container<CTask, deferred>CancellableContainerDeadlineDeffered;
        typedef queue_container<SCTask, deferred>StatCancellableContainerDeadlineDeffered;

        check<PrioContainerDeadlineDeffered>();
        check<StatPrioContainerDeadlineDeffered>();
        check<CancellablePrioContainerDeadlineDeffered>();
        check<StatCancellablePrioContainerDeadlineDeffered>();

        check<ContainerDeadlineDeffered>();
        check<StatContainerDeadlineDeffered>();
        check<CancellableContainerDeadlineDeffered>();
        check<StatCancellableContainerDeadlineDeffered>();
    }

}

#endif /* CHECK_SPECIALIZATIONS_CPP_ */
