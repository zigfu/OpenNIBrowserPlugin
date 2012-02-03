
#include "Sensor.h"
#include "SensorKinectSDK.h"
#include "SensorOpenNI.h"


SensorPtr Sensor::CreateSensor() {
	//try opening KinectSDK, then OpenNI
	SensorPtr result;
	if (SensorKinectSDK::Available()) {
		result = SensorKinectSDK::GetInstance();
		if ((result) && (result->Valid())) {
			return result;
		}
	}
	if (SensorOpenNI::Available()) {
		result = SensorPtr(new SensorOpenNI);
	}
	return result;
}

Sensor::~Sensor() {}

void Sensor::Unload()
{
	SensorOpenNI::Unload();
	SensorKinectSDK::Unload();
}