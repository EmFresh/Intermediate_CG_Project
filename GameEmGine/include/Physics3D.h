#pragma once
#include "Transformer.h"
#include "Utilities.h"
class Physics3D
{
public:
	Physics3D();
	Physics3D(Transformer*);
	~Physics3D();

	void setTransformer(Transformer*);

	void setGravityMagnitude(float);

	void setGravityDirection(Vec3);

	void update();

private:
	Transformer *transform;
	Vec3 direction;
	float magnitude;
};

