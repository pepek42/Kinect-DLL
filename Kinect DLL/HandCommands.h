#pragma once
#include "small.h"
#include "KinectDLL.h"

namespace KinectDLL{
	class HandCommands
	{
	public:
		/// <summary>
		/// Constructor
		/// </summary>
		KINECTDLL_API HandCommands(){ valid = false; moveMode = MoveMode::SEPARATE; };
		//KINECTDLL_API ~HandCommands();

		/// <summary>
		/// Creates depth reader and begins processing  
		/// </summary>
		/// <param name="currentLeft_">New left hand</param> 
		/// <param name="currentRigth_">New right hand</param>
		/// <param name="commands_">New commands to set</param>
		KINECTDLL_API void  updateHandCommands(MyPointFloat currentLeft_, MyPointFloat currentRigth_, Commands commands_){

			previousLeft = currentLeft;
			previousRight = currentRigth;

			currentLeft = currentLeft_;
			currentRigth = currentRigth_;

			commands = commands_;

			valid = (currentLeft.z != 0.0f && currentRigth.z != 0.0f);
			used = false;
		};

		/// <summary>
		/// Returns if hand commands are valid
		/// </summary>
		/// <returns>True if hand commands are valid</returns>
		KINECTDLL_API bool isValid(){
			return valid;
		}

		/// <summary>
		/// Sets starting point as reference
		/// </summary>
		/// <param name="startingLeft_">Left hand</param>
		/// <param name="startingRight_">Right hand</param>
		KINECTDLL_API void setStartingPoints(MyPointFloat startingLeft_, MyPointFloat startingRight_){
			startingLeft = startingLeft_;
			startingRight = startingRight_;
		}

		/// <summary>
		/// Sets hand movement interpretation mode
		/// </summary>
		/// <param name="moveMode_">Hands move modes</param>
		KINECTDLL_API void setMoveMode(MoveMode moveMode_){ moveMode = moveMode_; };

		/// <summary>
		/// Sets hand movement interpretation mode
		/// </summary>
		/// <returns>Hands move modes</returns>
		KINECTDLL_API MoveMode getMoveMode(){ return moveMode; };

		MyPointFloat startingLeft;
		MyPointFloat startingRight;

		MyPointFloat previousLeft;
		MyPointFloat previousRight;

		MyPointFloat currentLeft;
		MyPointFloat currentRigth;

		/// current hand commands
		Commands commands;

		///used flag, prevents same hand commands from beeing used second time...
		bool used;

	private:
		bool valid;

		MoveMode moveMode;
	};
}