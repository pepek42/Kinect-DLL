#pragma once

#include <Windows.h>
#include "stdafx.h"

namespace KinectDLL
{

	static const LPCTSTR path = L".\\KinectDLL.ini";;

	class MyPoint
	{
	public:
		USHORT x;
		USHORT y;
		USHORT z;
		MyPoint(){ x = 0; y = 0; z = 0; }
		MyPoint(USHORT ax, USHORT ay, USHORT adepth){ x = ax; y = ay; z = adepth; }
		~MyPoint(){}

		void clear(){ x = 0; y = 0; z = 0; };

		inline bool operator == (const MyPoint &b) const
		{
			return (
				(abs(b.x - x) < xy_threshold) &&
				(abs(b.y - y) < xy_threshold) &&
				(abs(b.z - z) < z_threshold)
				);
		}
	private:
		static const USHORT xy_threshold = 20;
		static const USHORT z_threshold = 20;
	};

	class MyPointFloat
	{
	public:
		float x;
		float y;
		float z;
		MyPointFloat(){ x = 0.0f; y = 0.0f; z = 0.0f; }
		MyPointFloat(float ax, float ay, float adepth){ x = ax; y = ay; z = adepth; }
		~MyPointFloat(){}

		inline bool operator == (const MyPointFloat &b) const
		{
			return (
				b.x == x && b.y == y && b.z == z
				);
		}

		inline bool operator != (const MyPointFloat &b) const
		{
			return (
				b.x != x || b.y != y || b.z != z
				);
		}

		MyPointFloat& operator += (const MyPointFloat &b)
		{
			x += b.x;
			y += b.y;
			z += b.z;
			return *this;
		}

		MyPointFloat& operator / (const USHORT &b)
		{
			x /= b;
			y /= b;
			z /= b;
			return *this;
		}

		void clear(){ x = 0.0f; y = 0.0f; z = 0.0f; };
	};

	class Commands
	{
	public:
		Commands(){};
		~Commands(){};

		MyPointFloat move;

		MyPointFloat rotate;
	};

	enum States{
		QUIT,
		MOVE,
		IDLE
	};

	enum MoveAccuracy{
		NORMAL,
		ACCURATE
	};

	enum MoveMode{
		SEPARATE,
		TOGETHER
	};
}