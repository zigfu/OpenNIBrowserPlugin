
#ifndef __SensorOpenNI_h__
#define __SensorOpenNI_h__

#include <XnOpenNI.h>
#include <XnLog.h>
#include <XnCppWrapper.h>

#include "PluginCore.h"
#include "json/json.h"
#include "Timer.h"

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
class SensorOpenNI {
public:
	SensorOpenNI();
	virtual ~SensorOpenNI();

	bool ReadFrame(); //true if there is new data, false otherwise

	bool Valid() const;
	boost::shared_ptr< FB::variant > GetImageBase64() const;
	const std::string& GetEventData() const;
private:
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
	xn::SceneMetaData m_sceneMD;
	xn::DepthMetaData m_depthMD;
	std::string m_eventData;

	// these are basically a lazily-evaluated and cached variable accessed through GetImageBase64
	mutable bool m_gotImage;
	mutable boost::shared_ptr< FB::variant > m_imageData;

	XnUInt64 m_lastNewDataTime;
private:
	XN_THREAD_HANDLE m_threadHandle;
	xn::Context m_context;
	xn::Device m_device;
	xn::DepthGenerator m_depth;
	xn::GestureGenerator m_gestures;
	xn::HandsGenerator m_hands;
	xn::UserGenerator m_users;
	volatile bool m_initialized;
	volatile bool m_error;
	XnLicense m_license;
	
	static void XN_CALLBACK_TYPE GestureRecognizedHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie);
	static void XN_CALLBACK_TYPE HandCreateHandler(xn::HandsGenerator& generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE HandUpdateHandler(xn::HandsGenerator& generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE HandDestroyHandler(xn::HandsGenerator& generator, XnUserID user, XnFloat fTime, void* pCookie);

	// User callbacks
	static void XN_CALLBACK_TYPE OnNewUser(xn::UserGenerator& generator, const XnUserID nUserId, void* pCookie);
	static void XN_CALLBACK_TYPE OnLostUser(xn::UserGenerator& generator, const XnUserID nUserId, void* pCookie);
	static void XN_CALLBACK_TYPE OnPoseDetected(xn::PoseDetectionCapability& poseDetection, const XnChar* strPose, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE OnCalibrationStart(xn::SkeletonCapability& skeleton, const XnUserID nUserId, void* pCookie);
	static void XN_CALLBACK_TYPE OnCalibrationEnd(xn::SkeletonCapability& skeleton, const XnUserID nUserId, XnBool bSuccess, void* pCookie);


	// implementations - attached to an actual object instance
	// UI callbacks
	void GestureRecognizedHandlerImpl(const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition);
	void HandCreateHandlerImpl(XnUserID user, const XnPoint3D* pPosition, XnFloat fTime);
	void HandUpdateHandlerImpl(XnUserID user, const XnPoint3D* pPosition, XnFloat fTime);
	void HandDestroyHandlerImpl(XnUserID user, XnFloat fTime);

	// User callbacks
	void OnNewUserImpl(const XnUserID nUserId);
	void OnLostUserImpl(const XnUserID nUserId);
	void OnPoseDetectedImpl(xn::PoseDetectionCapability& poseDetection, const XnChar* strPose, XnUserID nId);
	void OnCalibrationStartImpl(const XnUserID nUserId);
	void OnCalibrationEndImpl(xn::SkeletonCapability& skeleton, const XnUserID nUserId, XnBool bSuccess);

	static void XN_CALLBACK_TYPE ErrorCallback(XnStatus errorState, void *pCookie);
	XnCallbackHandle m_errorCB;
};


#endif
