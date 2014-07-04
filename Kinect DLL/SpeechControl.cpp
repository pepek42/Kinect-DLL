//------------------------------------------------------------------------------
// <copyright file="SpeechBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "SpeechControl.h"

#define INITGUID
#include <guiddef.h>
#include <iostream>
// Static initializers

namespace KinectDLL{
	LPCWSTR SpeechControl::GrammarFileName = L"SpeechBasics-D2D.grxml";

	/// <summary>
	/// Constructor
	/// </summary>
	SpeechControl::SpeechControl(void) :
		m_pNuiSensor(NULL),
		m_pKinectAudioStream(NULL),
		m_pSpeechStream(NULL),
		m_pSpeechRecognizer(NULL),
		m_pSpeechContext(NULL),
		m_pSpeechGrammar(NULL),
		m_hSpeechEvent(INVALID_HANDLE_VALUE),
		sync(NULL)
	{
		TCHAR   inBuf[100];

		GetPrivateProfileString(TEXT("SpeechControl"),
			TEXT("m_ConfidenceThreshold "),
			TEXT("0.4"),
			inBuf,
			80,
			path);

		m_ConfidenceThreshold = _tcstod(inBuf, NULL);
	}

	/// <summary>
	/// Destructor
	/// </summary>
	SpeechControl::~SpeechControl(void)
	{
		if (m_pNuiSensor)
		{
			m_pNuiSensor->NuiShutdown();
		}

		SafeRelease(m_pNuiSensor);
		SafeRelease(m_pKinectAudioStream);
		SafeRelease(m_pSpeechStream);
		SafeRelease(m_pSpeechRecognizer);
		SafeRelease(m_pSpeechContext);
		SafeRelease(m_pSpeechGrammar);
	}

	/// <summary>
	/// Creates the main window and begins processing
	/// </summary>
	/// <param name="syncPointer">Pointer to Synchronizer class object</param>
	int SpeechControl::Run(Synchronizer* syncPointer)
	{
		sync = syncPointer;
		HRESULT hr = CreateFirstConnected();
		if (FAILED(hr))
		{
			return 1;
		}

		const int eventCount = 1;
		HANDLE hEvents[eventCount];

		// Main message loop
		while (sync->getState() != QUIT)
		{
			hEvents[0] = m_hSpeechEvent;

			// Check to see if we have either a message (by passing in QS_ALLINPUT)
			// Or a speech event (hEvents)
			WaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE);

			// Explicitly check for new speech recognition events since
			// MsgWaitForMultipleObjects can return for other reasons
			// even though it is signaled.
			ProcessSpeech();
		}

		//Quit!
		if (NULL != m_pKinectAudioStream)
		{
			m_pKinectAudioStream->StopCapture();
		}

		if (NULL != m_pSpeechRecognizer)
		{
			m_pSpeechRecognizer->SetRecoState(SPRST_INACTIVE);
		}

