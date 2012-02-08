
#ifdef _WIN32
#include "SensorKinectSDK.h"
#include "logging.h"
#include "boost/bind.hpp"

#include <Windows.h>
#include <NuiApi.h>

#include <Eigen>
using Eigen::Vector3f;
using Eigen::Matrix3f;

using namespace boost::assign;

//TODO: get rid of this - it's only used for joint id conversion
#include <XnOpenNI.h>

const int DEPTH_MAP_WIDTH = 320;
const int DEPTH_MAP_HEIGHT = 240;
const int IMAGE_MAP_WIDTH = 640;
const int IMAGE_MAP_HEIGHT = 480;


/////////// VECTOR MATH
//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

static Vector3f vectorBetweenNuiJoints(NUI_SKELETON_DATA * skeleton, int p1, int p2)
{
	if (NUI_SKELETON_POSITION_NOT_TRACKED == skeleton->eSkeletonPositionTrackingState[p1] ||
		NUI_SKELETON_POSITION_NOT_TRACKED == skeleton->eSkeletonPositionTrackingState[p2]) {
			return Vector3f::Zero();
	}

	Vector3f v1((Vector3f::Scalar *)&skeleton->SkeletonPositions[p1]);
	Vector3f v2((Vector3f::Scalar *)&skeleton->SkeletonPositions[p2]);
	return v2 - v1;
}

static Matrix3f orientationFromX(Vector3f v)
{
	Matrix3f result;
	result.col(0) = v.normalized();
	result.col(1) = Vector3f(0.0f, v.z(), -v.y()).normalized();
	result.col(2) = result.col(0).cross(result.col(1));
	return result;
}

static Matrix3f orientationFromY(Vector3f v)
{
	Matrix3f result;
	result.col(0) = Vector3f(v.y(), -v.x(), 0.0f).normalized();
	result.col(1) = v.normalized();
	result.col(2) = result.col(0).cross(result.col(1));
	return result;
}

static Matrix3f orientationFromZ(Vector3f v)
{
	Matrix3f result;
	result.col(0) = Vector3f(v.y(), -v.x(), 0.0f).normalized();
	result.col(2) = v.normalized();
	result.col(1) = result.col(2).cross(result.col(0));
	return result;
}

static Matrix3f orientationFromXY(Vector3f vx, Vector3f vy)
{
	Matrix3f result;
	result.col(0) = vx.normalized();
	result.col(2) = result.col(0).cross(vy.normalized()).normalized();
	result.col(1) = result.col(2).cross(result.col(0));
	return result;
}

static Matrix3f orientationFromYX(Vector3f vx, Vector3f vy)
{
	Matrix3f result;
	result.col(1) = vy.normalized();
	result.col(2) = vx.normalized().cross(result.col(1)).normalized();
	result.col(0) = result.col(1).cross(result.col(2));
	return result;
}

static Matrix3f orientationFromYZ(Vector3f vy, Vector3f vz)
{
	Matrix3f result;
	result.col(1) = vy.normalized();
	result.col(0) = result.col(1).cross(vz.normalized()).normalized();
	result.col(2) = result.col(0).cross(result.col(1));
	return result;
}




/////// END OF THAT SHIT
const TCHAR KINECTDLL[] = TEXT("Kinect10.dll");
bool SensorKinectSDK::Available() {
	return Init();
}

//TODO: DON'T USE THIS RIGHT NOW!
void SensorKinectSDK::Unload() {
	s_activeInstance.reset();
}
SensorPtr SensorKinectSDK::s_activeInstance;
void CALLBACK SensorKinectSDK::SensorStatusCallback(long hrStatus, wchar_t* instanceName, wchar_t* uniqueDeviceName, void* pUserData)
{
	//if (SUCCEEDED(hrStatus)) {
	if (0 == hrStatus) {
		SensorPtr newInstance(new SensorKinectSDK(instanceName));
		s_activeInstance = newInstance;
	} else {
		// kill the instance?
	}
}

