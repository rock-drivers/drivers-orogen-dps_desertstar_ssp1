#include "Task.hpp"

#include "../../../drivers/dps/src/dps.h"
#include <rtt/FileDescriptorActivity.hpp>
using namespace dps;


RTT::FileDescriptorActivity* Task::getFileDescriptorActivity()
{ return dynamic_cast< RTT::FileDescriptorActivity* >(getActivity().get()); }


Task::Task(std::string const& name)
    : TaskBase(name)
{
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

 bool Task::configureHook()
 {
    char sys_stty_cmd[100];
    sprintf(sys_stty_cmd,"stty -F %s 4800 cs8", _device.get().c_str());
    system(sys_stty_cmd);
    
    // Open the device that is listed in the _device string property
    dps.comPort = open(_device.get().c_str(), O_RDONLY);
    if (dps.comPort == -1)
        return false;

    getFileDescriptorActivity()->watch(dps.comPort);
    getFileDescriptorActivity()->setTimeout(500);
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
   
  RTT::FileDescriptorActivity* fd_activity = getFileDescriptorActivity();
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
	      //std::cerr << "now getting curr speeds from this string:\n\n" << dps.readBuffer << "\n\n this was the string\n";
	      dps.getPressureFromString(dps.readBuffer, pr);
	      //std::cerr << dps.readBuffer << std::endl;
	      //dps.printPressure(pr);
	      //_dps_speeds.write(speeds);
	      _pressure.write(pr);
	      //CvMat img = dps.getSpeedVis(speeds);
	      //cvShowImage("DVL", &img);
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

