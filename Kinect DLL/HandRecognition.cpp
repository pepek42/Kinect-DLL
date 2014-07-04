#include "small.h"
#include "HandRecognition.h"

namespace KinectDLL
{
	HandRecognition::HandRecognition() :
		KinectRenderer(),
		HandCommandsRecognizer(),
		m_nearMode(FALSE),
		m_depthBufferSize(0),
		m_depthBufferPitch(0),
		m_playerIndex(0),
		sync(NULL),
		m_CoutFramesTryTracking(0),
		m_CoutFramesNoHand(0),
		m_CoutFramesNoHand2(0),
		m_PreviousState(States::IDLE)
	{
		m_pPlayers = new bool[m_numberOfPlayers];
		m_pDepthBuffer = new BYTE[c_DepthHeight*c_DepthWidth*c_BytesPerPixel];

		m_ThresholdXY = static_cast<USHORT>(GetPrivateProfileInt(TEXT("HandRecognition"),
			TEXT("m_ThresholdXY"),
			45,
			path));

		m_ThresholdZ = static_cast<USHORT>(GetPrivateProfileInt(TEXT("HandRecognition"),
			TEXT("m_ThresholdZ"),
			30,
			path));

		m_ThresholdZnewPosition = static_cast<USHORT>(GetPrivateProfileInt(TEXT("HandRecognition"),
			TEXT("m_ThresholdZnewPosition"),
			15,
			path));

		m_ThresholdXYnewPosition = static_cast<USHORT>(GetPrivateProfileInt(TEXT("HandRecognition"),
			TEXT("m_ThresholdXYnewPosition"),
			25,
			path));

		m_ThresholdFollow = static_cast<USHORT>(GetPrivateProfileInt(TEXT("HandRecognition"),
			TEXT("m_ThresholdFollow"),
			25,
			path));

		m_ThresholdClosest = static_cast<USHORT>(GetPrivateProfileInt(TEXT("HandRecognition"),
			TEXT("m_ThresholdClosest"),
			7,
			path));

		m_ThresholdLostTracking = static_cast<USHORT>(GetPrivateProfileInt(TEXT("HandRecognition"),
			TEXT("m_ThresholdLostTracking"),
			30,
			path));
	}


	HandRecognition::~HandRecognition()
	{
		delete[] m_pPlayers;
	}

	void HandRecognition::getHands(BYTE* depthBuffer){

		clearPlayers();
		setBasicTexture(depthBuffer, m_pPlayers);
		if (followPlayer()){
			bool trackingLeft;
			bool trackingRight;

			MyPoint handLeft;
			MyPoint handRight;

			sync->getTracking(trackingLeft, trackingRight, handLeft, handRight);
			if (trackingRight){
				if (trackingLeft){
					SBSMall(handRight, handLeft);
				}else{
					SBSMFindLeft(handRight);
				}
			}else{
				SBSMFindAny();
			}
		}
	}

