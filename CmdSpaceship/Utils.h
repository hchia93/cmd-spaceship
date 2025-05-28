#pragma once
#include <string>

struct FLocation2D
{
    int X;
    int Y;

    FLocation2D() { X = 0; Y = 0; }
    FLocation2D(const int iX, const int iY) : X(iX), Y(iY) {}
    bool operator==(const FLocation2D& P) const { return X == P.X && Y == P.Y; }
    FLocation2D& operator+=(const FLocation2D& P) { X += P.X; Y += P.Y; return *this; }
    FLocation2D& operator-=(const FLocation2D& P) { X -= P.X; Y -= P.Y; return *this; }
    FLocation2D operator+(const FLocation2D& P) const { return FLocation2D(X + P.X, Y + P.Y); }
    std::string ToString() const;
    void FromString(const std::string& context);
    static bool IsMatch(int x, int y, const FLocation2D& Location) { return Location.X == x && Location.Y == y; };
};

enum ENetRole
{
    LOCAL,
    REMOTE,
};

enum EDirection
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};
