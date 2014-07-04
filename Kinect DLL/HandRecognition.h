#pragma once

#include <ole2.h>
#include <Windows.h>
#include "NuiApi.h"
#include "KinectDLL.h"

#include "Synchronizer.h"
#include "KinectRenderer.h"
#include "HandCommandsRecognizer.h"
namespace KinectDLL
{
	class HandRecognition : protected KinectRenderer, HandCommandsRecognizer
	{
	protected:
		Synchronizer* sync;

		/// bool that is getting info from kinect if it is runnig near mode
		BOOL m_nearMode;

		int m_depthBufferSize;

		int m_depthBufferPitch;

		/// depth buffer storing all depth map bytes from Kinect
		BYTE* m_pDepthBuffer;

		/// pointer to array storing info about players recognised by Kinect
		bool* m_pPlayers;

		USHORT const m_numberOfPlayers = 8;

		/// index of currently observed player
		USHORT m_playerIndex;

		/// <summary>
		/// Constructor
		/// </summary>
		HandRecognition();

		/// <summary>
		/// Destructor
		/// </summary>
		~HandRecognition();

		//HWND m_hWnd;
		/// <summary>
		/// Preapares and executes hand searching
		/// </summary>
		/// <param name="depthBuffer">Pointer to depht map</param>
		void getHands(BYTE* depthBuffer);

	private:
		/// Temporary point for accepting candidate point for hand.
		MyPoint pointTMP;

		/// number of frames for point beeing candidate for hand that have hand in the same position
		USHORT m_CoutFramesTryTracking;

		/// Number of frames while hand is gone.
		USHORT m_CoutFramesNoHand;

		/// number of frames while hand is gone
		USHORT m_CoutFramesNoHand2;

		/// Threshold (radius) for XY plate. Used in sacond phase (we have closest point, we are looking for hand). For searching hand for the first time.
		USHORT m_ThresholdXY;

		/// Threshold for Z depth. Everything closer to this + closest point Z value and is in radious of m_ThresholdXY from front point is considered to be a hand point. For searching hand for the first time.
		USHORT m_ThresholdZ;

		/// Threshold for Z depth. Everything closer to this + closest point Z value and is in radious of m_ThresholdXY from front point is considered to be a hand point. For searching hand that we had in previous frame.
		USHORT m_ThresholdZnewPosition;

		/// Threshold (radius) for XY plate. Used in sacond phase (we have closest point, we are looking for hand). For searching hand that we had in previous frame.
		USHORT m_ThresholdXYnewPosition;

		/// Threshold (radius near old point) for searching new closest point. For searching front point of hand that we had in previous frame.
		USHORT m_ThresholdFollow;

		/// Threshold for difference betwen closest point candidate Z value and near points Z value. If all point near closest point candidate have Z axis value in m_ThresholdClosest range point is accepted as new closest
		USHORT m_ThresholdClosest;

		/// Number of frames with no hand location found before we admit that we lost track of hands...
		USHORT m_ThresholdLostTracking;

		/// Previous frame app state. If changed from idle to move then we are setting starting points
		States m_PreviousState;

		/// <summary>
		/// Looking for right hand
		/// </summary>
		void SBSMFindAny();

		/// <summary>
		/// Looking for left hand and following right
		/// </summary>
		void SBSMFindLeft(MyPoint handRight);

		/// <summary>
		/// Main algorithm for getting hands
		/// </summary>
		/// <param name="handRight">Previous right hand position</param>
		/// <param name="handLeft">Previous left hand position</param>
		void SBSMall(MyPoint handRight, MyPoint handLeft);

		/// <summary>
		/// Sets all players as absent
		/// </summary>
		void clearPlayers();		

		/// <summary>
		/// Checks if we still have player with m_playerIndex index array pointed by m_pPlayers or if player send change player request. If change player request is sent or we dont have player function is looking for active player.
		/// </summary>
		bool followPlayer();
	};
}