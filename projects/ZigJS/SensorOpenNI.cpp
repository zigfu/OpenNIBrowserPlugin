
#include "SensorOpenNI.h"
#include "logging.h"
#include "boost/bind.hpp"
#include <XnUSB.h>

using namespace boost::assign;

const int MAX_USERS = 16;
const char * RAISE_HAND = "RaiseHand";

SensorPtr SensorOpenNI::s_activeInstance;
SensorPtr SensorOpenNI::GetInstance()
{
	//TODO: there's probably a race between this and SensorStatusCallback, we just don't really want to mess with this shit,
	// and it'll require extremely unlikely timing
	if (!s_activeInstance || (!s_activeInstance->Valid())) {
		s_activeInstance.reset(); // force d'tor to be called before we call the c'tor
		s_activeInstance.reset(new SensorOpenNI);
	}
	return s_activeInstance; // creates a copy, hurrah!
}
FB::VariantList SensorOpenNI::PositionToVariant(XnPoint3D pos)
{
	FB::VariantList xyz;
	xyz += pos.X, pos.Y, pos.Z;
	return xyz;
}

FB::VariantList SensorOpenNI::OrientationToVariant(XnMatrix3X3 ori)
{
	FB::VariantList result;
	result.push_back(ori.elements[0]);
	result.push_back(ori.elements[3]);
	result.push_back(ori.elements[6]);
	result.push_back(ori.elements[1]);
	result.push_back(ori.elements[4]);
	result.push_back(ori.elements[7]);
	result.push_back(ori.elements[2]);
	result.push_back(ori.elements[5]);
	result.push_back(ori.elements[8]);
	//result.assign(ori.elements, ori.elements + 9);
	return result;
}

FB::VariantList SensorOpenNI::GetJointsList(XnUserID userid)
{
	FB::VariantList result;
	XnSkeletonJointTransformation jointData;

	// quick out if not tracking
	if (!xnIsSkeletonTracking(m_users, userid)) {
		return result;
	}

	// head is the first, right foot the last. probably not the best way.
	for (int i=XN_SKEL_HEAD; i<= XN_SKEL_RIGHT_FOOT; i++) {
		if (xnIsJointAvailable(m_users, (XnSkeletonJoint)i)) {
			xnGetSkeletonJoint(m_users, userid, (XnSkeletonJoint)i, &jointData);
			FB::VariantMap joint;
			joint["id"] = i;
			joint["position"] = PositionToVariant(jointData.position.position);
			joint["rotation"] = OrientationToVariant(jointData.orientation.orientation);
			joint["positionconfidence"] = jointData.position.fConfidence;
			joint["rotationconfidence"] = jointData.orientation.fConfidence;
			result.push_back(joint);
		}
	}

	return result;
}

FB::VariantList SensorOpenNI::MakeUsersList()
{
	// get the users
	XnUInt16 nUsers = MAX_USERS;
	XnUserID aUsers[MAX_USERS];
	xnGetUsers(m_users, aUsers, &nUsers);

	// construct JS object
	FB::VariantList jsUsers;
	XnPoint3D pos;
	for (int i = 0; i < nUsers; i++) {
		xnGetUserCoM(m_users, aUsers[i], &pos);
		FB::VariantMap user;
		user["tracked"] = xnIsSkeletonTracking(m_users, aUsers[i]);
		user["centerofmass"] = PositionToVariant(pos);
		user["id"] = aUsers[i];
		user["joints"] = GetJointsList(aUsers[i]);
		jsUsers += user;
	}

	return jsUsers;
}

FB::VariantList SensorOpenNI::MakeHandsList()
{
	FB::VariantList jsHands;

	for(std::list<HandPoint>::iterator i = m_handpoints.begin(); i != m_handpoints.end(); i++) {
		FB::VariantMap hand;
		hand["id"] = i->handid;
		hand["userid"] = i->userid;
		hand["position"] = PositionToVariant(i->position);
		jsHands += hand;
	}

	return jsHands;
}

//////////JSON
Json::Value SensorOpenNI::PositionToValue(XnPoint3D pos)
{
	Json::Value xyz(Json::arrayValue);
	xyz.resize(3);
	xyz[0u] = pos.X;
	xyz[1u] = pos.Y;
	xyz[2u] = pos.Z;
	return xyz;
}

