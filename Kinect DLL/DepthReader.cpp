#include "stdafx.h"
#include "DepthReader.h"
#include <thread>
#include <tchar.h>

namespace KinectDLL{
	DepthReader::DepthReader(void) :
		HandRecognition(),
		fps_frameCount(0),
		fps_previousTime(0),
		fps_previousTimeDelta(0),
		nextDepthFrameEvent(INVALID_HANDLE_VALUE),
		depthStreamHandle(INVALID_HANDLE_VALUE),
		m_pNuiSensor(NULL)
	{
	}

	DepthReader::~DepthReader(void){
		if (m_pNuiSensor){
			m_pNuiSensor->NuiShutdown();
		}

		if (nextDepthFrameEvent != INVALID_HANDLE_VALUE){
			CloseHandle(nextDepthFrameEvent);
		}

		SafeRelease(m_pNuiSensor);
	}

	/// <summary>
	/// Create the first connected Kinect found 
	/// </summary>
	/// <returns>indicates success or failure</returns>
	HRESULT DepthReader::CreateFirstConnected()
	{
		INuiSensor * pNuiSensor;
		HRESULT hr;

		int iSensorCount = 0;
		hr = NuiGetSensorCount(&iSensorCount);
		if (FAILED(hr))
		{
			std::cout << "cant get sensor count" << std::endl;
			return hr;
		}
		// Look at each Kinect sensor
		for (int i = 0; i < iSensorCount; ++i)
		{
			// Create the sensor so we can check status, if we can't create it, move on to the next
			hr = NuiCreateSensorByIndex(i, &pNuiSensor);
			if (FAILED(hr))
			{
				continue;
			}

			// Get the status of the sensor, and if connected, then we can initialize it
			hr = pNuiSensor->NuiStatus();
			if (S_OK == hr)
			{
				m_pNuiSensor = pNuiSensor;
				break;
			}

			// This sensor wasn't OK, so release it since we're not using it
			pNuiSensor->Release();
		}

		if (NULL != m_pNuiSensor)
		{
			// Initialize the Kinect and specify that we'll be using depth
			hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_AUDIO | NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE);
			if (SUCCEEDED(hr))
			{
				// Create an event that will be signaled when depth data is available
				nextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

				// Open a depth image stream to receive depth frames
				hr = m_pNuiSensor->NuiImageStreamOpen(
					NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
					NUI_IMAGE_RESOLUTION_640x480,
					0,
					2,
					nextDepthFrameEvent,
					&depthStreamHandle);
			}
		}

		if (FAILED(hr)){
			std::cout << "FAILED(hr)" << std::endl;
		}

		if (NULL == m_pNuiSensor || FAILED(hr)){
			return E_FAIL;
		}

		long nuiCameraElevationAngle = static_cast<long>(GetPrivateProfileInt(TEXT("DepthReader"),
			TEXT("nuiCameraElevationAngle "),
			0,
			path));

		NuiCameraElevationSetAngle(nuiCameraElevationAngle);

		return hr;
	}

	/// <summary>
	/// Main processing function
	/// </summary>
	void DepthReader::Update(){
		if (NULL == m_pNuiSensor){
			return;
		}
		if (WAIT_OBJECT_0 == WaitForSingleObject(nextDepthFrameEvent, 0)){
			if (SUCCEEDED(ProcessDepth()) && m_depthBufferPitch != 0){
				getHands(m_pDepthBuffer);
				sync->setTexture(m_pTexture);
			}
			calculateFPS();
		}
	}

	/// <summary>
	/// Handle new depth data
	/// </summary>
	HRESULT DepthReader::ProcessDepth(){

		HRESULT hr;
		NUI_IMAGE_FRAME imageFrame;

		// Attempt to get the depth frame
		hr = m_pNuiSensor->NuiImageStreamGetNextFrame(depthStreamHandle, 0, &imageFrame);
		if (FAILED(hr))
		{
			return hr;
		}

		INuiFrameTexture* pTexture;

		// Get the depth image pixel texture
		hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
			depthStreamHandle, &imageFrame, &m_nearMode, &pTexture);
		if (SUCCEEDED(hr)){
			NUI_LOCKED_RECT LockedRect;

			// Lock the frame data so the Kinect knows not to modify it while we're reading it
			pTexture->LockRect(0, &LockedRect, NULL, 0);

			// Make sure we've received valid data
			if (LockedRect.Pitch != 0){
				// Copy image information into buffer
				BYTE* pBuffer = LockedRect.pBits;
				INT size = LockedRect.size;
				m_depthBufferPitch = LockedRect.Pitch;

				// Only reallocate memory if the buffer size has changed
				if (size != m_depthBufferSize){
					delete[] m_pDepthBuffer;
					m_pDepthBuffer = new BYTE[size];
					m_depthBufferSize = size;
				}
				memcpy_s(m_pDepthBuffer, size, pBuffer, size);
			}else{
				m_depthBufferPitch = 0;
			}

			// We're done with the texture so unlock it
			pTexture->UnlockRect(0);

			pTexture->Release();
		}else{
			return E_FAIL;
		}
		// Release the frame
		m_pNuiSensor->NuiImageStreamReleaseFrame(depthStreamHandle, &imageFrame);

		return S_OK;
	}

	/// <summary>
	/// Creates the main window and begins processing
	/// </summary>
	/// <param name="syncToSet">Pointer to Synchronizer class object</param>
	int DepthReader::Run(Synchronizer* syncToSet)
	{
		sync = syncToSet;

		// Look for a connected Kinect, and create it if found
		if (SUCCEEDED(CreateFirstConnected())){
			sync->setKinectReady();
		}else{
			exit(1);
		}

		// Main message loop
		while (sync->getState() != QUIT){
			Update();
		}
		return 0;
	}

	void DepthReader::calculateFPS()
	{
		//  Increase frame count
		++fps_frameCount;

		//  Get the number of milliseconds since glutInit called
		//  (or first call to glutGet(GLUT ELAPSED TIME)).
		int currentTime = glutGet(GLUT_ELAPSED_TIME);

		//  Calculate time passed
		int timeInterval = currentTime - fps_previousTime;	
		
		if (timeInterval > 1000){
			//getting delta
			int delta = glutGet(GLUT_ELAPSED_TIME) - fps_previousTimeDelta;
			fps_previousTimeDelta = glutGet(GLUT_ELAPSED_TIME);

			//  calculate the number of frames per second
			double fps = fps_frameCount / (timeInterval / 1000.0f);

			//  Reset frame count
			fps_frameCount = 0;

			std::cout << "FPS KINECT: " << fps << ", delta KINECT: " << delta << std::endl;

			//  Set time
			fps_previousTime = currentTime;
		}
	}
}