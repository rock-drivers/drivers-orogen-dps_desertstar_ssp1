#include "Task.hpp"

#include <dps/dps.h>
#include <rtt/extras/FileDescriptorActivity.hpp>
using namespace desertstar_ssp1;

Task::Task(std::string const& name)
    : TaskBase(name)
{
}

bool Task::smoothReading(const dps::PressureReading& reading, dps::PressureReading& mean)
{
    if(_window_size.get() < 2)
	return false;
    
    // apply filter
    readings.push_front(reading);
    while(readings.size() > _window_size.get())
	readings.pop_back();
    
    double lambda = _lambda.get();
    if(lambda <= 0.0 || lambda > 1.0)
	lambda = 1.0;
    if(readings.size() == _window_size.get())
    {
	double mean_pressure = 0.0;
	double sum_weight = 0.0;
	double sample_count = 1.0;
	for(std::list<dps::PressureReading>::const_iterator it = readings.begin(); it != readings.end(); it++)
	{
	    double weight = pow(lambda, sample_count);
	    mean_pressure += weight * it->pressure;
	    sum_weight += weight;
	    sample_count++;
	}
	mean_pressure /= sum_weight;
	
	mean.pressure = mean_pressure;
	mean.timestamp = reading.timestamp;
	return true;
    }
    
    return false;
}

bool Task::configureHook()
{
  RTT::extras::FileDescriptorActivity* activity =
  getActivity<RTT::extras::FileDescriptorActivity>();

  char sys_stty_cmd[100];
  sprintf(sys_stty_cmd,"stty -F %s 4800 cs8", _device.get().c_str());
  system(sys_stty_cmd);
   
  // Open the device that is listed in the _device string property
  dps.comPort = open(_device.get().c_str(), O_RDONLY);
  if (dps.comPort == -1)
    return false;

  activity->watch(dps.comPort);
  activity->setTimeout(500);
  return true;
}

bool Task::startHook()
{
  dps.flushPort(0);
  dps.the_file_stream.open("dps.txt");
  return true;
}

void Task::updateHook()
{
  RTT::extras::FileDescriptorActivity* fd_activity =
  getActivity<RTT::extras::FileDescriptorActivity>();
  if (fd_activity)
  {
    if (fd_activity->hasError())
    {
      std::cerr << "error!" << std::endl;
    }
    else if (fd_activity->hasTimeout())
    {
      std::cerr << "DPS module: timeout! did not receive any data via the com port, going fatal..." << std::endl;
      fatal(IO_ERROR);
    }
    else
    {
      // If there is more than one FD, discriminate. Otherwise,
      // we don't need to use isUpdated
      if (fd_activity->isUpdated(dps.comPort))
      {
	dps.prevChar = dps.currChar;
	dps.currChar = dps.getCharFromPort();
	//std::cerr << dps.currChar;
	dps.the_file_stream << dps.currChar;
	dps.the_file_stream.flush();

	dps.readBuffer.push_back(dps.currChar);

	if(dps.prevChar == 'D' && dps.currChar == 'P')
	{
	  if(!dps.receivedFirstTimeStamp)
	  {
	    dps.readBuffer.clear();
	    dps.receivedFirstTimeStamp = true;
	  }
	  else
	  {
	    if(!dps.receivedSecondTimeStamp)
	    dps.receivedSecondTimeStamp = true;
	    else
	    {
		dps::PressureReading pr;
		dps.getPressureFromString(dps.readBuffer, pr);
		_pressure.write(pr);

		//dps.printPressure(pr);
		//_dps_speeds.write(speeds);
		base::samples::RigidBodyState rbs;
		base::Vector3d pos;
		rbs.time = base::Time::now();
		// use rbs to output pressure. x and y fields are set to zero
		pos[0] = pos[1] = 0.0;
		// z coordinate is set to depth in meters (directly calculated from pressure, 1m = 0.1bar 
		dps::PressureReading mean_pressure;
		if(smoothReading(pr, mean_pressure))
		    pos[2] = (mean_pressure.pressure-1.0) * -10.0;
		else
		    pos[2] = (pr.pressure-1.0) * -10.0;
		rbs.position = pos;
		// covariance for pressure
		rbs.cov_position = base::samples::RigidBodyState::setValueUnknown();
		rbs.cov_position(2,2) = _pressure_covariance;
		_depth_samples.write(rbs);
	    }
	  }
	}
      }
    }
  }
}

// void Task::errorHook()
// {
// }
// void Task::stopHook()
// {
// }
// void Task::cleanupHook()
// {
// }

