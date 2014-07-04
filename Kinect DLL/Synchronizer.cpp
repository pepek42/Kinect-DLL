#include "stdafx.h"
#include "Synchronizer.h"

namespace KinectDLL{
	Synchronizer::Synchronizer(void) :
		depthReaderReady(false),
		m_playerChooser(1),
		m_state(IDLE),
		m_movePrecision(NORMAL),
		m_TrackingLeft(false),
		m_TrackingRight(false),
		m_TryTracking(false),
		m_ObjectChooser(0),
		m_NumberOfObjects(1)
	{
		InitializeCriticalSection(&cs);
		m_pTexture = new GLubyte[c_width*c_height * c_BytesPerPixel];
		m_NumberOfObjects = GetPrivateProfileInt(TEXT("Synchronizer"),
			TEXT("m_NumberOfObjects "),
			2,
			path);;
	}


	Synchronizer::~Synchronizer(void){
		DeleteCriticalSection(&cs);
		delete m_pTexture;
	}

	void Synchronizer::logIn()
	{
		EnterCriticalSection(&cs);

	}

	void Synchronizer::logOut()
	{
		LeaveCriticalSection(&cs);
	}

	
	void Synchronizer::setTexture(GLubyte* dest){
		logIn();

		memcpy_s(m_pTexture, m_arraySize, dest, m_arraySize);

		logOut();
	}

	void Synchronizer::getTexture(GLubyte* dest){
		logIn();

		memcpy_s(dest, m_arraySize, m_pTexture, m_arraySize);

		logOut();
	}

	void Synchronizer::getTracking(bool &trackingLeft, bool &trackingRight, MyPoint &handLeft, MyPoint &handRight)
	{
		logIn();
		trackingLeft = m_TrackingLeft;
		trackingRight = m_TrackingRight;
		handLeft = m_HandLeft;
		handRight = m_HandRight;
		logOut();
	}

	void Synchronizer::setLostTracking() { 
		logIn();
		m_TrackingLeft = false;
		m_TrackingRight = false;
		m_state = States::IDLE;
		m_HandLeft.clear();
		m_HandRight.clear();
		logOut();
		std::cout << "LOST TRACKING" << std::endl;
	}

	void Synchronizer::setState(States stateToSet){ 
		logIn();
		if (stateToSet == States::MOVE && !m_TrackingLeft && !m_TrackingRight){
			stateToSet = States::IDLE;
		}
		m_state = stateToSet;
		logOut(); 
	}

}