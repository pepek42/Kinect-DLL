//------------------------------------------------------------------------------
// <copyright file="SpeechBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "KinectDLL.h"
#include "Synchronizer.h"

#include "KinectAudioStream.h"

// For configuring DMO properties
#include <wmcodecdsp.h>

// For FORMAT_WaveFormatEx and such
#include <uuids.h>

// For Kinect SDK APIs
#include <NuiApi.h>

// For speech APIs
// NOTE: To ensure that application compiles and links against correct SAPI versions (from Microsoft Speech
//       SDK), VC++ include and library paths should be configured to list appropriate paths within Microsoft
//       Speech SDK installation directory before listing the default system include and library directories,
//       which might contain a version of SAPI that is not appropriate for use together with Kinect sensor.
#include <sapi.h>
//#include <sphelper.h>

namespace KinectDLL{

	/// <summary>
	/// Main class for Speech Control.
	/// </summary>
	class SpeechControl{
	public:
		/// <summary>
		/// Constructor
		/// </summary>
		KINECTDLL_API SpeechControl(void);

		/// <summary>
		/// Destructor
		/// </summary>
		KINECTDLL_API ~SpeechControl(void);

		/// <summary>
		/// Begins processing
		/// </summary>
		KINECTDLL_API int Run(Synchronizer* syncPointer);

	private:
		static LPCWSTR          GrammarFileName;

		float m_ConfidenceThreshold;

		Synchronizer* sync;

		// Current Kinect sensor
		INuiSensor*             m_pNuiSensor;

		// Audio stream captured from Kinect.
		KinectAudioStream*      m_pKinectAudioStream;

		// Stream given to speech recognition engine
		ISpStream*              m_pSpeechStream;

		// Speech recognizer
		ISpRecognizer*          m_pSpeechRecognizer;

		// Speech recognizer context
		ISpRecoContext*         m_pSpeechContext;

		// Speech grammar
		ISpRecoGrammar*         m_pSpeechGrammar;

		// Event triggered when we detect speech recognition
		HANDLE                  m_hSpeechEvent;

		/// <summary>
		/// Create the first connected Kinect found.
		/// </summary>
		/// <returns>S_OK on success, otherwise failure code.</returns>
		HRESULT                 CreateFirstConnected();

		/// <summary>
		/// Initialize Kinect audio stream object.
		/// </summary>
		/// <returns>S_OK on success, otherwise failure code.</returns>
		HRESULT                 InitializeAudioStream();

		/// <summary>
		/// Create speech recognizer that will read Kinect audio stream data.
		/// </summary>
		/// <returns>
		/// <para>S_OK on success, otherwise failure code.</para>
		/// </returns>
		HRESULT                 CreateSpeechRecognizer();

		/// <summary>
		/// Load speech recognition grammar into recognizer.
		/// </summary>
		/// <returns>
		/// <para>S_OK on success, otherwise failure code.</para>
		/// </returns>
		HRESULT                 LoadSpeechGrammar();

		/// <summary>
		/// Start recognizing speech asynchronously.
		/// </summary>
		/// <returns>
		/// <para>S_OK on success, otherwise failure code.</para>
		/// </returns>
		HRESULT                 StartSpeechRecognition();

		/// <summary>
		/// Process recently triggered speech recognition events.
		/// </summary>
		void                    ProcessSpeech();
	};
}