
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

	virtual bool ReadFrame(bool updateDepth, bool updateImage, bool updateLabelMap) = 0; //true if there is new data, false otherwise

	virtual bool Valid() const = 0;

	virtual const std::string& GetEventData() const = 0;

	// points are in/out, every three doubles are treated as x,y,z coordinates and modified in-place
	virtual FB::VariantList convertWorldToImageSpace(const std::vector<double>& points) = 0;
	virtual FB::VariantList convertImageToWorldSpace(const std::vector<double>& points) = 0;

	virtual void StopTrackingPoint(int pointId) {}
	virtual void SetHandRaise(bool enable) {}

	//virtual const std::wstring& GetImage() {return std::wstring();}
	// TODO: refactor
	const FB::variant& GetImage() const { return m_imageJS; }
	const FB::variant& GetDepth() const { return m_depthJS; }
	const FB::variant& GetLabelMap() const { return m_labelMapJS; }
	//const std::wstring& GetImage() const { return m_imageBuffer; }
	//const std::wstring& GetDepth() const { return m_depthBuffer; }
	Sensor() {
		//m_depthBuffer.resize(MAP_XRES*MAP_YRES*2); //16-bpp
		//m_imageBuffer.resize(MAP_XRES*MAP_YRES*3); //24-bpp
		m_depthBuffer.resize(MAP_XRES*MAP_YRES*2*4/3); //16-bpp, base64
		m_labelMapBuffer.resize(MAP_XRES*MAP_YRES*2*4/3); //implicit assumption that the division by 3 will be round, which works because of the aspect ratio (4x3)
		
		m_imageBuffer.resize(MAP_XRES*MAP_YRES*4); //24-bpp, base64
	}
	static void Unload();
	static void Init();
protected:
	FB::variant m_imageJS;
	FB::variant m_depthJS;
	FB::variant m_labelMapJS;
	std::string m_imageBuffer;
	std::string m_depthBuffer;
	std::string m_labelMapBuffer;


	//base64 stuff
	//TODO: move to separate file
	static const char* base64_charset;
	inline static void b64_encode_triplet(std::string& out, int pos, unsigned char d1, unsigned char d2, unsigned char d3)
	{
		out[pos] = base64_charset[d1 >> 2];
		out[pos+1] = base64_charset[((d1 & 0x03) << 4) | (d2 >> 4)]; 
		out[pos+2] = base64_charset[((d2 & 0x0f) << 2) | (d3 >> 6)];
		out[pos+3] = base64_charset[d3 & 0x3f];
	}

};


#endif
