#pragma once

#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define RECEIVE_TIMEOUT         (20000000) 
#define INVALID_RANGE_16BIT     ((signed short)0x8000)          // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range data. 
#define DOUBLE_MAX              ((k64f)1.7976931348623157e+308) // 64-bit double - largest positive value.  
#define INVALID_RANGE_DOUBLE    ((k64f)-DOUBLE_MAX)             // floating point value to represent invalid range data.    
#define SENSOR_IP               "192.168.1.10"                      

#define NM_TO_MM(VALUE) (((k64f)(VALUE))/1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE))/1000.0)

typedef struct
{
	double x;   // x-coordinate in engineering units (mm) - position along laser line
	double z;   // z-coordinate in engineering units (mm) - height (at the given x position)
	unsigned char intensity;
}ProfilePoint;

class NewReceiveProfile
{
public:
	NewReceiveProfile();
	~NewReceiveProfile();

	void SetStartCamera();

	void SetStopCamera();

	void ReflushCamer();

	kAssembly api = kNULL;
	kStatus status;
	unsigned int i, j, k, arrayIndex;
	GoSystem system = kNULL;
	GoSensor sensor = kNULL;
	GoDataSet dataset = kNULL;
	ProfilePoint* profileBuffer = NULL;
	GoStamp* stamp = kNULL;
	GoDataMsg dataObj;
	kIpAddress ipAddress;
	GoSetup setup = kNULL;
	k32u profilePointCount;

	CString NewProTemp, NEwProOut = 0;
};