static SensorPtr s_activeInstance;
SensorPtr SensorKinectSDK::GetInstance() {
	//TODO: there's probably a race between this and SensorStatusCallback, we just don't really want to mess with this shit,
	// and it'll require extremely unlikely timing
	if (!s_activeInstance || (!s_activeInstance->Valid())) {
		s_activeInstance.reset(new SensorKinectSDK(NULL));
	}
	return s_activeInstance; // creates a copy, hurrah!
}
bool SensorKinectSDK::Init()
{
	//TODO: leaking a reference to kinect DLL if we load it
	if ((NULL != GetModuleHandle(KINECTDLL)) || (NULL != LoadLibrary(KINECTDLL))) {
		NuiSetDeviceStatusCallback((NuiStatusProc)&SensorKinectSDK::SensorStatusCallback, NULL);
		return true;
	}
	return false;
}

SensorKinectSDK::SensorKinectSDK(const wchar_t* id) :
	m_error(false), m_initialized(false), m_sensor(NULL)
{
	// try initializing the SDK
	if (!SensorKinectSDK::Init()) return;
	if (!SensorKinectSDK::Available()) return; // if no sensor connected fail silently
	// create sensor
	HRESULT hr = S_OK;
	INuiSensor * pNuiSensor = NULL;

	if (NULL != id) {
		hr = NuiCreateSensorById(id, &pNuiSensor);//NuiCreateSensorByIndexFuncPtr(0, &pNuiSensor);
	} else {
		int sensorCount = 0;
		hr = NuiGetSensorCount(&sensorCount);
		if (FAILED(hr)) {
			return;
		}
		if (sensorCount <= 0) {
			return;
		}
		hr = NuiCreateSensorByIndex(0, &pNuiSensor);
	}
	// error test both cases of the previous if
    if ( FAILED(hr) )
    {
		return;
    }
	m_sensor =pNuiSensor;

	// init it to use depth, skeleton
	DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR;
	hr = m_sensor->NuiInitialize( nuiFlags );
	if ( FAILED( hr ) )
	{
		return;
	}
	
	// .. and enable skel tracking
	hr = m_sensor->NuiSkeletonTrackingEnable( NULL, 0 );
    if( FAILED( hr ) ) 
	{
		return;
	}
	// open depth and image streams
	hr = m_sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240, 0, 2, NULL, &m_depth);
    if( FAILED( hr ) ) 
	{
		return;
	}

	hr = m_sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, NULL, &m_image);
    if( FAILED( hr ) ) 
	{
		return;
	}
	m_initialized = true;
}

SensorKinectSDK::~SensorKinectSDK() {
	// Unforunately once NuiShutdown is called we cant call NuiInitialize again, so we're
	// not really shutting down
	if (m_sensor) {
		m_sensor->NuiShutdown( );
		m_sensor->Release();
		m_sensor = NULL;
	}
	return;
}

bool SensorKinectSDK::Valid() const {
	return m_initialized && (!m_error);
}

//boost::shared_ptr< FB::variant > SensorKinectSDK::GetImageBase64() const
//{
//	return boost::make_shared< FB::variant >();
//}

