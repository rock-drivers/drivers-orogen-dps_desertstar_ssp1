#include "Task.hpp"

#include <dps/dps.h>
#include <rtt/extras/FileDescriptorActivity.hpp>
using namespace desertstar_ssp1;

double median(std::deque<double> &v)
{
    int n = v.size() / 2;
    nth_element(v.begin(), v.begin()+n, v.end());
    return v[n];
}


std::deque<double> dps_readings;


int window_width = 7;

Task::Task(std::string const& name)
    : TaskBase(name)
{
	IIR = 0.0;
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

 bool Task::configureHook()
 {
    RTT::extras::FileDescriptorActivity* activity =
  getActivity<RTT::extras::FileDescriptorActivity>();

    window_width = _filter_width;

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
	      //std::cerr << "now getting curr speeds from this string:\n\n" << dps.readBuffer << "\n\n this was the string\n";
	      dps.getPressureFromString(dps.readBuffer, pr);
	      //std::cerr << dps.readBuffer << std::endl;
//	      dps.printPressure(pr);
	      //_dps_speeds.write(speeds);
	      base::samples::RigidBodyState rbs;
	      base::Vector3d pos;
	      rbs.time = base::Time::now();
	      pos[0] = pos[1] = 0.0;
	      pos[2] = (pr.pressure-1.0) * 10.0;
	      rbs.position = pos;
	      rbs.cov_position(2,2) = 0.0216505327374;
	      _depth_samples.write(rbs);
	      _pressure.write(pr);
IIR = IIR * 0.95 + pr.pressure * 0.05;
//cout << "pressure (iir): " << IIR << "\n";

dps_readings.push_back(pr.pressure);

//std::cerr << "back-pushed " << pr.pressure << std::endl;

if(dps_readings.size() > window_width)
    dps_readings.pop_front();

std::deque<double> sort_deque = dps_readings;

double sum = 0;
for(int i = 0 ; i < sort_deque.size() ; i++)
    sum += sort_deque[i];

double moving_avg = sum / sort_deque.size();

std::cerr 
<< "raw = " << pr.pressure << "median = " << median(sort_deque) << ", moving avg = " << moving_avg << " "
 << std::endl;

              _pressure_double.write(moving_avg);

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

