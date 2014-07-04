#include "stdafx.h"
#include "KinectRenderer.h"
#include <NuiApi.h>

namespace KinectDLL
{
	KinectRenderer::KinectRenderer()
	{
		m_pTexture = new GLubyte[c_DepthWidth * c_DepthHeight * c_BytesPerPixel];
	}


	KinectRenderer::~KinectRenderer()
	{
		delete[] m_pTexture;
	}

	void KinectRenderer::setBasicTexture(BYTE* pDepthBuffer, bool* pbPlayers){
		NUI_DEPTH_IMAGE_PIXEL* pBufferRun = reinterpret_cast<NUI_DEPTH_IMAGE_PIXEL*>(pDepthBuffer);

		GLubyte* pDepthB = m_pTexture;
		int bufferSize = c_DepthWidth*c_DepthHeight;

		for (int i = 0; i < bufferSize; ++i){
			USHORT depth = pBufferRun->depth;
			USHORT playerIndex = pBufferRun->playerIndex;
			if (playerIndex == 0){
				GLubyte intensity = depth % 256;
				*pDepthB++ = intensity;
				*pDepthB++ = intensity;
				*pDepthB++ = intensity;
			}else{
				DepthShortToRgb(depth, playerIndex, pDepthB++, pDepthB++, pDepthB++);
				pbPlayers[playerIndex] = true;
			}
			*pDepthB++ = 0xff;

			++pBufferRun;
		}
	}

	void KinectRenderer::DepthShortToRgb(USHORT depth, USHORT playerIndex, GLubyte* redPixel, GLubyte* greenPixel, GLubyte* bluePixel){

		// Convert depth info into an intensity for display
		BYTE b = 255 - static_cast<BYTE>(256 * depth / 0x0fff);

		// Color the output based on the player index
		switch (playerIndex){
		case 0:
			*redPixel = depth % 256;
			*greenPixel = depth % 256;
			*bluePixel = depth % 256;
			break;

		case 1:
			*redPixel = b;
			*greenPixel = 0;
			*bluePixel = 0;
			break;

		case 2:
			*redPixel = 0;
			*greenPixel = b;
			*bluePixel = 0;
			break;

		case 3:
			*redPixel = b / 4;
			*greenPixel = b;
			*bluePixel = b;
			break;

		case 4:
			*redPixel = b;
			*greenPixel = b;
			*bluePixel = b / 4;
			break;

		case 5:
			*redPixel = b;
			*greenPixel = b / 4;
			*bluePixel = b;
			break;

		case 6:
			*redPixel = b / 2;
			*greenPixel = b / 2;
			*bluePixel = b;
			break;

		case 7:
			*redPixel = 255 - (b / 2);
			*greenPixel = 255 - (b / 2);
			*bluePixel = 255 - (b / 2);
			break;

		default:
			*redPixel = 0;
			*greenPixel = 0;
			*bluePixel = 0;
			break;
		}

	}

	void KinectRenderer::setPoint(MyPoint point, GLubyte r_col, GLubyte g_col, GLubyte b_col, short size){

		short abs_rad = abs(size);
		short rad = -abs_rad;

		for (short i = rad; i <= abs_rad; ++i){
			for (short j = rad; j <= abs_rad; ++j){
				int coor = ((point.x + i) + (point.y + j)*c_DepthWidth) * 4;
				if (coor + 3 < textureSize && coor > 0){
					m_pTexture[coor] = r_col;
					m_pTexture[coor + 1] = g_col;
					m_pTexture[coor + 2] = b_col;
					m_pTexture[coor + 3] = 0xff;
				}
			}
		}
	}
}