
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
	static bool Available(); // installed? is there a sensor connected using this API?
	SensorKinectSDK();
	virtual ~SensorKinectSDK();

	virtual bool ReadFrame(bool updateDepth, bool updateImage, bool isWebplayer); //true if there is new data, false otherwise

	virtual bool Valid() const;
	virtual const std::string& GetEventData() const;

private:
	static bool Init();
	static void Unload();
	INuiSensorPtr m_sensor;

	bool m_initialized;
	bool m_error;

	std::string m_lastFrameData;
	Json::FastWriter m_writer;
};
#else // non-windows - dummy object
class SensorKinectSDK : public Sensor {
public:
	static bool Available() { return false; } // installed? is there a sensor connected using this API?
	virtual ~SensorKinectSDK() {}

	virtual bool ReadFrame(bool updateDepth, bool updateImage, bool isWebplayer) { return false; } //true if there is new data, false otherwise

	virtual bool Valid() const { return false; }
	virtual boost::shared_ptr< FB::variant > GetImageBase64() const { return boost::shared_ptr< FB::variant >(); }
	virtual const std::string& GetEventData() const { return std::string(); }
};
#endif
#endif