Json::Value SensorOpenNI::OrientationToValue(XnMatrix3X3 ori)
{
	Json::Value result(Json::arrayValue);
	result.resize(9);
	result[0u] = ori.elements[0];
	result[1u] = ori.elements[3];
	result[2u] = ori.elements[6];
	result[3u] = ori.elements[1];
	result[4u] = ori.elements[4];
	result[5u] = ori.elements[7];
	result[6u] = ori.elements[2];
	result[7u] = ori.elements[5];
	result[8u] = ori.elements[8];
	//result.assign(ori.elements, ori.elements + 9);
	return result;
}

Json::Value SensorOpenNI::GetJointsJsonList(XnUserID userid)
{
	Json::Value result(Json::arrayValue);

	// quick out if not tracking
	if (!xnIsSkeletonTracking(m_users, userid)) {
		return result;
	}

	int jointCount = 0;
	for (int i=XN_SKEL_HEAD; i<= XN_SKEL_RIGHT_FOOT; i++) {
		if (xnIsJointAvailable(m_users, (XnSkeletonJoint)i)) {
			jointCount++;
		}
	}
	result.resize(jointCount);
	XnSkeletonJointTransformation jointData;
	int position = 0;
	// head is the first, right foot the last. probably not the best way.
	for (int i=XN_SKEL_HEAD; i<= XN_SKEL_RIGHT_FOOT; i++) {
		if (xnIsJointAvailable(m_users, (XnSkeletonJoint)i)) {
			xnGetSkeletonJoint(m_users, userid, (XnSkeletonJoint)i, &jointData);
			Json::Value joint(Json::objectValue);
			joint["id"] = i;
			joint["position"] = PositionToValue(jointData.position.position);
			joint["rotation"] = OrientationToValue(jointData.orientation.orientation);
			joint["positionconfidence"] = jointData.position.fConfidence;
			joint["rotationconfidence"] = jointData.orientation.fConfidence;
			result[position] = joint;
			position++;
		}
	}

	return result;
}

Json::Value SensorOpenNI::MakeUsersJsonList()
{
	// get the users
	XnUInt16 nUsers = MAX_USERS;
	XnUserID aUsers[MAX_USERS];
	xnGetUsers(m_users, aUsers, &nUsers);

	// construct JS object
	Json::Value jsUsers(Json::arrayValue);
	//jsUsers.resize(nUsers);
	XnPoint3D pos;
	int outIndex = 0;
	for (int i = 0; i < nUsers; i++) {
		if (m_usersOutOfScene.find(aUsers[i]) != m_usersOutOfScene.end()) continue; // skip 'out of scene' users
		xnGetUserCoM(m_users, aUsers[i], &pos);
		Json::Value user(Json::objectValue);
		user["tracked"] = xnIsSkeletonTracking(m_users, aUsers[i]);
		user["centerofmass"] = PositionToValue(pos);
		user["id"] = aUsers[i];
		user["joints"] = GetJointsJsonList(aUsers[i]);
		jsUsers[outIndex] = user;
		outIndex++;
	}

	return jsUsers;
}

Json::Value SensorOpenNI::MakeHandsJsonList()
{
	Json::Value jsHands(Json::arrayValue);
	jsHands.resize(m_handpoints.size());
	int idx = 0;
	for(std::list<HandPoint>::iterator i = m_handpoints.begin(); i != m_handpoints.end(); i++) {
		Json::Value hand(Json::objectValue);
		hand["id"] = i->handid;
		hand["userid"] = i->userid;
		hand["position"] = PositionToValue(i->position);
		hand["focusposition"] = PositionToValue(i->focusposition);
		jsHands[idx] = hand;
		idx++;
	}

	return jsHands;
}

XnUserID SensorOpenNI::WhichUserDoesThisPointBelongTo(XnPoint3D point)
{
	XnPoint3D proj;
	xnConvertRealWorldToProjective(m_depth, 1, &point, &proj);
	xnGetUserPixels(m_users, 0, m_pSceneMD);
	return m_pSceneMD->pData[(XnUInt32)proj.X + (XnUInt32)proj.Y * m_pSceneMD->pMap->Res.X];
}

