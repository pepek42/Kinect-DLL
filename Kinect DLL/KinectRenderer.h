#pragma once

#include "GL/freeglut.h"
#include "small.h"
namespace KinectDLL
{

	class KinectRenderer
	{
	public: 
		/// depth map and textrure width
		static const int        c_DepthWidth = 640;
		/// depth map and textrure height
		static const int        c_DepthHeight = 480;
		/// textrure bytes per pixel (red green blue alpha) 
		static const int        c_BytesPerPixel = 4;
	protected:

		int textureSize = c_DepthWidth * c_DepthHeight * c_BytesPerPixel;

		/// pointer to texture
		GLubyte* m_pTexture;
		
		/// <summary>
		/// Constructor
		/// </summary>
		KinectRenderer();

		/// <summary>
		/// Destructor
		/// </summary>
		~KinectRenderer();

		/// <summary>
		/// Converts depth of the point to rgba pixel; if player index not equal 0 then it gives adequate player collor to the pixel 
		/// </summary>
		/// <param name="depth">Depth from Kinect sensor in mm</param>
		/// <param name="playerIndex">Pleyer index (0-7); if 0 then pixel does not belongs to any player according to Kinect</param>
		/// <param name="redPixel">Pointer to red value byte</param>
		/// <param name="greenPixel">Pointer to green value byte</param>
		/// <param name="bluePixel">Pointer to blue value byte</param>
		void DepthShortToRgb(USHORT depth, USHORT playerIndex, GLubyte* redPixel, GLubyte* greenPixel, GLubyte* bluePixel);

		/// <summary>
		/// Sets point on texture (as square) 
		/// </summary>
		/// <param name="point">X, Y, Z coordinates of point that we want to set on texture</param>
		/// <param name="r_col">Red value (0-255)</param>
		/// <param name="g_col">Green value (0-255)</param>
		/// <param name="b_col">Blue value (0-255)</param>
		/// <param name="size">Size of the square (2*sizex2*size)</param>
		void setPoint(MyPoint point, GLubyte r_col, GLubyte g_col, GLubyte b_col, short size);

		/// <summary>
		/// Sets basic texture based on depth map
		/// </summary>
		/// <param name="pDepthBuffer">Pointer to depth map</param>
		/// <param name="pbPlayers">Pointer to players table. If pleyer is recognised his index is flaged with true. For example if player 3 is recognized then  pbPlayers[3]=true</param>
		void setBasicTexture(BYTE* pDepthBuffer, bool* pbPlayers);
	};

}