static bool getOrientation(NUI_SKELETON_DATA * skeleton, int joint, float  orientation[9])
{
	Matrix3f result; 
	switch (joint) {
  		case NUI_SKELETON_POSITION_HIP_CENTER:
			result = orientationFromYX(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_HIP_LEFT,NUI_SKELETON_POSITION_HIP_RIGHT),
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_HIP_CENTER,NUI_SKELETON_POSITION_SPINE));
			break;
   
		case NUI_SKELETON_POSITION_SPINE:
			result = orientationFromYX(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_SHOULDER_LEFT,NUI_SKELETON_POSITION_SHOULDER_RIGHT),
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_SPINE,NUI_SKELETON_POSITION_SHOULDER_CENTER));
			break;
   
		case NUI_SKELETON_POSITION_SHOULDER_CENTER:
			result = orientationFromYX(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_SHOULDER_LEFT,NUI_SKELETON_POSITION_SHOULDER_RIGHT),
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_SHOULDER_CENTER,NUI_SKELETON_POSITION_HEAD));
			break;

		case NUI_SKELETON_POSITION_HEAD:
			result = orientationFromY(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_SHOULDER_CENTER,NUI_SKELETON_POSITION_HEAD));
			break;
   
		case NUI_SKELETON_POSITION_SHOULDER_LEFT:
			result = orientationFromXY(
				-vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_SHOULDER_LEFT,NUI_SKELETON_POSITION_ELBOW_LEFT),
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_ELBOW_LEFT,NUI_SKELETON_POSITION_WRIST_LEFT));
			break;
   
		case NUI_SKELETON_POSITION_ELBOW_LEFT:
			result = orientationFromXY(
				-vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_ELBOW_LEFT,NUI_SKELETON_POSITION_WRIST_LEFT),
				-vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_SHOULDER_LEFT,NUI_SKELETON_POSITION_ELBOW_LEFT));
			break;
   
		case NUI_SKELETON_POSITION_WRIST_LEFT:
			result = orientationFromX(
				-vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_WRIST_LEFT,NUI_SKELETON_POSITION_HAND_LEFT));
			break;
   
		case NUI_SKELETON_POSITION_HAND_LEFT:
			result = orientationFromX(
				-vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_WRIST_LEFT,NUI_SKELETON_POSITION_HAND_LEFT));
			break;
   
		case NUI_SKELETON_POSITION_HIP_LEFT:
			result = orientationFromYX(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_HIP_LEFT,NUI_SKELETON_POSITION_HIP_RIGHT),
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_KNEE_LEFT,NUI_SKELETON_POSITION_HIP_LEFT));
			break;
   
		case NUI_SKELETON_POSITION_KNEE_LEFT:
			result = orientationFromY(
				-vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_KNEE_LEFT,NUI_SKELETON_POSITION_ANKLE_LEFT));
			break;
   
		case NUI_SKELETON_POSITION_ANKLE_LEFT:
			result = orientationFromZ(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_FOOT_LEFT,NUI_SKELETON_POSITION_ANKLE_LEFT));
			break;
   
		case NUI_SKELETON_POSITION_FOOT_LEFT:
			result = orientationFromZ(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_FOOT_LEFT,NUI_SKELETON_POSITION_ANKLE_LEFT));
			break;
   
   
		case NUI_SKELETON_POSITION_SHOULDER_RIGHT:
			result = orientationFromXY(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_SHOULDER_RIGHT,NUI_SKELETON_POSITION_ELBOW_RIGHT),
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_ELBOW_RIGHT,NUI_SKELETON_POSITION_WRIST_RIGHT));
			break;
   
		case NUI_SKELETON_POSITION_ELBOW_RIGHT:
			result = orientationFromXY(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_ELBOW_RIGHT,NUI_SKELETON_POSITION_WRIST_RIGHT),
				-vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_SHOULDER_RIGHT,NUI_SKELETON_POSITION_ELBOW_RIGHT));
			break;
   
		case NUI_SKELETON_POSITION_WRIST_RIGHT:
			result = orientationFromX(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_WRIST_RIGHT,NUI_SKELETON_POSITION_HAND_RIGHT));
			break;
   
		case NUI_SKELETON_POSITION_HAND_RIGHT:
			result = orientationFromX(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_WRIST_RIGHT,NUI_SKELETON_POSITION_HAND_RIGHT));
			break;
   
		case NUI_SKELETON_POSITION_HIP_RIGHT:
			result = orientationFromYX(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_HIP_LEFT,NUI_SKELETON_POSITION_HIP_RIGHT),
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_KNEE_RIGHT,NUI_SKELETON_POSITION_HIP_RIGHT));
			break;
   
		case NUI_SKELETON_POSITION_KNEE_RIGHT:
			result = orientationFromYZ(
				-vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_KNEE_RIGHT,NUI_SKELETON_POSITION_ANKLE_RIGHT),
				-vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_ANKLE_RIGHT,NUI_SKELETON_POSITION_FOOT_RIGHT));
			/*result = orientationFromY(
				-vectorBetweenNuiJoints(skeleton, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT));*/
			break;
   
		case NUI_SKELETON_POSITION_ANKLE_RIGHT:
			result = orientationFromZ(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_FOOT_RIGHT,NUI_SKELETON_POSITION_ANKLE_RIGHT));
			break;
   
		case NUI_SKELETON_POSITION_FOOT_RIGHT:
			result = orientationFromZ(
				vectorBetweenNuiJoints(skeleton,NUI_SKELETON_POSITION_FOOT_RIGHT,NUI_SKELETON_POSITION_ANKLE_RIGHT));
			break;
		default:
			return false;
	}
	
	CopyMemory(orientation, result.data(), sizeof(float) * 9);
	return true;
}


