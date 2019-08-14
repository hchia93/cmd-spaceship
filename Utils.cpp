#include "Utils.h"
#include <sstream>

const char* FLocation2D::ToString()
{
	std::stringstream ssx;
	ssx << X; 
	std::string xcoord = ssx.str();
	
	std::stringstream ssy;
	ssy << Y;
	std::string ycoord = ssy.str();
	std::string coord = xcoord + "," + ycoord;

	strcpy_s(LocationString, 6, coord.c_str());

	return LocationString;
}

void FLocation2D::FromString(const char* Context)
{
	//std::istringstream is()
}
