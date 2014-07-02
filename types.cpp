#include "stdafx.h"

using namespace System;

class Vector3Double
{
	double x,y,z;
	
public:
	Vector3Double();
};

Vector3Double::Vector3Double()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}


class PositionXYZKey
{

	double time;

	Vector3Double* value;

	Vector3Double* invector;

	Vector3Double* outVector;

public:
	PositionXYZKey();
};

PositionXYZKey::PositionXYZKey()
{
	time = 0.0;

	value = new Vector3Double();

	invector = new Vector3Double();

	outVector = new Vector3Double();
}
