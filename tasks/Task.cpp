#include "Task.hpp"

#include <rtt/NonPeriodicActivity.hpp>
#include "../../../drivers/dps/src/dps.h"

using namespace dps;


RTT::NonPeriodicActivity* Task::getNonPeriodicActivity()
{ return dynamic_cast< RTT::NonPeriodicActivity* >(getActivity().get()); }


Task::Task(std::string const& name)
    : TaskBase(name)
{
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

 bool Task::configureHook()
 {
     return true;
 }
 bool Task::startHook()
 {
	dps.openComPort("/dev/ttyS0");
	dps.flushPort(0);
        dps.the_file_stream.open("dps.txt");
     return true;
 }

 void Task::updateHook()
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
		dps.printPressure(pr);
		//_dps_speeds.write(speeds);
		_pressure.write(pr);
		//CvMat img = dps.getSpeedVis(speeds);
		//cvShowImage("DVL", &img);
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

