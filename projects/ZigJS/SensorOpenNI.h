
#ifndef __SensorOpenNI_h__
#define __SensorOpenNI_h__

#include <XnOpenNI.h>
#include <XnLog.h>
#include <XnCppWrapper.h>

#include "PluginCore.h"
#include "json/json.h"
#include "Sensor.h"

//TODO: move away from header?
class HandPoint
{
public:
	int handid;
	int userid;
	XnPoint3D position;
	XnPoint3D focusposition;

	HandPoint(int handid, int userid, XnPoint3D position, XnPoint3D focusposition)
		: handid(handid), userid(userid), position(position), focusposition(focusposition)
	{}

};

//TODO: inherit from some Sensor interface
class SensorOpenNI : public Sensor {
public:
	static SensorPtr GetInstance();
	static bool Available(); // installed? is there a sensor connected using this API?

	SensorOpenNI();
	virtual ~SensorOpenNI();

	bool ReadFrame(bool updateDepth, bool updateImage, bool updateLabelMap); //true if there is new data, false otherwise

	bool Valid() const;
	//boost::shared_ptr< FB::variant > GetImageBase64() const;
	const std::string& GetEventData() const;
	virtual FB::VariantList convertWorldToImageSpace(const std::vector<double>& points);
	virtual FB::VariantList convertImageToWorldSpace(const std::vector<double>& points);
	static void Unload();
	static bool Init();
private:

	static SensorPtr s_activeInstance;

	FB::VariantList GetJointsList(XnUserID userid);
	FB::VariantList PositionToVariant(XnPoint3D pos);
	FB::VariantList OrientationToVariant(XnMatrix3X3 ori);
	FB::VariantList MakeUsersList();
	FB::VariantList MakeHandsList();

	Json::Value GetJointsJsonList(XnUserID userid);
	Json::Value PositionToValue(XnPoint3D pos);
	Json::Value OrientationToValue(XnMatrix3X3 ori);
	Json::Value MakeUsersJsonList();
	Json::Value MakeHandsJsonList();

	XnUserID WhichUserDoesThisPointBelongTo(XnPoint3D point);

	// we keep a list of hand points
	std::list<HandPoint> m_handpoints;

	Json::FastWriter m_writer;

	//TODO: refactor these members into "result" object that encapsulates the per-frame data
	// (it'll make a lot of code make more sense (e.g. memoized getImage thingy)
	int m_lastFrame;
	XnSceneMetaData *m_pSceneMD;
	XnDepthMetaData *m_pDepthMD;
	XnImageMetaData *m_pImageMD;
	
	std::string m_eventData;

	// these are basically a lazily-evaluated and cached variable accessed through GetImageBase64
	//mutable bool m_gotImage;
	//mutable boost::shared_ptr< FB::variant > m_imageData;

	XnUInt64 m_lastNewDataTime;
private:
	XnContext* m_pContext;
	XnNodeHandle m_device;
	XnNodeHandle m_depth;
	XnNodeHandle m_gestures;
	XnNodeHandle m_hands;
	XnNodeHandle m_users;
	XnNodeHandle m_image;
	volatile bool m_initialized;
	volatile bool m_error;
	volatile bool m_imageOk;
	XnLicense m_license;
	
	static void XN_CALLBACK_TYPE GestureRecognizedHandler(XnNodeHandle generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie);
	static void XN_CALLBACK_TYPE HandCreateHandler(XnNodeHandle generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE HandUpdateHandler(XnNodeHandle generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE HandDestroyHandler(XnNodeHandle generator, XnUserID user, XnFloat fTime, void* pCookie);

	// User callbacks
	static void XN_CALLBACK_TYPE OnNewUser(XnNodeHandle generator, const XnUserID nUserId, void* pCookie);
	static void XN_CALLBACK_TYPE OnLostUser(XnNodeHandle generator, const XnUserID nUserId, void* pCookie);
	static void XN_CALLBACK_TYPE OnPoseDetected(XnNodeHandle poseDetection, const XnChar* strPose, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE OnCalibrationStart(XnNodeHandle skeleton, const XnUserID nUserId, void* pCookie);
	static void XN_CALLBACK_TYPE OnCalibrationEnd(XnNodeHandle skeleton, const XnUserID nUserId, XnBool bSuccess, void* pCookie);


	// implementations - attached to an actual object instance
	// UI callbacks
	void GestureRecognizedHandlerImpl(const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition);
	void HandCreateHandlerImpl(XnUserID user, const XnPoint3D* pPosition, XnFloat fTime);
	void HandUpdateHandlerImpl(XnUserID user, const XnPoint3D* pPosition, XnFloat fTime);
	void HandDestroyHandlerImpl(XnUserID user, XnFloat fTime);

	// User callbacks
	void OnNewUserImpl(const XnUserID nUserId);
	void OnLostUserImpl(const XnUserID nUserId);
	void OnPoseDetectedImpl(XnNodeHandle poseDetection, const XnChar* strPose, XnUserID nId);
	void OnCalibrationStartImpl(const XnUserID nUserId);
	void OnCalibrationEndImpl(XnNodeHandle skeleton, const XnUserID nUserId, XnBool bSuccess);

	static void XN_CALLBACK_TYPE ErrorCallback(XnStatus errorState, void *pCookie);
	XnCallbackHandle m_errorCB;
};


#endif
