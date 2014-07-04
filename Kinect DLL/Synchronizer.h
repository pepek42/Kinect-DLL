#pragma once

#include "KinectDLL.h"
#include "GL\freeglut.h"
#include "HandCommands.h"

namespace KinectDLL{

	/// <summary>
	/// Class that allows to safely exchange data between threads
	/// </summary>
	class Synchronizer
	{
	private:
		/// <summary>
		/// Locks data so that no other thread can change it
		/// </summary>
		void logIn();

		/// <summary>
		/// Unlocks data so that they are available for other threads
		/// </summary>
		void logOut();

		/// Critical Section
		CRITICAL_SECTION cs;

		// Hand commands that are set by depth reader
		HandCommands m_handCommands;

		// Information about move mode
		MoveAccuracy m_movePrecision;

		// State of dll
		States m_state;

		// Is Kinect init finished
		bool depthReaderReady;

		// consts
		static const int c_width = 640;
		static const int c_height = 480;
		static const int c_BytesPerPixel = 4;

		// Array for texture
		GLubyte* m_pTexture;

		// texture size
		int m_arraySize = c_width*c_height * c_BytesPerPixel;

		// int for player chooser
		int m_playerChooser;

		// are we tracking left hand
		bool m_TrackingLeft;

		// are we tracking right hand
		bool m_TrackingRight;

		// user request to change player
		bool m_ChangePlayer;

		// last known left hand position (x and y - pixel position, z - mm from sensor)
		MyPoint m_HandLeft;

		// last known right hand position (x and y - pixel position, z - mm from sensor)
		MyPoint m_HandRight;

		// user requested to track point that is shown currently with white area
		bool m_TryTracking;

		// Index of choosen object. Current index is determined by operation m_ObjectChooser % m_NumberOfObjects
		UINT m_ObjectChooser;
		
		// Number of objects
		UINT m_NumberOfObjects;

	public:
		
		/// <summary>
		/// Constructor
		/// </summary>
		KINECTDLL_API Synchronizer(void);

		/// <summary>
		/// Destructor
		/// </summary>
		KINECTDLL_API ~Synchronizer(void);


		/// <summary>
		/// Gives texture data
		/// </summary>
		/// <param name="dest">Array for texture</param>
		KINECTDLL_API void getTexture(GLubyte* dest);

		/// <summary>
		/// Gives texture data
		/// </summary>
		/// <param name="dest">Array with texture</param>
		KINECTDLL_API void setTexture(GLubyte* dest);

		/// <summary>
		/// Sets application state requested by user (via speech control or from application) 
		/// </summary>
		/// <param name="stateToSet">New application state</param>
		KINECTDLL_API void setState(States stateToSet);

		/// <summary>
		/// Gives information about current application state
		/// </summary>
		/// <returns>Current application state</returns>
		KINECTDLL_API States getState(){ logIn(); States ret = m_state; logOut(); return ret; };

		/// <summary>
		/// Gives information about current move mode (slow or normal)
		/// </summary>
		/// <returns>Move mode</returns>
		KINECTDLL_API MoveAccuracy getMoveAccuracy(){ logIn(); MoveAccuracy ret = m_movePrecision; logOut(); return ret; };

		/// <summary>
		/// Gives information about current move mode (slow or normal)
		/// </summary>
		/// <returns>Move mode</returns>
		KINECTDLL_API void setMoveAccuracy(MoveAccuracy movePrecision_){ logIn(); m_movePrecision = movePrecision_; logOut(); };


		/// <summary>
		/// Gives information about depth reader state (if it is initialize or not). Kinect need to be initialized before runing speech contorl module.
		/// </summary>
		/// <returns>Is kinect ready</returns>
		KINECTDLL_API bool isKinectReady(){ logIn(); bool ret = depthReaderReady; logOut(); return ret; };

		/// <summary>
		/// Sets info that depth reader thread is initialized 
		/// </summary>
		KINECTDLL_API void setKinectReady(){ logIn(); depthReaderReady = true; logOut(); };


		/// <summary>
		/// Sets player change request and increases player index (m_playerChooser % number of players = index of current player)
		/// </summary>
		KINECTDLL_API void requestPlayerChange(){ logIn(); m_ChangePlayer = true; ++m_playerChooser; logOut(); };

		/// <summary>
		/// Returns change player request to depth reading thread
		/// </summary>
		/// <returns>If player requested for player change. If true depth reading thread will change player to next index.</returns>
		KINECTDLL_API bool isChangePlayerRequest(){ logIn(); bool ret = m_ChangePlayer; logOut(); return ret; };

