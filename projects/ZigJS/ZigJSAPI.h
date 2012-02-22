/**********************************************************\

  Auto-generated ZigJSAPI.h

\**********************************************************/

#ifndef H_ZigJSAPI
#define H_ZigJSAPI

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "ZigJS.h"
#include "Watermark.h"
#include "Timer.h"

class ZigJSAPI : public FB::JSAPIAuto
{
public:
	

    ZigJSAPI(const ZigJSPtr& plugin, const FB::BrowserHostPtr& host);
    virtual ~ZigJSAPI();

    ZigJSPtr getPlugin();
	//////////////// DON'T USE FROM OUTSIDE ZigJSAPI!!! (unless you know what you doing (take off every zig!))
	//////////////// These APIs don't check against the watermark!
    // Event helpers
	FB_JSAPI_EVENT(WaveGesture, 3, (float,float,float));
	FB_JSAPI_EVENT(HandCreate, 5, (int,float,float,float,float));
	FB_JSAPI_EVENT(HandUpdate, 5, (int,float,float,float,float));
	FB_JSAPI_EVENT(HandDestroy, 2, (int,float));

	FB_JSAPI_EVENT(UserEntered, 1, (int));
	FB_JSAPI_EVENT(UserLeft, 1, (int));
	FB_JSAPI_EVENT(UserTrackingStarted, 1, (int));
	FB_JSAPI_EVENT(UserTrackingStopped, 1, (int));

	//FB_JSAPI_EVENT(NewFrame, 2,(const FB::variant&, const FB::variant&));
	FB_JSAPI_EVENT(NewFrame, 1,(const std::string&));
	//FB_JSAPI_EVENT(NewFrame, 0, ());
	FB_JSAPI_EVENT(StatusChange, 1, (bool));
	void onHandCreate(int handId, float x, float y, float z, float time);
	void onHandUpdate(int handId, float x, float y, float z, float time);
	void onHandDestroy(int handId, float time);

	void setHands(const FB::VariantList& hands);
	void setUsers(const FB::VariantList& users);

	void onUserEntered(int userId);
	void onUserLeft(int userId);
	void onUserTrackingStarted(int userId);
	void onUserTrackingStopped(int userId);
	///////////////// END OF DON'T USE
	void onStatusChange(bool connected);
	//void onNewFrame(const FB::variant& users, const FB::variant& hands);
	void onNewFrame(const std::string& out);
	void setDepthMap(const FB::variant& depthMap);
	void setImageMap(const FB::variant& imageMap);

	FB::VariantList convertWorldToImageSpace(std::vector<double>& points);
	FB::VariantList convertImageToWorldSpace(std::vector<double>& points);

private:
	ZigJSWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;
	Watermark m_watermark;
	FB::JSObjectPtr get_Watermark() const { return m_watermark.GetElement(); }

	bool get_firingEvents();
	void set_firingEvents(const bool firingEvents);
	bool firingEvents;

	bool get_sensorConnected();

	void requestStreams(bool updateDepth, bool updateImage, bool isWebplayer);

	void Invalidate() { m_watermark.Invalidate(); }
	bool Test() {return m_watermark.Test(); }
	void Validate(int key) { m_watermark.Validate(key); }

	FB::TimerPtr m_watermarkTimer;
	void WatermarkTimerCB();
	void WatermarkTest();
	//bool showWatermark();

	//TODO: unhack
//public:
//	void setImage(FB::JSAPIPtr img);
//	FB::JSAPIPtr getImage() const;
//private:
//	FB::JSAPIPtr m_image;
//	mutable boost::recursive_mutex m_imageMutex;

};

#endif // H_ZigJSAPI