	void HandRecognition::SBSMFindAny(){
		NUI_DEPTH_IMAGE_PIXEL* pBufferRun = reinterpret_cast<NUI_DEPTH_IMAGE_PIXEL*>(m_pDepthBuffer);

		USHORT smallz = USHRT_MAX;
		MyPoint frontPoint;

		// Get the min and max reliable depth for the current frame
		int minDepth = (m_nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
		int maxDepth = (m_nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

		int bufferSize = c_DepthWidth*c_DepthHeight;

		//looking for front point that belongs to player
		for (int i = 0; i < bufferSize; ++i){

			if (pBufferRun[i].playerIndex == m_playerIndex){

				USHORT depth = static_cast<USHORT>(pBufferRun[i].depth >= minDepth && pBufferRun[i].depth <= maxDepth ? pBufferRun[i].depth : 0);

				if (depth < smallz && depth > 0){
					//we have it...
					smallz = depth;
					frontPoint.x = i%c_DepthWidth;					//x
					frontPoint.y = i / c_DepthWidth;				//y
					frontPoint.z = depth;							//z
				}

			}
		}
		
		//for middle point
		int accx = 0;
		int accy = 0;
		int accz = 0;
		int pcout = 0;

		//Z axis threshold
		int zthreshold = smallz + m_ThresholdZ;

		//looking for points near front point
		for (int i = 0; i < bufferSize; ++i){
			USHORT depth = static_cast<USHORT>(pBufferRun[i].depth >= minDepth && pBufferRun[i].depth <= maxDepth ? pBufferRun[i].depth : 0);
			if (pBufferRun[i].playerIndex == m_playerIndex && depth < zthreshold && depth > 0){

				int bx = i%c_DepthWidth;
				int by = i / c_DepthWidth;
				//if is between threshold and fron then check if it is in aria of front point
				if (sqrt(pow((bx - frontPoint.x), 2) + pow((by - frontPoint.y), 2)) < m_ThresholdXY){
					memset(m_pTexture+(i * 4), 0xff, 3);
					accx += bx;
					accy += by;
					accz += depth;
					++pcout;
				}
			}
		}

		//if we have cloud lets see where the point is
		if (pcout > 50){
			//getting and rendering point
			MyPoint newPoint(accx / pcout, accy / pcout, accz / pcout);
			setPoint(newPoint, 0x99, 0x66, 0x33, 3);
			//if wa are trying to track point
			if (sync->isTryTracking()){
				if (newPoint == pointTMP){				//checking if points are in similar positions...
					if (++m_CoutFramesTryTracking > 3){		//if point didn't move after 4 frames it is that...
						sync->setRightHandTracking(newPoint);
						m_CoutFramesTryTracking = 0;
					}
				}else{
					//if not near each other lets set new point as tmp point because old one might be in space...
					pointTMP = newPoint;
					m_CoutFramesTryTracking = 0;
				}
			}
		}else
			m_CoutFramesTryTracking = 0;

	}

	void HandRecognition::SBSMFindLeft(MyPoint handRight)
	{
		NUI_DEPTH_IMAGE_PIXEL* pBufferRun = reinterpret_cast<NUI_DEPTH_IMAGE_PIXEL*>(m_pDepthBuffer);

		USHORT smallz = USHRT_MAX;
		MyPoint frontPoint;

		USHORT smallzRight = USHRT_MAX;
		MyPoint frontPointRight;

		// Get the min and max reliable depth for the current frame
		int minDepth = (m_nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
		int maxDepth = (m_nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

		int bufferSize = c_DepthWidth*c_DepthHeight;

		for (int i = 0; i < bufferSize; ++i){

			if (pBufferRun[i].playerIndex == m_playerIndex){

				USHORT depth = static_cast<USHORT>(pBufferRun[i].depth >= minDepth && pBufferRun[i].depth <= maxDepth ? pBufferRun[i].depth : 0);
				//looking for front point that belongs to player
				if (depth < smallz && depth > 0){
					smallz = depth;
					frontPoint.x = i%c_DepthWidth;					//x
					frontPoint.y = i / c_DepthWidth;				//y
					frontPoint.z = depth;							//z
				}

				////looking for front point that belongs to player and is near old position of right hand
				if (depth < smallzRight && sqrt(pow(handRight.x - i%c_DepthWidth, 2) + pow(handRight.y - i / c_DepthWidth, 2)) < m_ThresholdFollow && depth > 0){
					smallzRight = depth;
					frontPointRight.x = i%c_DepthWidth;					//x
					frontPointRight.y = i / c_DepthWidth;				//y
					frontPointRight.z = depth;							//z
				}

			}
		}

		//for new looking for left hand...
		int accx = 0;
		int accy = 0;
		int accz = 0;
		int pcout = 0;

		USHORT zthreshold = smallz + m_ThresholdZ;

		//for new position of right hand
		int accxRight = 0;
		int accyRight = 0;
		int acczRight = 0;
		int pcoutRight = 0;

		USHORT zthresholdRight = smallzRight + m_ThresholdZnewPosition;

		for (int i = 0; i < bufferSize; ++i){
			USHORT depth = static_cast<USHORT>(pBufferRun[i].depth >= minDepth && pBufferRun[i].depth <= maxDepth ? pBufferRun[i].depth : 0);
			if (pBufferRun[i].playerIndex == m_playerIndex && depth > 0){

				int bx = i%c_DepthWidth;
				int by = i / c_DepthWidth;

				//looking for left hand
				if (depth < zthreshold 
					&& sqrt(pow((bx - frontPoint.x), 2) + pow((by - frontPoint.y), 2)) < m_ThresholdXY){
					memset(m_pTexture + (i * 4), 0xff, 3);
					accx += bx;
					accy += by;
					accz += depth;
					++pcout;
				}

				//looking for right hand 
				if (sqrt(pow(frontPointRight.x - bx, 2) + pow(frontPointRight.y - by, 2)) < m_ThresholdXYnewPosition && depth < zthresholdRight) {
					memset(m_pTexture + (i * 4), 0xCC, 3);
					accxRight += bx;
					accyRight += by;
					acczRight += depth;
					++pcoutRight;
				}
			}
		}

		//if we have cloud lets see where the point is
		if (pcout > 50){
			MyPoint newPoint(accx / pcout, accy / pcout, accz / pcout);
			setPoint(newPoint, 0x99, 0x66, 0x33, 3);
			if (sync->isTryTracking()){
				if (newPoint == pointTMP){
					++m_CoutFramesTryTracking;
					if (m_CoutFramesTryTracking > 3){
						sync->setLeftHandTracking(newPoint);
						m_CoutFramesTryTracking = 0;
					}
				}else{
					pointTMP = newPoint;
					m_CoutFramesTryTracking = 0;
				}
			}
		}

		//following right hand point
		if (pcoutRight > 50){
			m_CoutFramesNoHand = 0;
			MyPoint handRightNew(accxRight / pcoutRight, accyRight / pcoutRight, acczRight / pcoutRight);
			setPoint(handRightNew, 0x99, 0x66, 0x33, 5);
			sync->setHandRight(handRightNew);
		}else{
			//if we cant seee it lets cout how many frames it was. If too much, then we lost right hand..
			if (++m_CoutFramesNoHand > m_ThresholdLostTracking){
				sync->setLostTracking();
			}
		}

	}

	void HandRecognition::SBSMall(MyPoint handRight, MyPoint handLeft){

		setPoint(handRight, 0xff, 0xff, 0xff, 3);
		setPoint(handLeft, 0xff, 0xff, 0xff, 3);

		NUI_DEPTH_IMAGE_PIXEL* pBufferRun = reinterpret_cast<NUI_DEPTH_IMAGE_PIXEL*>(m_pDepthBuffer);

		MyPoint frontPointLeft;
		frontPointLeft.z = USHRT_MAX;
		
		MyPoint frontPointRight;
		frontPointRight.z = USHRT_MAX;

		// Get the min and max reliable depth for the current frame
		int minDepth = (m_nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
		int maxDepth = (m_nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

		int bufferSize = c_DepthWidth*c_DepthHeight;


		NUI_DEPTH_IMAGE_PIXEL pixel;
		NUI_DEPTH_IMAGE_PIXEL pixelHelp;
		USHORT depth;
		USHORT x;
		USHORT y;
		bool allowPoint;
		int coor;
		int i;
		int j;
		int k;
		USHORT depthHelp;

		int smallRad = -3;
		int smallRadMod = abs(smallRad);

		for (i = 0; i < bufferSize; ++i){
			pixel = pBufferRun[i];
			if (pixel.playerIndex == m_playerIndex){
				
				depth = static_cast<USHORT>(pixel.depth >= minDepth && pixel.depth <= maxDepth ? pixel.depth : 0);

				x = i%c_DepthWidth;
				y = i / c_DepthWidth;

				//looking for front point that belongs to player
				if (depth < frontPointLeft.z && sqrt(pow(handLeft.x - x, 2) + pow(handLeft.y - y, 2)) < m_ThresholdFollow && depth > 0){
					allowPoint = true;
					for (j = smallRad; j <= smallRadMod; ++j){
						if (!allowPoint){
							break;
						}
						for (k = smallRad; k <= smallRadMod; ++k){
							coor = (x + j) + (y + k)*c_DepthWidth;
							if (coor < bufferSize && coor > 0){
								pixelHelp = pBufferRun[coor];
								depthHelp = static_cast<USHORT>(pixelHelp.depth >= minDepth && pixelHelp.depth <= maxDepth ? pixelHelp.depth : 0);
								if (abs(depth - depthHelp) > m_ThresholdClosest){
									allowPoint = false;
									break;
								}
							}
						}
					}
					if (allowPoint){
						frontPointLeft.x = x;								//x
						frontPointLeft.y = y;								//y
						frontPointLeft.z = depth;							//z
					}
				}

				////looking for front point that belongs to player and is near old position of right hand
				if (depth < frontPointRight.z && sqrt(pow(handRight.x - x, 2) + pow(handRight.y - y, 2)) < m_ThresholdFollow && depth > 0){
					allowPoint = true;
					for (j = smallRad; j <= smallRadMod; ++j){
						if (!allowPoint){
							break;
						}
						for (k = smallRad; j <= smallRadMod; ++j){
							coor = (x + j) + (y + k)*c_DepthWidth;
							if (coor < bufferSize && coor > 0){
								pixelHelp = pBufferRun[coor];
								depthHelp = static_cast<USHORT>(pixelHelp.depth >= minDepth && pixelHelp.depth <= maxDepth ? pixelHelp.depth : 0);
								if (abs(depth - depthHelp) > m_ThresholdClosest){
									allowPoint = false;
									break;
								}
							}
						}
					}
					if (allowPoint){
						frontPointRight.x = x;								//x
						frontPointRight.y = y;								//y
						frontPointRight.z = depth;							//z
					}
				}
			}
		}

		//for new looking for left hand...
		int accxLeft = 0;
		int accyLeft = 0;
		int acczLeft = 0;
		int pcoutLeft = 0;

		USHORT zthresholdLeft = frontPointLeft.z + m_ThresholdZnewPosition;

		//for new position of right hand
		int accxRight = 0;
		int accyRight = 0;
		int acczRight = 0;
		int pcoutRight = 0;

		USHORT zthresholdRight = frontPointRight.z + m_ThresholdZnewPosition;

		short rad_minus = -m_ThresholdXYnewPosition;
		short rad = abs(rad_minus);

		int bx;
		int by;

		//left hand
		for (i = rad_minus; i <= rad; ++i){
			for (j = rad_minus; j <= rad; ++j){
				coor = (frontPointLeft.x + i) + (frontPointLeft.y + j)*c_DepthWidth;
				if (coor < bufferSize && coor > 0){
					pixel = pBufferRun[coor];
					if (pixel.playerIndex == m_playerIndex){
						depth = static_cast<USHORT>(pixel.depth >= minDepth && pixel.depth <= maxDepth ? pixel.depth : 0);
						if (depth < zthresholdLeft){
							bx = coor%c_DepthWidth;
							by = coor / c_DepthWidth;
							if (sqrt(pow((bx - frontPointLeft.x), 2) + pow((by - frontPointLeft.y), 2)) < m_ThresholdXYnewPosition){
								memset(m_pTexture + (coor * 4), 0xCC, 3);
								accxLeft += bx;
								accyLeft += by;
								acczLeft += depth;
								++pcoutLeft;
							}
						}
					}
				}
			}
		}

		for (i = rad_minus; i <= rad; ++i){
			for (j = rad_minus; j <= rad; ++j){
				coor = (frontPointRight.x + i) + (frontPointRight.y + j)*c_DepthWidth;
				if (coor < bufferSize && coor > 0){
					pixel = pBufferRun[coor];
					if (pixel.playerIndex == m_playerIndex){
						depth = static_cast<USHORT>(pixel.depth >= minDepth && pixel.depth <= maxDepth ? pixel.depth : 0);
						if (depth < zthresholdRight){
							bx = coor%c_DepthWidth;
							by = coor / c_DepthWidth;
							if (sqrt(pow((bx - frontPointRight.x), 2) + pow((by - frontPointRight.y), 2)) < m_ThresholdXYnewPosition){
								memset(m_pTexture + (coor * 4), 0xCC, 3);
								accxRight += bx;
								accyRight += by;
								acczRight += depth;
								++pcoutRight;
							}
						}
					}
				}
			}
		}

		MyPoint handLeftNew;
		MyPoint handRightNew;

		//following Left hand point
		if (pcoutLeft > 50){
			m_CoutFramesNoHand2 = 0;
			handLeftNew = MyPoint(accxLeft / pcoutLeft, accyLeft / pcoutLeft, acczLeft / pcoutLeft);
			setPoint(handLeftNew, 0x99, 0x66, 0x33, 5);
		}
		else{
			//if we cant see it lets cout how many frames it was. If too much, then we lost right hand..
			if (++m_CoutFramesNoHand2 > m_ThresholdLostTracking){
				sync->setLostTracking();
			}
		}

		//following right hand point
		if (pcoutRight > 50){
			m_CoutFramesNoHand = 0;
			handRightNew = MyPoint(accxRight / pcoutRight, accyRight / pcoutRight, acczRight / pcoutRight);
			setPoint(handRightNew, 0x99, 0x66, 0x33, 5);
		}
		else{
			//if we cant seee it lets cout how many frames it was. If too much, then we lost right hand..
			if (++m_CoutFramesNoHand > m_ThresholdLostTracking){
				sync->setLostTracking();
			}
		}

		States state = sync->getState();

		if (handRightNew.z != 0 && handLeftNew.z != 0){
			sync->setBothHands(handRightNew, handLeftNew);
			if (state == States::MOVE){
				HandCommands HC = sync->getHandCommands();
				HC = getCommancds(HC, handLeftNew, handRightNew, m_PreviousState == States::IDLE);
				if (HC.isValid()){
					sync->setHandCommands(HC);
				}
			}
		}
		
		m_PreviousState = state;
		
	}

	bool HandRecognition::followPlayer()
	{
		if (m_pPlayers[m_playerIndex] && m_playerIndex && !sync->isChangePlayerRequest()){
			return true;		//if player that we follow exists it's ok
		}else{
			//get number of active players, start loop from 1, o is not a player
			USHORT numberOfActivPlayers = 0;
			for (USHORT i = 1; i < m_numberOfPlayers; ++i){
				if (m_pPlayers[i]){
					++numberOfActivPlayers;
				}
			}
			//get number of player we need to to bypass
			if (numberOfActivPlayers){
				USHORT index = sync->getPlayerChooser() % numberOfActivPlayers;
				numberOfActivPlayers = 0;
				//get player index
				for (USHORT i = 1; i < m_numberOfPlayers; ++i){
					if (m_pPlayers[i]){
						if (numberOfActivPlayers++ == index){
							m_playerIndex = i;
							sync->setPlayerChanged();
							return true;
						}
					}
				}
				return false;
			}
			return false;
		}
	}

	void HandRecognition::clearPlayers()
	{
		memset(m_pPlayers, false, sizeof(bool)*m_numberOfPlayers);
	}
}