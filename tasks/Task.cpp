/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <rtt/extras/FileDescriptorActivity.hpp>
#include <aggregator/TimestampEstimator.hpp>

using namespace dps_desertstar_ssp1;

Task::Task(std::string const& name)
    : TaskBase(name)
{
    driver = new Driver(); 
    timestamp_estimator = new aggregator::TimestampEstimator(base::Time::fromSeconds(2),base::Time::fromSeconds(1/16.0),INT_MAX);
	driver->registerHandler(this);
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine)
{
    driver = new Driver();
    timestamp_estimator = new aggregator::TimestampEstimator(base::Time::fromSeconds(2),base::Time::fromSeconds(1/16.0),INT_MAX);
	driver->registerHandler(this);
}

Task::~Task()
{
    delete driver;
    delete timestamp_estimator;
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

bool Task::configureHook()
{
    if (! TaskBase::configureHook())
        return false;

    activity = getActivity<RTT::extras::FileDescriptorActivity>();
    return driver->openSerial(_port.get(),4800);
}
bool Task::startHook()
{
    if (! TaskBase::startHook())
        return false;
    if(activity){
        activity->watch(driver->getFileDescriptor());
        activity->setTimeout(_timeout.get()*1000.0);
    }
    timestamp_estimator->reset();
    return true;
}

void Task::updateHook()
{
    TaskBase::updateHook();
    if (activity->hasError() || activity->hasTimeout()){
        return exception(IO_ERROR);
    }
    driver->process();
}

void Task::errorHook()
{
    TaskBase::errorHook();
    if(activity)
    {
        activity->clearAllWatches();
    }
}
void Task::stopHook()
{
    TaskBase::stopHook();
}
void Task::cleanupHook()
{
    TaskBase::cleanupHook();
}

void Task::newDepthData(const base::samples::RigidBodyState &depth){
    base::samples::RigidBodyState myDepth = depth;
    myDepth.time = timestamp_estimator->update(base::Time::now());
    _depth_samples.write(myDepth);
    _timestamp_estimator_status.write(timestamp_estimator->getStatus());
}

void Task::newTemperatureData(const double &temperature){
    _temperature.write(temperature);
}