void XN_CALLBACK_TYPE SensorOpenNI::GestureRecognizedHandler(XnNodeHandle generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->GestureRecognizedHandlerImpl(strGesture, pIDPosition, pEndPosition);
	}
}
const float MIN_DISTANCE_FROM_EXISTING_POINTS = 300; // 30cm
void SensorOpenNI::GestureRecognizedHandlerImpl(const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition)
{
	if (xnOSStrCmp(strGesture, RAISE_HAND) == 0) {
		// if it's a hand-raise, if the point doesn't belong to a user,  don't start tracking
		if (WhichUserDoesThisPointBelongTo(*pEndPosition) == 0) return;
	}
	// check that the hand point isn't too close to existing points (so as to remove false positives for an already-tracked hand)
	for(std::list<HandPoint>::iterator i = m_handpoints.begin(); i != m_handpoints.end(); i++) {
		XnFloat distanceSqr = (i->position.X - pEndPosition->X)*(i->position.X - pEndPosition->X) +
						(i->position.Y - pEndPosition->Y)*(i->position.Y - pEndPosition->Y) +
						(i->position.Z - pEndPosition->Z)*(i->position.Z - pEndPosition->Z);
		if (distanceSqr < MIN_DISTANCE_FROM_EXISTING_POINTS*MIN_DISTANCE_FROM_EXISTING_POINTS) {
			return; // too close
		}
	}
	xnStartTracking(m_hands, pEndPosition);
}

void XN_CALLBACK_TYPE SensorOpenNI::HandCreateHandler(XnNodeHandle generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->HandCreateHandlerImpl(user, pPosition, fTime);
	}
}
void SensorOpenNI::HandCreateHandlerImpl(XnUserID user, const XnPoint3D* pPosition, XnFloat fTime)
{
	m_handpoints.push_back(HandPoint(user, WhichUserDoesThisPointBelongTo(*pPosition), *pPosition, *pPosition));
}

void XN_CALLBACK_TYPE SensorOpenNI::HandUpdateHandler(XnNodeHandle generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->HandUpdateHandlerImpl(user, pPosition, fTime);
	}
}

void SensorOpenNI::HandUpdateHandlerImpl(XnUserID user, const XnPoint3D* pPosition, XnFloat fTime)
{
	// NOTE: user is actually handid - OpenNI ftl
	for(std::list<HandPoint>::iterator i = m_handpoints.begin(); i != m_handpoints.end(); i++) {
		if (i->handid == user) {
			i->position.X = pPosition->X;
			i->position.Y = pPosition->Y;
			i->position.Z = pPosition->Z;
			// TODO: possibly check for user id again in case its 0
			break;
		}
	}

}

void XN_CALLBACK_TYPE SensorOpenNI::HandDestroyHandler(XnNodeHandle generator, XnUserID user, XnFloat fTime, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->HandDestroyHandlerImpl(user, fTime);
	}
}
void SensorOpenNI::HandDestroyHandlerImpl(XnUserID user, XnFloat fTime)
{
	for(std::list<HandPoint>::iterator i = m_handpoints.begin(); i != m_handpoints.end(); ) {
		if (i->handid == user) {
			m_handpoints.erase(i);
			break;
		} else {
			++i;
		}
	}
}

void XN_CALLBACK_TYPE SensorOpenNI::OnNewUser(XnNodeHandle generator, const XnUserID nUserId, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->OnNewUserImpl(nUserId);
	}
}
void SensorOpenNI::OnNewUserImpl(const XnUserID nUserId)
{
	// just in case, make sure we won't be removing a new user
	m_usersOutOfScene.erase(nUserId);
	if (xnNeedPoseForSkeletonCalibration(m_users)) {
		//std::string
		xnStartPoseDetection(m_users, "Psi", nUserId);
	} else {
		xnStartSkeletonTracking(m_users, nUserId);
	}
}

void XN_CALLBACK_TYPE SensorOpenNI::OnLostUser(XnNodeHandle generator, const XnUserID nUserId, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->OnLostUserImpl(nUserId);
	}
}
void SensorOpenNI::OnLostUserImpl(const XnUserID nUserId)
{
	m_usersOutOfScene.erase(nUserId); // in case sequence of events is New User -> User Exit -> User Lost
}

