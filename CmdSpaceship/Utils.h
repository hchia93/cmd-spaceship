#pragma once

#define SCREEN_Y_MAX 20
#define SCREEN_X_MAX 41 
#define CONSOLE_MAX_MESSAGE_LINES 10
#define CONSOLE_SCREEN_BUFFER_X 50
#define CONSOLE_SCREEN_BUFFER_Y (SCREEN_Y_MAX) + (CONSOLE_MAX_MESSAGE_LINES)

#define CLAMP_RANGE(x, Min, Max)  x = ((x) < Min) ?  Min : ((x) > Max) ? Max : (x);
#define CLAMP(Location) CLAMP_RANGE(Location.X, 2 ,SCREEN_X_MAX-3) CLAMP_RANGE(Location.Y, 1, SCREEN_Y_MAX-2)

struct FHitResult
{
    bool bHitRemotePlayer = false;
    bool bHitBullet = false;
};

struct FLocation2D
{
    int X;
    int Y;

    FLocation2D() { X = 0; Y = 0; }
    FLocation2D(const int iX, const int iY) : X(iX), Y(iY) {}
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
    Unknown,
    Up,
    Right,
    Down,
    Left
};