static Json::Value OrientationToValue(float ori[9])
{
	Json::Value result(Json::arrayValue);
	result.resize(9);
	result[0u] = ori[0];
	result[1u] = ori[3];
	result[2u] = ori[6];
	result[3u] = ori[1];
	result[4u] = ori[4];
	result[5u] = ori[7];
	result[6u] = ori[2];
	result[7u] = ori[5];
	result[8u] = ori[8];
	//result.assign(ori.elements, ori.elements + 9);
	return result;
}

static Json::Value PositionToValue(Vector4& pos)
{
	Json::Value xyz(Json::arrayValue);
	xyz.resize(3);
	xyz[0u] = pos.x*1000;
	xyz[1u] = pos.y*1000;
	xyz[2u] = pos.z*1000;
	return xyz;
}

int KinectToZigId(int joint)
{
		switch (joint) {
  		case NUI_SKELETON_POSITION_HIP_CENTER: return XN_SKEL_WAIST;
		case NUI_SKELETON_POSITION_SPINE: return XN_SKEL_TORSO;
		case NUI_SKELETON_POSITION_SHOULDER_CENTER: return XN_SKEL_NECK;
		case NUI_SKELETON_POSITION_HEAD: return XN_SKEL_HEAD;
		case NUI_SKELETON_POSITION_SHOULDER_LEFT: return XN_SKEL_LEFT_SHOULDER;
		case NUI_SKELETON_POSITION_ELBOW_LEFT: return XN_SKEL_LEFT_ELBOW;
		case NUI_SKELETON_POSITION_WRIST_LEFT: return XN_SKEL_LEFT_WRIST;
		case NUI_SKELETON_POSITION_HAND_LEFT: return XN_SKEL_LEFT_HAND;
		case NUI_SKELETON_POSITION_HIP_LEFT: return XN_SKEL_LEFT_HIP;
		case NUI_SKELETON_POSITION_KNEE_LEFT: return XN_SKEL_LEFT_KNEE;
		case NUI_SKELETON_POSITION_ANKLE_LEFT: return XN_SKEL_LEFT_ANKLE;
		case NUI_SKELETON_POSITION_FOOT_LEFT: return XN_SKEL_LEFT_FOOT;
		case NUI_SKELETON_POSITION_SHOULDER_RIGHT: return XN_SKEL_RIGHT_SHOULDER;
		case NUI_SKELETON_POSITION_ELBOW_RIGHT: return XN_SKEL_RIGHT_ELBOW;
		case NUI_SKELETON_POSITION_WRIST_RIGHT: return XN_SKEL_RIGHT_WRIST;
		case NUI_SKELETON_POSITION_HAND_RIGHT: return XN_SKEL_RIGHT_HAND;
		case NUI_SKELETON_POSITION_HIP_RIGHT: return XN_SKEL_RIGHT_HIP;
		case NUI_SKELETON_POSITION_KNEE_RIGHT: return XN_SKEL_RIGHT_KNEE;
		case NUI_SKELETON_POSITION_ANKLE_RIGHT: return XN_SKEL_RIGHT_ANKLE;
		case NUI_SKELETON_POSITION_FOOT_RIGHT: return XN_SKEL_RIGHT_FOOT;
	}
	return -1337; // should never happen
}


