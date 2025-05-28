#include "Utils.h"
#include <sstream>

// https://www.systutorials.com/131/convert-string-to-int-and-reverse/

std::string FLocation2D::ToString() const
{
    std::stringstream ss;
    ss << X << "," << Y;
    return ss.str();
}

void FLocation2D::FromString(const std::string& context)
{
    std::stringstream ss(context);
    char comma;
    ss >> X >> comma >> Y;
}
