
#include "Sensor.h"
#include "SensorKinectSDK.h"
#include "SensorOpenNI.h"


SensorPtr Sensor::CreateSensor() {
	//try opening KinectSDK, then OpenNI
	SensorPtr result;
	if (SensorKinectSDK::Available()) {
		result = SensorPtr(new SensorKinectSDK);
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
