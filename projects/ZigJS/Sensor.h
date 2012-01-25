
#ifndef __Sensor_h__
#define __Sensor_h__


#include "PluginCore.h"
#include "json/json.h"

FB_FORWARD_PTR(Sensor);

class Sensor {
public:
	static SensorPtr CreateSensor();

	virtual ~Sensor();

	virtual bool ReadFrame() = 0; //true if there is new data, false otherwise

	virtual bool Valid() const = 0;
	virtual boost::shared_ptr< FB::variant > GetImageBase64() const = 0;
	virtual const std::string& GetEventData() const = 0;
};


#endif
