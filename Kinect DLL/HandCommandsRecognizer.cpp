#include "stdafx.h"
#include "HandCommandsRecognizer.h"
#include "KinectRenderer.h"
#include <NuiApi.h>
#include "Tester.h"//michal ppk

namespace KinectDLL{
	HandCommandsRecognizer::HandCommandsRecognizer()
	{
		TCHAR   inBuf[100];
		m_avrgCount = GetPrivateProfileInt(TEXT("HandCommandsRecognizer"),
			TEXT("m_avrgCount "),
			30,
			path);

		GetPrivateProfileString(TEXT("HandCommandsRecognizer"),
			TEXT("thresholdMovedTogeder "),
			TEXT("0.01"),
			inBuf,
			80,
			path);

		thresholdMovedTogeder = _tcstod(inBuf, NULL);
		
		if (m_avrgCount == 0){
			std::cout << "Wrong m_avrgCount..." << std::endl;
			getchar();
			exit(1);
		}
		m_pAvrgLeft = new MyPointFloat[m_avrgCount];
		m_pAvrgRight = new MyPointFloat[m_avrgCount];
	}


	HandCommandsRecognizer::~HandCommandsRecognizer()
	{
		delete[] m_pAvrgLeft;
		delete[] m_pAvrgRight;
	}

	HandCommands HandCommandsRecognizer::getCommancds(HandCommands previous, MyPoint handLeft, MyPoint handRight, bool firstMoveModeTime){

		if (m_avrgCount > 1){
			for (int i = 0; i < m_avrgCount - 1; ++i){
				m_pAvrgLeft[i + 1] = m_pAvrgLeft[i];
				m_pAvrgRight[i + 1] = m_pAvrgRight[i];
			}
		}

		int width = KinectRenderer::c_DepthWidth;
		int height = KinectRenderer::c_DepthHeight;

		//depth coordinate system to sceleton, we need everything in the same unit (meters)
		//from NuiTransformDepthImageToSkeleton
		m_pAvrgLeft[0].z = static_cast<FLOAT>(handLeft.z) / 1000.0f;
		m_pAvrgRight[0].z = static_cast<FLOAT>(handRight.z) / 1000.0f;

		m_pAvrgLeft[0].x = (handLeft.x - width / 2.0f) * (320.0f / width) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * m_pAvrgLeft[0].z;
		m_pAvrgLeft[0].y = -(handLeft.y - height / 2.0f) * (240.0f / height) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * m_pAvrgLeft[0].z;

		m_pAvrgRight[0].x = (handRight.x - width / 2.0f) * (320.0f / width) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * m_pAvrgRight[0].z;
		m_pAvrgRight[0].y = -(handRight.y - height / 2.0f) * (240.0f / height) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * m_pAvrgRight[0].z;

		//getting commands
		MyPointFloat test;
		MyPointFloat left;
		MyPointFloat right;
		USHORT count = 0;
		for (int i = 0; i < m_avrgCount; ++i){
			if (m_pAvrgLeft[i] != test && m_pAvrgRight[i] != test){
				left += m_pAvrgLeft[i];
				right += m_pAvrgRight[i];
				++count;
			}
		}
		if (count > 0){
			left = left / count;

			right = right / count;
		}

		Commands commands;

		//getting difference between old position and current
		float difXLeft = previous.currentLeft.x - left.x;
		float difXRight = previous.currentRigth.x - right.x;

		float difYLeft = previous.currentLeft.y - left.y;
		float difYRight = previous.currentRigth.y - right.y;

		float difZLeft = previous.currentLeft.z - left.z;
		float difZRight = previous.currentRigth.z - right.z;

		/*michal ppk*
		std::string dane = "";
		dane = dane + std::to_string(m_pAvrgLeft[0].x) + ";" + std::to_string(m_pAvrgLeft[0].y) + ";" + std::to_string(m_pAvrgLeft[0].z) + ";"
			+ std::to_string(left.x) + ";" + std::to_string(left.y) + ";" + std::to_string(left.z);

		Tester::logToFile("pointLOG.csv", dane);

		/*michal ppk*/

		if (!firstMoveModeTime){
			if (previous.getMoveMode() == MoveMode::SEPARATE){
				commands.move.x = difXLeft;
				commands.move.y = difYLeft;
				commands.move.z = difZLeft;

				commands.rotate.x = difXRight;
				commands.rotate.y = difYRight;
				commands.rotate.z = difZRight;
			}else{
				if (abs(difXLeft - difXRight) < thresholdMovedTogeder && abs(difYLeft - difYRight) < thresholdMovedTogeder && abs(difZLeft - difZRight) < thresholdMovedTogeder){
					//moving together
					commands.move.x = (difXLeft + difXRight) / 2;
					commands.move.y = (difYLeft + difYRight) / 2;
					commands.move.z = (difZLeft + difZRight) / 2;
				}else{
					//moving in oposite direction
					commands.rotate.x = (difXLeft + difXRight) / 2;
					commands.rotate.y = (difYLeft + difYRight) / 2;
					commands.rotate.z = (difZLeft + difZRight) / 2;
				}
			}
		}else{
			previous.setStartingPoints(left, right);
		}

		previous.updateHandCommands(left, right, commands);

		return previous;
	}

}