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

class ZigJSAPI : public FB::JSAPIAuto
{
public:
	
//	void XN_CALLBACK_TYPE GestureRecognizedHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie);

    ZigJSAPI(const ZigJSPtr& plugin, const FB::BrowserHostPtr& host);
    virtual ~ZigJSAPI();

    ZigJSPtr getPlugin();

    // Event helpers
	FB_JSAPI_EVENT(WaveGesture, 3, (float,float,float));
	FB_JSAPI_EVENT(HandCreate, 5, (int,float,float,float,float));
	FB_JSAPI_EVENT(HandUpdate, 5, (int,float,float,float,float));
	FB_JSAPI_EVENT(HandDestroy, 2, (int,float));

	FB_JSAPI_EVENT(UserEntered, 1, (int));
	FB_JSAPI_EVENT(UserLeft, 1, (int));
	FB_JSAPI_EVENT(UserTrackingStarted, 1, (int));
	FB_JSAPI_EVENT(UserTrackingStopped, 1, (int));

	FB_JSAPI_EVENT(NewFrame, 0,());

	void onHandCreate(int handId, float x, float y, float z, float time);
	void onHandUpdate(int handId, float x, float y, float z, float time);
	void onHandDestroy(int handId, float time);

	void setHands(const FB::VariantList& hands);
	void setUsers(const FB::VariantList& users);

	void onUserEntered(int userId);
	void onUserLeft(int userId);
	void onUserTrackingStarted(int userId);
	void onUserTrackingStopped(int userId);
private:
	ZigJSWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

	//TODO: unhack
public:
	void setImage(FB::JSAPIPtr img);
	FB::JSAPIPtr getImage() const;
private:
	FB::JSAPIPtr m_image;
	mutable boost::recursive_mutex m_imageMutex;

};

#endif // H_ZigJSAPI

