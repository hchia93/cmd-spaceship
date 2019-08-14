#pragma once


#define SCREEN_Y_MAX 20
#define SCREEN_X_MAX 41 
#define CLAMP_RANGE(x, Min, Max)  x = ((x) < Min) ?  Min : ((x) > Max) ? Max : (x);
#define CLAMP(Location) CLAMP_RANGE(Location.X, 2 ,SCREEN_X_MAX-3) CLAMP_RANGE(Location.Y, 1, SCREEN_Y_MAX-2)

struct FLocation2D
{
	int X;
	int Y;

	FLocation2D() { X = 0; Y = 0; }
	FLocation2D(const int iX, const int iY) : X(iX), Y(iY) { }
	bool operator==(const FLocation2D P) { return X == P.X && Y == P.Y; }
	FLocation2D& operator+=(const FLocation2D P) { X += P.X; Y += P.Y; return *this; }
	FLocation2D& operator-=(const FLocation2D P) { X -= P.X; Y -= P.Y; return *this; }
	FLocation2D operator+(FLocation2D P) { return FLocation2D(X + P.X, Y + P.Y); }
	const char* ToString();
	void FromString(const char* Context);
	static bool IsMatch(int x, int y, const FLocation2D& Location) { return Location.X == x && Location.Y == y; };

private:

	char LocationString[6];
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
