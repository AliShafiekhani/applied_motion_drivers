#include <string>
#include <iostream>
#include <cstdio>

 // OS Specific sleep
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// serial library by William Woodall
// See https://github.com/wjwwood/serial for documentation
#include "serial/serial.h"

#include "applied_motion_drivers.h"


int main(int argc, char** argv){
	AppliedMotionDrivers my_driver(20, 6);
	std::cout << "acceleration rate: " << my_driver.getAccRate() << std::endl;
	std::cout << "alarm code : " << my_driver.getAlarmCode() << std::endl;
	
	return 0;
}