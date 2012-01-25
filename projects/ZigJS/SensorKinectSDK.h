
#ifndef __SensorKinectSDK_h__
#define __SensorKinectSDK_h__
#ifdef _WIN32
#include "PluginCore.h"
#include "json/json.h"
#include "Sensor.h"

class INuiSensor;
FB_FORWARD_PTR(INuiSensor);

class SensorKinectSDK : public Sensor {
public:
	SensorKinectSDK();
	virtual ~SensorKinectSDK();

	virtual bool ReadFrame(); //true if there is new data, false otherwise

	virtual bool Valid() const;
	virtual boost::shared_ptr< FB::variant > GetImageBase64() const;
	virtual const std::string& GetEventData() const;
private:
	INuiSensorPtr m_sensor;

	bool m_initialized;
	bool m_error;

	std::string m_lastFrameData;
	Json::FastWriter m_writer;
};
#endif
#endif