void XN_CALLBACK_TYPE SensorOpenNI::OnUserExit(XnNodeHandle generator, const XnUserID nUserId, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->OnUserExitImpl(nUserId);
	}
}

void XN_CALLBACK_TYPE SensorOpenNI::OnUserReEnter(XnNodeHandle generator, const XnUserID nUserId, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->OnUserReEnterImpl(nUserId);
	}
}
void SensorOpenNI::OnUserExitImpl(const XnUserID nUserId)
{
	m_usersOutOfScene.insert(nUserId);
}

void SensorOpenNI::OnUserReEnterImpl(const XnUserID nUserId)
{
	m_usersOutOfScene.erase(nUserId);
}



void XN_CALLBACK_TYPE SensorOpenNI::OnPoseDetected(XnNodeHandle poseDetection, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->OnPoseDetectedImpl(poseDetection, strPose, nId);
	}
}
void SensorOpenNI::OnPoseDetectedImpl(XnNodeHandle poseDetection, const XnChar* strPose, XnUserID nId)
{
	// Stop detecting the pose
	xnStopPoseDetection(poseDetection, nId);

	// Start calibrating
	xnRequestSkeletonCalibration(m_users, nId, TRUE);
}

void XN_CALLBACK_TYPE SensorOpenNI::OnCalibrationStart(XnNodeHandle skeleton, const XnUserID nUserId, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->OnCalibrationStartImpl(nUserId);
	}
}
void SensorOpenNI::OnCalibrationStartImpl(const XnUserID nUserId)
{
}

void XN_CALLBACK_TYPE SensorOpenNI::OnCalibrationEnd(XnNodeHandle skeleton, const XnUserID nUserId, XnBool bSuccess, void* pCookie)
{
	SensorOpenNI * instance = reinterpret_cast<SensorOpenNI *>(pCookie);
	if (instance) {
		instance->OnCalibrationEndImpl(skeleton, nUserId, bSuccess);
	}
}
void SensorOpenNI::OnCalibrationEndImpl(XnNodeHandle skeleton, const XnUserID nUserId, XnBool bSuccess)
{
	// If this was a successful calibration
	if (bSuccess) {
		// start tracking
		xnStartSkeletonTracking(skeleton, nUserId);
	} else {
		// Restart pose detection
		xnStartPoseDetection(m_users, "Psi", nUserId);
	}	
}
void XN_CALLBACK_TYPE SensorOpenNI::ErrorCallback(XnStatus errorState, void *pCookie) {
	FBLOG_INFO("errorShit", "global error!");
	if (XN_STATUS_OK != errorState) {
		reinterpret_cast<SensorOpenNI *>(pCookie)->m_error = true;
	}
}

