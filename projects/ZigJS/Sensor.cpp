
#include "Sensor.h"
#include "SensorKinectSDK.h"
#include "SensorOpenNI.h"


SensorPtr Sensor::CreateSensor() {
	//TODO: separate SDK availability + initialization from
	//      the sensor read object instead of static functions
	//try opening KinectSDK, then OpenNI
	SensorPtr result;
	if (SensorKinectSDK::Available()) {
		result = SensorKinectSDK::GetInstance();
		if ((result) && (result->Valid())) {
			return result;
		}
	}
	if (SensorOpenNI::Available()) {
		result = SensorOpenNI::GetInstance();
	}
	return result;
}

Sensor::~Sensor() {}

void Sensor::Unload()
{
	if (SensorOpenNI::Available()) SensorOpenNI::Unload();
	if (SensorKinectSDK::Available()) SensorKinectSDK::Unload();
}

void Sensor::Init()
{
	if (SensorOpenNI::Available()) SensorOpenNI::Init();
	if (SensorKinectSDK::Available()) SensorKinectSDK::Init();
}