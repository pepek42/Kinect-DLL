#pragma once
#include "small.h"
#include "HandCommands.h"

namespace KinectDLL{
	class HandCommandsRecognizer
	{
	protected:

		/// <summary>
		/// Constructor
		/// </summary>
		HandCommandsRecognizer();

		/// <summary>
		/// Destructor
		/// </summary>
		~HandCommandsRecognizer();

		/// <summary>
		/// Interprets current and previous hand postion returning commands
		/// </summary>
		/// <param name="previous">previous hand commands</param>
		/// <param name="handLeft">current left hand postion</param>
		/// <param name="handRight">current right hand postion</param>
		/// <param name="firstMoveModeTime">was previous state idle? the we can not interpret difference between hand postions.</param>
		/// <returns>New commands</returns>
		HandCommands getCommancds(HandCommands previous, MyPoint handLeft, MyPoint handRight, bool firstMoveModeTime);
	private:
		/// Pointer to array of previous left hand positions
		MyPointFloat* m_pAvrgLeft;

		/// Pointer to array of previous right hand positions
		MyPointFloat* m_pAvrgRight;

		/// while move mode is move together this is threshold determining wheather hands are moved the in the same direction 
		float thresholdMovedTogeder;

		/// Number of previous hands positions taken into account while getting average position. In other way it is size of m_pAvrgLeft and m_pAvrgRight arrays. Need to be 1 or more
		USHORT m_avrgCount;

	};
}