SensorOpenNI::SensorOpenNI() : 
	m_initialized(false), m_error(false), m_imageOk(false),
	m_lastNewDataTime(0xFFFFFFFFFFFFFFFFULL),
	m_pContext(NULL),
	m_pSceneMD(NULL), m_pDepthMD(NULL), m_pImageMD(NULL),
	m_depth(NULL), m_users(NULL), m_device(NULL),
	m_gestures(NULL), m_hands(NULL), m_image(NULL)
{
	//TODO: refactor this function to reduce error checking boilerplate
	XnStatus nRetVal = XN_STATUS_OK;
	nRetVal = xnInit(&m_pContext);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_INFO("xnInit", "fail context init");
		return;
	} else {
		FBLOG_INFO("xnInit", "ok context init");
	}
	
	nRetVal = xnRegisterToGlobalErrorStateChange(m_pContext, &SensorOpenNI::ErrorCallback, this, &m_errorCB);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_INFO("xnInit", "fail register for error callback");
		xnContextRelease(m_pContext);
		m_pContext = NULL;
		return;
	} else {
		FBLOG_INFO("xnInit", "ok register for error callback");
	}
	m_pSceneMD = xnAllocateSceneMetaData();
	m_pDepthMD = xnAllocateDepthMetaData();
	m_pImageMD = xnAllocateImageMetaData();
	xnOSStrCopy(m_license.strKey, "0KOIk2JeIBYClPWVnMoRKn5cdY4=", sizeof(m_license.strKey));
	xnOSStrCopy(m_license.strVendor, "PrimeSense", sizeof(m_license.strVendor));
	xnAddLicense(m_pContext, &m_license);

	nRetVal = xnCreateAnyProductionTree(m_pContext, XN_NODE_TYPE_DEVICE, NULL, &m_device, NULL);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_DEBUG("xnInit", "fail get device");
		m_lastFrame = -6;
		xnContextRelease(m_pContext);
		m_pContext = NULL;
		return;
	} else {
		FBLOG_DEBUG("xnInit", "ok get device");
	}
	nRetVal = xnCreateAnyProductionTree(m_pContext, XN_NODE_TYPE_DEPTH, NULL, &m_depth, NULL);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_DEBUG("xnInit", "fail get depth");
		m_lastFrame = -6;
		xnContextRelease(m_pContext);
		m_pContext = NULL;
		return;
	} else {
		FBLOG_DEBUG("xnInit", "ok get depth");
	}
	
	nRetVal = xnCreateAnyProductionTree(m_pContext, XN_NODE_TYPE_GESTURE, NULL, &m_gestures, NULL);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_DEBUG("xnInit", "fail get gesture");
		m_lastFrame = -6;
		xnContextRelease(m_pContext);
		m_pContext = NULL;
		return;
	} else {
		FBLOG_INFO("xnInit", "ok get gesture");
	}
	
	nRetVal = xnCreateAnyProductionTree(m_pContext, XN_NODE_TYPE_HANDS, NULL, &m_hands, NULL);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_DEBUG("xnInit", "fail get hands");
		m_lastFrame = -6;
		xnContextRelease(m_pContext);
		return;
	} else {
		FBLOG_INFO("xnInit", "ok get hands");
	}

	nRetVal = xnCreateAnyProductionTree(m_pContext, XN_NODE_TYPE_USER, NULL, &m_users, NULL);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_DEBUG("xnInit", "fail create production tree");
		m_lastFrame = -6;
		xnContextRelease(m_pContext);
		m_pContext = NULL;
		return;
	} else {
		FBLOG_INFO("xnInit", "ok create production tree");
	}
	nRetVal = xnCreateAnyProductionTree(m_pContext, XN_NODE_TYPE_IMAGE, NULL, &m_image, NULL);
	if (nRetVal != XN_STATUS_OK) {
		m_image = NULL;
		// fail "gracefully" for image node - just keep on going
		FBLOG_DEBUG("xnInit", "fail create image node");
	} else {
		FBLOG_INFO("xnInit", "ok create production tree");
		m_imageOk = true;
	}
	// make sure global mirror is on
	xnSetGlobalMirror(m_pContext, TRUE);

	XnCallbackHandle ignore;
	// register to gesture/hands callbacks
	xnRegisterGestureCallbacks(m_gestures, &SensorOpenNI::GestureRecognizedHandler, NULL, this, &ignore);
	xnRegisterHandCallbacks(m_hands, &SensorOpenNI::HandCreateHandler, 
								  &SensorOpenNI::HandUpdateHandler, 
								  &SensorOpenNI::HandDestroyHandler,
								  this, &ignore);

	xnRegisterUserCallbacks(m_users, &SensorOpenNI::OnNewUser, 
								  &SensorOpenNI::OnLostUser, 
								  this, &ignore);
	xnRegisterToUserReEnter(m_users, &SensorOpenNI::OnUserReEnter, this, &ignore);
	xnRegisterToUserExit(m_users, &SensorOpenNI::OnUserExit, this, &ignore);
	xnSetSkeletonProfile(m_users, XN_SKEL_PROFILE_ALL);
	xnRegisterCalibrationCallbacks(m_users, &SensorOpenNI::OnCalibrationStart,
														  &SensorOpenNI::OnCalibrationEnd,
														  this, &ignore);
	xnSetSkeletonSmoothing(m_users, 0.5);
	xnRegisterToPoseCallbacks(m_users, &SensorOpenNI::OnPoseDetected, NULL, this, &ignore);

	xnAddGesture (m_gestures, "Wave",  NULL); //no bounding box
	xnAddGesture (m_gestures, "Click",  NULL); //no bounding box

	nRetVal = xnStartGeneratingAll(m_pContext);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_INFO("xnInit", "fail start generating");
		m_lastFrame = -1;
		xnContextRelease(m_pContext);
		m_pContext = NULL;
		return;
	} else {
		FBLOG_INFO("xnInit", "ok start generating");
	}
	nRetVal = xnWaitAndUpdateAll(m_pContext); // do a single real read
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_INFO("xnInit", "fail read frame");
		m_lastFrame = -1;
		xnContextRelease(m_pContext);
		m_pContext = NULL;
		return;
	} else {
		FBLOG_INFO("xnInit", "ok read frame");
	}

	m_initialized = true;
}