		return 0;
	}

	/// <summary>
	/// Create the first connected Kinect found.
	/// </summary>
	/// <returns>S_OK on success, otherwise failure code.</returns>
	HRESULT SpeechControl::CreateFirstConnected()
	{
		INuiSensor * pNuiSensor;
		HRESULT hr;

		int iSensorCount = 0;
		hr = NuiGetSensorCount(&iSensorCount);
		if (FAILED(hr))
		{
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

		if (NULL == m_pNuiSensor || FAILED(hr))
		{
			std::cout << "No ready Kinect found!" << std::endl;
			return E_FAIL;
		}

		hr = InitializeAudioStream();
		if (FAILED(hr))
		{
			std::cout << "Could not initialize audio stream." << std::endl;
			return hr;
		}

		hr = CreateSpeechRecognizer();
		if (FAILED(hr))
		{
			std::cout << "Could not create speech recognizer. Please ensure that Microsoft Speech SDK and other sample requirements are installed." << std::endl;
			return hr;
		}

		hr = LoadSpeechGrammar();
		if (FAILED(hr))
		{
			std::cout << "Could not load speech grammar. Please ensure that grammar configuration file was properly deployed." << std::endl;
			return hr;
		}

		hr = StartSpeechRecognition();
		if (FAILED(hr))
		{
			std::cout << "Could not start recognizing speech." << std::endl;
			return hr;
		}

		return hr;
	}

	/// <summary>
	/// Initialize Kinect audio stream object.
	/// </summary>
	/// <returns>
	/// <para>S_OK on success, otherwise failure code.</para>
	/// </returns>
	HRESULT SpeechControl::InitializeAudioStream()
	{
		INuiAudioBeam*      pNuiAudioSource = NULL;
		IMediaObject*       pDMO = NULL;
		IPropertyStore*     pPropertyStore = NULL;
		IStream*            pStream = NULL;

		// Get the audio source
		HRESULT hr = m_pNuiSensor->NuiGetAudioSource(&pNuiAudioSource);
		if (SUCCEEDED(hr))
		{
			hr = pNuiAudioSource->QueryInterface(IID_IMediaObject, (void**)&pDMO);

			if (SUCCEEDED(hr))
			{
				hr = pNuiAudioSource->QueryInterface(IID_IPropertyStore, (void**)&pPropertyStore);

				// Set AEC-MicArray DMO system mode. This must be set for the DMO to work properly.
				// Possible values are:
				//   SINGLE_CHANNEL_AEC = 0
				//   OPTIBEAM_ARRAY_ONLY = 2
				//   OPTIBEAM_ARRAY_AND_AEC = 4
				//   SINGLE_CHANNEL_NSAGC = 5
				PROPVARIANT pvSysMode;
				PropVariantInit(&pvSysMode);
				pvSysMode.vt = VT_I4;
				pvSysMode.lVal = (LONG)(2); // Use OPTIBEAM_ARRAY_ONLY setting. Set OPTIBEAM_ARRAY_AND_AEC instead if you expect to have sound playing from speakers.
				pPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);
				PropVariantClear(&pvSysMode);

				// Set DMO output format
				WAVEFORMATEX wfxOut = {AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0};
				DMO_MEDIA_TYPE mt = {0};
				MoInitMediaType(&mt, sizeof(WAVEFORMATEX));

				mt.majortype = MEDIATYPE_Audio;
				mt.subtype = MEDIASUBTYPE_PCM;
				mt.lSampleSize = 0;
				mt.bFixedSizeSamples = TRUE;
				mt.bTemporalCompression = FALSE;
				mt.formattype = FORMAT_WaveFormatEx;	
				memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));

				hr = pDMO->SetOutputType(0, &mt, 0);

				if (SUCCEEDED(hr))
				{
					m_pKinectAudioStream = new KinectAudioStream(pDMO);

					hr = m_pKinectAudioStream->QueryInterface(IID_IStream, (void**)&pStream);

					if (SUCCEEDED(hr))
					{
						hr = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&m_pSpeechStream);

						if (SUCCEEDED(hr))
						{
							hr = m_pSpeechStream->SetBaseStream(pStream, SPDFID_WaveFormatEx, &wfxOut);
						}
					}
				}

				MoFreeMediaType(&mt);
			}
		}

		SafeRelease(pStream);
		SafeRelease(pPropertyStore);
		SafeRelease(pDMO);
		SafeRelease(pNuiAudioSource);

		return hr;
	}

	/// <summary>
	/// Create speech recognizer that will read Kinect audio stream data.
	/// </summary>
	/// <returns>
	/// <para>S_OK on success, otherwise failure code.</para>
	/// </returns>
	HRESULT SpeechControl::CreateSpeechRecognizer()
	{
		ISpObjectToken *pEngineToken = NULL;

		HRESULT hr = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&m_pSpeechRecognizer);

		if (SUCCEEDED(hr))
		{
			m_pSpeechRecognizer->SetInput(m_pSpeechStream, FALSE);

			if (SUCCEEDED(hr))
			{
				m_pSpeechRecognizer->SetRecognizer(pEngineToken);
				hr = m_pSpeechRecognizer->CreateRecoContext(&m_pSpeechContext);
				if (FAILED(hr)){
					exit(1);
				}
				// For long recognition sessions (a few hours or more), it may be beneficial to turn off adaptation of the acoustic model. 
				// This will prevent recognition accuracy from degrading over time.
				//if (SUCCEEDED(hr))
				//{
				//    hr = m_pSpeechRecognizer->SetPropertyNum(L"AdaptationOn", 0);                
				//}
			}
		}

		SafeRelease(pEngineToken);

		return hr;
	}

	/// <summary>
	/// Load speech recognition grammar into recognizer.
	/// </summary>
	/// <returns>
	/// <para>S_OK on success, otherwise failure code.</para>
	/// </returns>
	HRESULT SpeechControl::LoadSpeechGrammar()
	{
		HRESULT hr = m_pSpeechContext->CreateGrammar(1, &m_pSpeechGrammar);
		
		if (SUCCEEDED(hr))
		{
			// Populate recognition grammar from file
			hr = m_pSpeechGrammar->LoadCmdFromFile(GrammarFileName, SPLO_STATIC);
			if (SUCCEEDED(hr)){
				std::cout << "Poprawnie wczytano plik komend." << std::endl;
			}
			else{
				std::cout << "Nie uda³o siê wczytaæ pliku komend." << std::endl;
			}
		}

		return hr;
	}

	/// <summary>
	/// Start recognizing speech asynchronously.
	/// </summary>
	/// <returns>
	/// <para>S_OK on success, otherwise failure code.</para>
	/// </returns>
	HRESULT SpeechControl::StartSpeechRecognition()
	{
		HRESULT hr = m_pKinectAudioStream->StartCapture();

		if (SUCCEEDED(hr))
		{
			// Specify that all top level rules in grammar are now active
			m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

			// Specify that engine should always be reading audio
			m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

			// Specify that we're only interested in receiving recognition events
			m_pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

			// Ensure that engine is recognizing speech and not in paused state
			hr = m_pSpeechContext->Resume(0);
			if (SUCCEEDED(hr))
			{
				m_hSpeechEvent = m_pSpeechContext->GetNotifyEventHandle();
			}
		}

		return hr;
	}

	/// <summary>
	/// Process recently triggered speech recognition events.
	/// </summary>
	void SpeechControl::ProcessSpeech()
	{

		SPEVENT curEvent;
		ULONG fetched = 0;
		HRESULT hr = S_OK;

		m_pSpeechContext->GetEvents(1, &curEvent, &fetched);

		while (fetched > 0)
		{
			switch (curEvent.eEventId)
			{
			case SPEI_RECOGNITION:
				if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
				{
					// this is an ISpRecoResult
					ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
					SPPHRASE* pPhrase = NULL;

					hr = result->GetPhrase(&pPhrase);
					if (SUCCEEDED(hr))
					{
						if ((pPhrase->pProperties != NULL) && (pPhrase->pProperties->pFirstChild != NULL))
						{
							const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
							if (pSemanticTag->SREngineConfidence > m_ConfidenceThreshold)
							{
								LPCWSTR quit = L"QUIT";
								LPCWSTR move = L"MOVE";
								LPCWSTR stop = L"STOP";
								LPCWSTR track = L"TRACK";
								LPCWSTR stoptrack = L"STOPTRACKING";
								LPCWSTR movetogether = L"HANDTOGETHER";
								LPCWSTR moveseparately = L"HANDSEPARATELY";
								LPCWSTR changePlayer = L"CHANGEPLAYER";
								LPCWSTR switchObject = L"SWITCHOBJECT";
								LPCWSTR accurateMovement = L"ACCURATEMOVEMENT";
								LPCWSTR normalMovement = L"NORMALMOVEMENT";
								LPCWSTR debug = L"DEBUG";
									
								LPCWSTR speech = pSemanticTag->pszValue;

								std::wcout << speech << std::endl;

								if(0 == wcscmp(quit, speech)){
									sync->setState(QUIT);
								}else{
									if(0 == wcscmp(move, speech)){
										sync->setState(MOVE);
									}else{
										if (0 == wcscmp(stop, speech)){
											sync->setState(IDLE);
										}else{
											if (0 == wcscmp(track, speech)){
												sync->setTryTracking(true);
											}
											else{
												if (0 == wcscmp(stoptrack, speech)){
													sync->setLostTracking();
												}else{
													if (0 == wcscmp(movetogether, speech)){
														sync->setHandCommandsMode(MoveMode::TOGETHER);
													}else{
														if (0 == wcscmp(moveseparately, speech)){
															sync->setHandCommandsMode(MoveMode::SEPARATE);
														}else{
															if (0 == wcscmp(changePlayer, speech)){
																sync->requestPlayerChange();
															}else{
																if (0 == wcscmp(switchObject, speech)){
																	sync->switchObject();
																}else{
																	if (0 == wcscmp(accurateMovement, speech)){
																		sync->setMoveAccuracy(MoveAccuracy::ACCURATE);
																	}else{
																		if (0 == wcscmp(normalMovement, speech)){
																			sync->setMoveAccuracy(MoveAccuracy::NORMAL);
																		}else{
																			if (0 == wcscmp(debug, speech)){
																				std::cout << "lol" << std::endl;
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
								
							}
						}
						::CoTaskMemFree(pPhrase);
					}
				}
				break;
			}

			m_pSpeechContext->GetEvents(1, &curEvent, &fetched);
		}

		return;
	}
}