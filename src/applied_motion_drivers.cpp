/*
 *  Copyright (C) 2017 Vision-Guided and Intelligent Robotics Lab
 *  Written by Ali Shafiekhani <Ashafiekhani@mail.missouri.edu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation. Meaning:
 *          keep this copyright notice,
 *          do  not try to make money out of it,
 *          it's distributed WITHOUT ANY WARRANTY,
 *
 *  You can get a copy of the GNU General Public License by writing to
 *  the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *  Boston, MA  02111-1307  USA
 */

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

 // Constructor
 AppliedMotionDrivers::AppliedMotionDrivers(int model_no, int firmware_ver, int baudrate)
 	:	model_no_(model_no), 
 	 	firmware_ver_(firmware_ver), 
 	 	baudrate_(baudrate){
 	initDriver();
 }

// Destructor
 AppliedMotionDrivers::~AppliedMotionDrivers(){
 	driver_serial_.close();
 }


std::vector<std::string> AppliedMotionDrivers::findDriver()
{
	std::vector<std::string> rs232_ports;
    std::vector<serial::PortInfo> devices_found = serial::list_ports();
    std::vector<serial::PortInfo>::iterator iter = devices_found.begin();
    std::cout << "Scanning All Ports with Hardware ID PNP0501 ..." << std::endl;
    while( iter != devices_found.end() )
    {
        serial::PortInfo device = *iter++;
        if(device.hardware_id == "PNP0501"){
        	rs232_ports.push_back(device.port.c_str());
        	std::cout << device.port.c_str() << ", ";
        }
    }
    std::cout << std::endl;
    return rs232_ports;
}

void AppliedMotionDrivers::initDriver()
{
	std::vector<std::string> rs232_ports = findDriver();
	// Applied Motion Drivers: 
	// default baudrate = 9600
	// 1 start bit, 8 data bits, 0(no) parity bits, and 1 stop bit

	for(std::vector<std::string>::iterator it = rs232_ports.begin(); it != rs232_ports.end(); ++it){
		std::string port = *it;
		driver_serial_.setPort(port);
		driver_serial_.setBaudrate(baudrate_);
		
		driver_serial_.setTimeout(serial::Timeout::max(), 100, 0, 200, 0);
		driver_serial_.setBytesize(serial::bytesize_t(8));
		driver_serial_.setParity(serial::parity_t(0));		
		driver_serial_.setStopbits(serial::stopbits_t(1));
		driver_serial_.setFlowcontrol(serial::flowcontrol_t(0));	

		try{
			driver_serial_.open();
		}
		catch(std::exception &e){
			std::cout << "Unable to open " << port_ << std::endl;
		}
		// powerup the driver: host needs to send :
		// (255)(firmware version)(internal model number)(carriage return)

		char command[64];
		int len = sprintf(command, "%d%d%d", 255, firmware_ver_, model_no_);
		command[len] = '\r';
		comSendRecv(command);
		std::string model_revision = getModelRevision();

		if(model_revision.length() >= 7)
		{
			char fw[10], mn[10];	
			sprintf(fw, "%02d",firmware_ver_);
			sprintf(mn, "%03d",model_no_);
			if(model_revision.substr(1,2) == fw && model_revision.substr(4,3) == mn){
				port_ = port;
				std::cout << "Applied Motion Driver Detected on port: " << port << " with firmware and model number: " << model_revision<< std::endl;
				break;
			}
		}
		driver_serial_.close();
	}
}

std::string AppliedMotionDrivers::comSendRecv(char *command){
	driver_serial_.flush();
	size_t bytes_wrote = driver_serial_.write(command);
	std::string result;
	size_t bytes_read = driver_serial_.readline(result, 65536, "\r");
	std::string::size_type sz, p;

	p = result.find('=');
	std::string output;
	if(p != 0){
		std::istringstream buffer(result.substr(p+1));
		buffer >> output;
	}
	else
		output = result;
	
	return output;
}

std::string AppliedMotionDrivers::getModelRevision(){
	// Example command: MV --> returns 106G020 ==> firmware V: 1.06G Model: 20
	char command[64];
	int len = sprintf(command, "%s", "MV");
	command[len] = '\r';
	std::string model_revision = comSendRecv(command);
	return model_revision;
}

void AppliedMotionDrivers::setAccRate(int acceleration_rate){
	// Example command: AC100 --> set acceleration to 100 rev/sec/sec
	char command[64];
	int len = sprintf(command, "%s%d", "AC", acceleration_rate);
	command[len] = '\r';
	comSendRecv(command);
}
int AppliedMotionDrivers::getAccRate(){
	// Example command: AC --> returns "AC=25"
	char command[64];
	int len = sprintf(command, "%s", "AC");
	command[len] = '\r';
	
	std::string acc_rate_str = comSendRecv(command);
	int acc_rate;
	std::istringstream(acc_rate_str) >> acc_rate;
	return acc_rate;
}

int AppliedMotionDrivers::getAlarmCode(){
	// Example command: AL --> returns "AL=0002"
	char command[64];
	int len = sprintf(command, "%s", "AL");
	command[len] = '\r';
	
	std::string alarm_code_str = comSendRecv(command);
	int alarm_code;
	std::istringstream(alarm_code_str) >> std::hex >> alarm_code;
	return alarm_code;
}

void AppliedMotionDrivers::resetAlarm(){
	// Example command: AR --> resets drive fault and clear alram code(if possible)
	char command[64];
	int len = sprintf(command, "%s", "AR");
	command[len] = '\r';
	size_t bytes_wrote = driver_serial_.write(command);
}

void AppliedMotionDrivers::setJogAcc(double jogging_acc){
	char command[64];
	int len = sprintf(command,"%s%.3f", "JA", jogging_acc);
	command[len] = '\r';
	comSendRecv(command);
}

double AppliedMotionDrivers::getJogAcc(){
	char command[64];
	int len = sprintf(command, "%s", "JA");
	command[len] = '\r';
	std::string jog_acc_str = comSendRecv(command);
	double jogging_acc;
	std::istringstream(jog_acc_str) >> jogging_acc;
	return jogging_acc;
}

void AppliedMotionDrivers::setJogSpeed(double jogging_speed){
	char command[64];
	int len = sprintf(command,"%s%.3f", "JS", jogging_speed);
	command[len] = '\r';
	comSendRecv(command);
}

double AppliedMotionDrivers::getJogSpeed(){
	char command[64];
	int len = sprintf(command, "%s", "JS");
	command[len] = '\r';
	std::string jog_sp_str = comSendRecv(command);
	double jogging_speed;
	std::istringstream(jog_sp_str) >> jogging_speed;
	return jogging_speed;
}

void AppliedMotionDrivers::commenceJogging(){
	char command[64];
	int len = sprintf(command, "%s", "CJ");
	command[len] = '\r';
	comSendRecv(command);
}

void AppliedMotionDrivers::changeJogSpeed(double jogging_speed){
	// change jogging speed while moving
	char command[64];
	int len = sprintf(command, "%s%.3f", "CS", jogging_speed);
	command[len] = '\r';
	comSendRecv(command);
}

void AppliedMotionDrivers::stopJogging(){
	// stops jogging with decel rate
	char command[64];
	int len = sprintf(command, "%s", "SJ");
	command[len] = '\r';
	comSendRecv(command);
}