SensorOpenNI::~SensorOpenNI()
{
	if (NULL != m_pSceneMD) {
		xnFreeSceneMetaData(m_pSceneMD);
		m_pSceneMD = NULL;
	}
	if (NULL != m_pDepthMD) {
		xnFreeDepthMetaData(m_pDepthMD);
		m_pDepthMD = NULL;
	}
	if (NULL != m_pImageMD) {
		xnFreeImageMetaData(m_pImageMD);
		m_pImageMD = NULL;
	}
	if (NULL != m_hands) { 
		xnProductionNodeRelease(m_hands);
		m_hands = NULL;
	}
	if (NULL != m_users) { 
		xnProductionNodeRelease(m_users);
		m_users = NULL;
	}
	if (NULL != m_gestures) { 
		xnProductionNodeRelease(m_gestures);
		m_gestures = NULL;
	}
	if (NULL != m_depth) { 
		xnProductionNodeRelease(m_depth);
		m_depth = NULL;
	}
	if (NULL != m_device) { 
		xnProductionNodeRelease(m_device);
		m_device = NULL;
	}
	if (NULL != m_pContext) {
		xnContextRelease(m_pContext);
		m_pContext = NULL;
	}
}
bool SensorOpenNI::Valid() const {
	return m_initialized && (!m_error);
}

