#pragma once

#define RESULT_SUCCEED 0 
#define RESULT_ERROR 1
#define RESULT_NOT_SUPPORTED 2

struct FLocation2D
{
	int X;
	int Y;

	FLocation2D() { X = 0; Y = 0; }
	FLocation2D(const int iX, const int iY) : X(iX), Y(iY) { }
	bool operator==(const FLocation2D P) { return X == P.X && Y == P.Y; }
	FLocation2D& operator+=(const FLocation2D P) { X += P.X; Y += P.Y; return *this; }
	FLocation2D& operator-=(const FLocation2D P) { X -= P.X; Y -= P.Y; return *this; }
};

enum EDirection
{
	EDR_Unknown,
	EDR_Up,
	EDR_Right,
	EDR_Down,
	EDR_Left,
	EDR_MAX
};

enum EGameEvent
{
	EPE_Unknown,
	EPE_Initialise,
	EPE_Activate,
	EPE_Sleep,
};
