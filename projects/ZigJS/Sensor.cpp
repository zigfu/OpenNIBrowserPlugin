
#include "Sensor.h"
#include "SensorKinectSDK.h"
#include "SensorOpenNI.h"


SensorPtr Sensor::CreateSensor() {
#ifdef _WIN32 
	return SensorPtr(new SensorKinectSDK);
#else
	return SensorPtr(new SensorOpenNI);
#endif
}

Sensor::~Sensor() {}