bool SensorOpenNI::ReadFrame(bool updateDepth, bool updateImage, bool updateLabelMap) {
	//TODO: refactor so that the per-frame results are encapsulated in an object
	//      instead of having a stateful object

	if (!Valid()) return false;
	//m_gotImage = false;

	XnStatus nRetVal = xnWaitNoneUpdateAll(m_pContext);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_INFO("ReadFrame", "fail wait & update");
		m_initialized = false; // can't read no depth no more
		return false; //TODO: throw exception (so we know to create a new reader thingy)
	}
	xnGetDepthMetaData(m_depth, m_pDepthMD);

	if (m_lastFrame == (int)m_pDepthMD->pMap->pOutput->nFrameID) {
		if (m_lastNewDataTime == 0xFFFFFFFFFFFFFFFFULL) {
			xnOSGetTimeStamp(&m_lastNewDataTime);
		}
		XnUInt64 currentTime;
		xnOSGetTimeStamp(&currentTime);
		if ((currentTime - m_lastNewDataTime) > 5*1000ULL) {
			FBLOG_INFO("ReadFrame", "timed out waiting for new data!");
			m_error = true;
		}
		return false; // not a new frame, do nothing
	}
	
	m_lastFrame = (int)m_pDepthMD->pMap->pOutput->nFrameID;
	xnOSGetTimeStamp(&m_lastNewDataTime);
	//m_lastNewDataTime = m_depthMD.Timestamp();
	xnGetUserPixels(m_users, 0, m_pSceneMD);

	if (updateDepth) {
		XnUInt32 xRatio = m_pDepthMD->pMap->Res.X / MAP_XRES; // assume there's never going to up upscaling
		XnUInt32 yRatio = m_pDepthMD->pMap->Res.Y / MAP_YRES;

		// base64 encode + downscale loop follows. Here's some info about it:
		// I'm assuming total number of pixels will always divide by 3 (works for 4x3 and 16x9 aspect ratios)
		// so as not to have special code for the end of the input.
		// base64 encodes 6 bits to 8 bits, so to fall on whole-byte boundaries, it's 3 input bytes into 4 characters
		// Every depth pixel is two bytes, so 3 pixels = 6 input bytes = 8 output characters
		// but, the catch is that the number of pixels in a row does not divide by 3
		// so I keep a pixel count (outputState), and so for every pixel I read, the behavior is:
		// if it's the first pixel (outputState % 3 == 0), store the two input bytes (in b1, b2)
		// if it's the second, use the two stored bytes from last iteration and the LSB of the current
		// pixel as a triplet, and store the MSB of the pixel in b1
		// if it's the third, use b1 and the two bytes of the pixel for a triplet

		int outputIndex = 0;
		int outputState = 0;
		unsigned char b1,b2;
		for(XnUInt32 y = 0; y < MAP_YRES; y++) {
			const XnDepthPixel* p = m_pDepthMD->pData + (y*yRatio*m_pDepthMD->pMap->Res.X);
			for(XnUInt32 x = 0; x < MAP_XRES; x++, p += xRatio, outputState++) {
				switch(outputState % 3) {
					case 0:
						b1 = (unsigned char)(*p);
						b2 = (unsigned char)((*p) >> 8);
						break;
					case 1:
						b64_encode_triplet(m_depthBuffer, outputIndex, b1, b2, (unsigned char)(*p));
						outputIndex += 4;
						b1 = (*p) >> 8;
						break;
					case 2:
						b64_encode_triplet(m_depthBuffer, outputIndex, b1, (unsigned char)(*p), (unsigned char)((*p) >> 8));
						outputIndex += 4;
						break;
				}
			}
		}

		m_depthJS = m_depthBuffer;
	} // if (updateDepth)
	if (updateImage && m_imageOk) {
		xnGetImageMetaData(m_image, m_pImageMD);
		XnUInt32 xRatio = m_pImageMD->pMap->Res.X / MAP_XRES; // assume there's never going to up upscaling
		XnUInt32 yRatio = m_pImageMD->pMap->Res.Y / MAP_YRES;
		//TODO: support only RGB24 by checking m_pImageMD->pMap->PixelFormat
		// right now it's assuming RGB24, not validating

		for(XnUInt32 y = 0; y < MAP_YRES; y++) {
			// get start-of-line read pointer
			const XnUInt8 * p = m_pImageMD->pData + (y*yRatio*m_pImageMD->pMap->Res.X*3);
			// unrolled for two pixels (3 characters)
			// x is in characters (it's used for output)
			for(XnUInt32 x = 0; x < MAP_XRES * 4; x+=4, p += xRatio*3) {
				unsigned char r = p[0];
				unsigned char g = p[1];
				unsigned char b = p[2];
				b64_encode_triplet(m_imageBuffer, x + y*MAP_XRES*4, r,g,b);
			}
		}
			
		m_imageJS = m_imageBuffer;
	}// if (updateImage)
	if (!m_imageOk) {
		m_imageJS = "";
	}
	//TODO: get 2d bounds per user when iterating over labelmap
	if (updateLabelMap) {
		XnUInt32 xRatio = m_pSceneMD->pMap->Res.X / MAP_XRES; // assume there's never going to up upscaling
		XnUInt32 yRatio = m_pSceneMD->pMap->Res.Y / MAP_YRES;
		int outputIndex = 0;
		int outputState = 0;
		unsigned char b1,b2;
		for(XnUInt32 y = 0; y < MAP_YRES; y++) {
			const XnLabel* p = m_pSceneMD->pData + (y*yRatio*m_pDepthMD->pMap->Res.X);
			for(XnUInt32 x = 0; x < MAP_XRES; x++, p += xRatio, outputState++) {
				switch(outputState % 3) {
					case 0:
						b1 = (unsigned char)(*p);
						b2 = (unsigned char)((*p) >> 8);
						break;
					case 1:
						b64_encode_triplet(m_labelMapBuffer, outputIndex, b1, b2, (unsigned char)(*p));
						outputIndex += 4;
						b1 = (*p) >> 8;
						break;
					case 2:
						b64_encode_triplet(m_labelMapBuffer, outputIndex, b1, (unsigned char)(*p), (unsigned char)((*p) >> 8));
						outputIndex += 4;
						break;
				}
			}
		}

		m_labelMapJS = m_labelMapBuffer;
	} // if (updateLabelmap)

	Json::Value pluginData;
	pluginData["hands"] = MakeHandsJsonList();
	pluginData["users"] = MakeUsersJsonList();
	pluginData["frameId"] = m_lastFrame;
	pluginData["timestamp"] = (double)m_lastNewDataTime;
	m_eventData = m_writer.write(pluginData); //TODO: unhack
	return true;
}
//
//boost::shared_ptr< FB::variant > SensorOpenNI::GetImageBase64() const {
//	if (!Valid()) return boost::shared_ptr< FB::variant >();
//	if (!m_gotImage) {
//		m_imageData = boost::make_shared< FB::variant >(*bitmap_from_depth(m_pDepthMD, m_pSceneMD));
//		m_gotImage = true;
//	}
//	return m_imageData;
//}

