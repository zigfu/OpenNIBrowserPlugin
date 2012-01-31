
#ifndef __Sensor_h__
#define __Sensor_h__


#include "PluginCore.h"
#include "json/json.h"

FB_FORWARD_PTR(Sensor);

class Sensor {
public:
	static const int MAP_XRES = 160;
	static const int MAP_YRES = 120;
	static SensorPtr CreateSensor();
	virtual ~Sensor();

	virtual bool ReadFrame(bool updateDepth, bool updateImage, bool isWebplayer) = 0; //true if there is new data, false otherwise

	virtual bool Valid() const = 0;

	virtual const std::string& GetEventData() const = 0;
	//virtual const std::wstring& GetImage() {return std::wstring();}
	// TODO: refactor
	const FB::variant& GetImage() const { return m_imageJS; }
	const FB::variant& GetDepth() const { return m_depthJS; }
	//const std::wstring& GetImage() const { return m_imageBuffer; }
	//const std::wstring& GetDepth() const { return m_depthBuffer; }
	Sensor() {
		m_depthBuffer.resize(MAP_XRES*MAP_YRES*2); //16-bpp
		m_imageBuffer.resize(MAP_XRES*MAP_YRES*3); //24-bpp
	}
protected:
	FB::variant m_imageJS;
	FB::variant m_depthJS;
	std::wstring m_imageBuffer;
	std::wstring m_depthBuffer;


};


#endif
