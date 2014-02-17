#include "Task.hpp"

#include <dps/dps.h>
#include <rtt/extras/FileDescriptorActivity.hpp>
using namespace desertstar_ssp1;

Task::Task(std::string const& name)
    : TaskBase(name)
{
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
              //dps.printPressure(pr);
	      //_dps_speeds.write(speeds);
	      base::samples::RigidBodyState rbs;
	      base::Vector3d pos;
	      rbs.time = base::Time::now();
              // use rbs to output pressure. x and y fields are set to zero
	      pos[0] = pos[1] = 0.0;
              // z coordinate is set to depth in meters (directly calculated from pressure, 1m = 0.1bar 
	      pos[2] = (pr.pressure-1.0) * -10.0;
	      rbs.position = pos;
              // covariance for pressure
	      rbs.cov_position(2,2) = _pressure_covariance;
	      _depth_samples.write(rbs);
	      _pressure.write(pr);
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