bool SensorKinectSDK::ReadFrame(bool updateDepth, bool updateImage, bool isWebplayer)
{
	if (!Valid()) return false;
	HRESULT read;
	NUI_SKELETON_FRAME skeletonFrame; 
	if ( FAILED(read = m_sensor->NuiSkeletonGetNextFrame( 0, &skeletonFrame )) )
    {
		if (read != E_NUI_FRAME_NO_DATA) {
			m_error = true; // actual error, not only "no new data"
		}
		return false;
	} else {
		// smooth out the skeleton data
		HRESULT hr = m_sensor->NuiTransformSmooth( &skeletonFrame, NULL );
		if ( FAILED(hr) )
		{
			m_error = true;
			return false;
		}
	}

	// get image/depth if available/needed
	if (updateDepth) {
		int xRatio = DEPTH_MAP_WIDTH / MAP_XRES; // assume there's never going to up upscaling
		int yRatio = DEPTH_MAP_HEIGHT / MAP_YRES;

		NUI_IMAGE_FRAME frame;
		HRESULT hr = m_sensor->NuiImageStreamGetNextFrame(m_depth, 0, &frame);
		if ( FAILED(hr) ) {
			return false; //TODO: test out that this doesn't just happen and cause us to drop frames
		}
		NUI_LOCKED_RECT rect;
		hr = frame.pFrameTexture->LockRect(0, &rect, NULL, 0);
		if ( FAILED(hr) ) {
			return false;
		}
		unsigned short * pixels = (unsigned short *)rect.pBits;
		if (isWebplayer) {
			// for webplayer, increment every pixel by 1 to solve null-termination problem
			for(int y = 0; y < MAP_YRES; y++) {
				// get start-of-line read pointer
				const unsigned short * p = pixels + (y*yRatio*DEPTH_MAP_WIDTH);
				for(int x = 0; x < MAP_XRES*2; x += 2, p += xRatio) {
					unsigned short pixel = (*p) >> 3;
					m_depthBuffer[x + y*MAP_XRES*2] = (pixel) | 1; // set LSB on least-significant byte
					m_depthBuffer[x + 1 + y*MAP_XRES*2] = ((pixel) >> 8) | (1<<7); // MSB on most-significant
				}
			}
		} else {
			for(int y = 0; y < MAP_YRES; y++) {
				const unsigned short * p = pixels + (y*yRatio*DEPTH_MAP_WIDTH);
				for(int x = 0; x < MAP_XRES*2; x += 2, p += xRatio) {
					unsigned short pixel = (*p) >> 3;
					m_depthBuffer[x + y*MAP_XRES*2] = pixel;
					m_depthBuffer[x + 1 + y*MAP_XRES*2] = pixel >> 8;
				}
			}
		}
		frame.pFrameTexture->UnlockRect(0); // TODO: check result?
		m_sensor->NuiImageStreamReleaseFrame(m_depth, &frame);

		m_depthJS.assign(FB::make_variant(m_depthBuffer));
	} // if (updateDepth)

	if (updateImage) {
		int xRatio = IMAGE_MAP_WIDTH / MAP_XRES; // assume there's never going to up upscaling
		int yRatio = IMAGE_MAP_HEIGHT / MAP_YRES;

		NUI_IMAGE_FRAME frame;
		HRESULT hr = m_sensor->NuiImageStreamGetNextFrame(m_image, 0, &frame);
		if ( FAILED(hr) ) {
			return false; //TODO: test out that this doesn't just happen and cause us to drop frames
		}
		NUI_LOCKED_RECT rect;
		hr = frame.pFrameTexture->LockRect(0, &rect, NULL, 0);
		if ( FAILED(hr) ) {
			return false;
		}
		// right now it's assuming RGBX (kinectSDK)
		if (isWebplayer) {
			// for webplayer, increment every pixel by 1 to solve null-termination problem
			for(int y = 0; y < MAP_YRES; y++) {
				// get start-of-line read pointer
				const unsigned char * p = rect.pBits + (y*yRatio*IMAGE_MAP_WIDTH*4);
				for(int x = 0; x < MAP_XRES * 3; x+=3, p += xRatio*4) {
					unsigned char r = p[0];
					unsigned char g = p[1];
					unsigned char b = p[2];
					m_imageBuffer[x + y*MAP_XRES*3] = (char) (r | 1);
					m_imageBuffer[x + 1 + y*MAP_XRES*3] = (char) (g | 1);
					m_imageBuffer[x + 2 + y*MAP_XRES*3] = (char) (b | 1);
				}
			}
		} else {
			// for webplayer, increment every pixel by 1 to solve null-termination problem
			for(int y = 0; y < MAP_YRES; y++) {
				// get start-of-line read pointer
				const unsigned char * p = rect.pBits + (y*yRatio*IMAGE_MAP_WIDTH*4);
				for(int x = 0; x < MAP_XRES * 3; x+=3, p += xRatio*4) {
					unsigned char r = p[0];
					unsigned char g = p[1];
					unsigned char b = p[2];
					m_imageBuffer[x + y*MAP_XRES*3] = (char) (r | 1);
					m_imageBuffer[x + 1 + y*MAP_XRES*3] = (char) (g | 1);
					m_imageBuffer[x + 2 + y*MAP_XRES*3] = (char) (b | 1);
				}
			}
		}
		frame.pFrameTexture->UnlockRect(0); // TODO: check result?
		m_sensor->NuiImageStreamReleaseFrame(m_image, &frame);
		//m_imageJS.reset(); //needed to stop memory leak
		m_imageJS.assign(FB::make_variant(m_imageBuffer));
	}// if (updateImage)


	Json::Value jsUsers(Json::arrayValue);
	// now parse skeletonFrame - it has data
	for(int i = 0; i < 6; i++) {
		NUI_SKELETON_DATA& user = skeletonFrame.SkeletonData[i];
		if (user.eTrackingState == NUI_SKELETON_NOT_TRACKED) continue; // skip non-existent users
		Json::Value joints(Json::arrayValue);
		if (user.eTrackingState == NUI_SKELETON_TRACKED) {
			float orientationTemp[9];
			// build joints list
			for(int j = 0; j < 20; j++) { //magic number is used by MS :(
				// skip joint if not being tracked
				if (NUI_SKELETON_POSITION_NOT_TRACKED == user.eSkeletonPositionTrackingState[j]) continue;

				Json::Value joint(Json::objectValue);
				joint["id"] = KinectToZigId(j);
				joint["position"] = PositionToValue(user.SkeletonPositions[j]);
				getOrientation(&user, j, orientationTemp);
				joint["rotation"] = OrientationToValue(orientationTemp);
				joint["rotationconfidence"] = joint["positionconfidence"] = 
						(user.eSkeletonPositionTrackingState[j] == NUI_SKELETON_POSITION_INFERRED) ? 0.5 : 1.0;
				joints.append(joint);
			}
		}
		Json::Value userJson(Json::objectValue);
		// if NUI_SKELETON_TRACKED then 1, otherwise (NUI_SKELETON_POSITION_ONLY), 0
		userJson["tracked"] = (user.eTrackingState == NUI_SKELETON_TRACKED) ? 1 : 0;
		userJson["centerofmass"] = PositionToValue(user.Position);
		userJson["id"] = (int)user.dwTrackingID;
		userJson["joints"] = joints;
		jsUsers.append(userJson);
	}
	
	Json::Value pluginData(Json::objectValue);
	pluginData["users"] = jsUsers;
	pluginData["hands"] = Json::Value(Json::arrayValue);
	pluginData["frameId"] = (int)skeletonFrame.dwFrameNumber;
	pluginData["timestamp"] = (double)skeletonFrame.liTimeStamp.QuadPart;
	m_lastFrameData = m_writer.write(pluginData);
	return true;

}

const std::string& SensorKinectSDK::GetEventData() const {
	return m_lastFrameData;
}

#endif