		/// <summary>
		/// Confirms change of player and flags m_ChangePlayer as false.
		/// </summary>
		KINECTDLL_API void setPlayerChanged(){ logIn(); m_ChangePlayer = false; logOut(); };

		/// <summary>
		/// Returns m_playerChooser. m_playerChooser% number_of_players gives index of current player. 
		/// </summary>
		/// <returns>Player</returns>
		KINECTDLL_API int getPlayerChooser(){ logIn(); int ret = m_playerChooser; logOut(); return ret; };


		/// <summary>
		/// Returns player request to try set currently observed point as hand point.
		/// </summary>
		/// <returns>Is player requesting to track currently observed point.</returns>
		KINECTDLL_API bool isTryTracking(){ logIn(); bool ret = m_TryTracking; logOut(); return ret; };

		/// <summary>
		/// Sets player request to try set currently observed point as hand point.
		/// </summary>
		/// <param name="b">Is player requesting to track currently observed point.</param>
		KINECTDLL_API void setTryTracking(bool b){ logIn(); m_TryTracking = b; logOut(); };


		/// <summary>
		/// Sets right hand position for the first time.
		/// </summary>
		/// <param name="handRight">Right hand 3D point</param>
		KINECTDLL_API void setRightHandTracking(MyPoint handRight){ logIn(); m_HandRight = handRight; m_TryTracking = false; m_TrackingRight = true; logOut(); };

		/// <summary>
		/// Sets left hand position for the first time.
		/// </summary>
		/// <param name="handLeft">Left hand 3D point</param>
		KINECTDLL_API void setLeftHandTracking(MyPoint handLeft){ logIn(); m_HandLeft = handLeft; m_TryTracking = false; m_TrackingLeft = true; logOut(); };


		/// <summary>
		/// Updates right hand position.
		/// </summary>
		/// <param name="handRight">Right hand 3D point</param>
		KINECTDLL_API void setHandRight(MyPoint handRight){ logIn(); m_HandRight = handRight; logOut(); };

		/// <summary>
		/// Updates both hands position.
		/// </summary>
		/// <param name="handRight">Right hand 3D point</param>
		/// <param name="handLeft">Left hand 3D point</param>
		KINECTDLL_API void setBothHands(MyPoint handRight, MyPoint handLeft) { m_HandLeft = handLeft; m_HandRight = handRight; };

		/// <summary>
		/// Gives information about tracking state
		/// </summary>
		/// <param name="trackingLeft">gets info if left hand is tracked</param>
		/// <param name="trackingRight">gets info if right hand is tracked</param>
		/// <param name="handLeft">gets position of left hand</param>
		/// <param name="handRight">gets position of right hand</param>
		KINECTDLL_API void getTracking(bool &trackingLeft, bool &trackingRight, MyPoint &handLeft, MyPoint &handRight);

		/// <summary>
		/// Clears hands positions and sets tracking flags to false.
		/// </summary>
		KINECTDLL_API void setLostTracking();


		/// <summary>
		/// Gives hand commands
		/// </summary>
		/// <returns>Current hand commands.</returns>
		KINECTDLL_API HandCommands getHandCommands(){ logIn(); HandCommands ret = m_handCommands; logOut(); return ret; };

		/// <summary>
		/// Sets new hand commnads.
		/// </summary>
		/// <param name="_handCommands">Hand commands to set</param>
		KINECTDLL_API void setHandCommands(HandCommands _handCommands){ logIn(); m_handCommands = _handCommands; logOut(); };

		/// <summary>
		/// Sets interpretation mode for moving hands.
		/// </summary>
		/// <param name="handCommandsSeparateMode">Hand commands interpretation mode</param>
		KINECTDLL_API void setHandCommandsMode(MoveMode handCommandsSeparateMode){ logIn(); m_handCommands.setMoveMode(handCommandsSeparateMode); logOut(); };


		/// <summary>
		/// Increments currnet object index.
		/// </summary>
		KINECTDLL_API void switchObject(){ logIn(); ++m_ObjectChooser; logOut(); }

		/// <summary>
		/// Sets number of movable objects
		/// </summary>
		/// <param name="number">Number of movable objects</param>
		KINECTDLL_API void setNumberOfObjects(UINT number){ logIn(); m_NumberOfObjects = number; logOut(); }

		/// <summary>
		/// Returns index of object that application should be moving,
		/// </summary>
		/// <returns>Index</returns>
		KINECTDLL_API UINT getObject(){ logIn(); m_ObjectChooser = m_ObjectChooser % m_NumberOfObjects; UINT ret = m_ObjectChooser; logOut(); return ret; }

	};

}