const std::string& SensorOpenNI::GetEventData() const {
	return m_eventData;
}

void SensorOpenNI::Unload() {
}

#ifdef _WIN32
#include <Windows.h>
const TCHAR OPENNIDLL[] = TEXT("OpenNI.dll");
bool SensorOpenNI::Available()
{
	//TODO: leaking a reference to the OpenNI DLL
	return (GetModuleHandle(OPENNIDLL) != NULL) || (LoadLibrary(OPENNIDLL) != NULL);
}
#else
// this is the only SDK available on non-windows, so we treat it as a prerequisite
bool SensorOpenNI::Available()
{
	return true;
}
#endif

bool SensorOpenNI::Init()
{
	if (!SensorOpenNI::Available()) return false;
	//xnUSBInit();
	//TODO: write this code once xnUSBSetCallbackHandler is written for *nix
	//xnUSBSetCallbackHandler
}

FB::VariantList SensorOpenNI::convertWorldToImageSpace(const std::vector<double>& points)
{
	if (!m_pDepthMD) return FB::VariantList(); // we need it
	FB::VariantList vl((points.size()/3)*3);
	double actualToOutputRatio = (double)MAP_XRES / m_pDepthMD->pMap->FullRes.X;
	std::vector<XnPoint3D> pts(points.size()/3);
	for(int i = 0; i < pts.size(); i++) {
		pts[i].X = points[i*3];
		pts[i].Y = points[i*3+1];
		pts[i].Z = points[i*3+2];
	}
	xnConvertRealWorldToProjective(m_depth, pts.size(), pts.data(), pts.data());
	for(int i = 0; i < pts.size(); i++) {
		vl[i*3] = (double)pts[i].X * actualToOutputRatio;
		vl[i*3+1] = (double)pts[i].Y * actualToOutputRatio;
		vl[i*3+2] = (double)pts[i].Z; // Z stays the same, no need to scale
	}
	return vl;
}


FB::VariantList SensorOpenNI::convertImageToWorldSpace(const std::vector<double>& points)
{
	if (!m_pDepthMD) return FB::VariantList(); // we need it
	FB::VariantList vl((points.size()/3)*3);
	double inputToActualRatio = (double)m_pDepthMD->pMap->FullRes.X / MAP_XRES;
	std::vector<XnPoint3D> pts(points.size()/3);
	for(int i = 0; i < pts.size(); i++) {
		pts[i].X = points[i*3] * inputToActualRatio;
		pts[i].Y = points[i*3+1] * inputToActualRatio;
		pts[i].Z = points[i*3+2];  // Z stays the same, no need to scale
	}
	xnConvertProjectiveToRealWorld(m_depth, pts.size(), pts.data(), pts.data());
	for(int i = 0; i < pts.size(); i++) {
		vl[i*3] = (double)pts[i].X;
		vl[i*3+1] = (double)pts[i].Y;
		vl[i*3+2] = (double)pts[i].Z;
	}
	return vl;
}

void SensorOpenNI::StopTrackingPoint(int pointId)
{
	xnStopTracking(m_hands, pointId);
}
void SensorOpenNI::SetHandRaise(bool enable)
{
	if (enable) {
		xnAddGesture(m_gestures, RAISE_HAND, NULL);
	} else {
		xnRemoveGesture(m_gestures, RAISE_HAND);
	}
}