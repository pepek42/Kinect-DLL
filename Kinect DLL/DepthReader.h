#pragma once
#include "HandRecognition.h"

namespace KinectDLL{

	class DepthReader : private HandRecognition{

	public:
		/// <summary>
		/// Constructor
		/// </summary>
		KINECTDLL_API DepthReader(void);

		/// <summary>
		/// Destructor
		/// </summary>
		KINECTDLL_API ~DepthReader(void);

		/// <summary>
		/// Creates depth reader and begins processing
		/// </summary>
		/// <param name="syncToSet">Pointer for global synchronizer</param>
		KINECTDLL_API int Run(Synchronizer* syncToSet);
	private:

		HANDLE depthStreamHandle;
		HANDLE nextDepthFrameEvent;
		HANDLE depthStream;

		/// pointer for kinect sensor
		INuiSensor* m_pNuiSensor;

		//for frames countig
		int fps_frameCount;

		int fps_previousTime;

		int fps_previousTimeDelta;

		//HWND m_hWnd;
		/// <summary>
		/// Create the first connected Kinect found 
		/// </summary>
		/// <returns>S_OK on success, otherwise failure code</returns>
		HRESULT CreateFirstConnected();

		/// <summary>
		/// Main processing function
		/// </summary>
		void Update();

		/// <summary>
		/// Handle new depth data
		/// </summary>
		HRESULT ProcessDepth();

		/// <summary>
		/// Gets delta and "fps" of depth reader
		/// </summary>
		void calculateFPS();
	};
}