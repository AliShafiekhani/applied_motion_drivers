#ifndef APPLIED_MOTION_Drivers_H
#define APPLIED_MOTION_Drivers_H

 class AppliedMotionDrivers{
 private:
 	serial::Serial driver_serial_;
 	std::string port_;
 	unsigned long baudrate_;
 	unsigned long timeout_;
 	int model_no_;
 	int firmware_ver_;

 public:
 	AppliedMotionDrivers(int, int, int boadrate = 9600);
 	~AppliedMotionDrivers();
 	std::vector<std::string> findDriver();
 	void initDriver();
 	std::string getModelRevision();
 	std::string comSendRecv(char *);
 	void setAccRate(int);
 	int getAccRate();
 	int getAlarmCode();
 	void resetAlarm();
 	void setJogAcc(double);
 	double getJogAcc();
 	void setJogSpeed(double);
 	double getJogSpeed();
 	void commenceJogging();
 	void changeJogSpeed(double);
 	void stopJogging();
 };

#endif