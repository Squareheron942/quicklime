#pragma once

#include <3ds.h>
#include <citro3d.h>

class Light {
  protected:
	C3D_FVec _tp; // transformed position
	static LightLock lock;

  public:
	C3D_LightLut _lut;
	Light(C3D_FVec color);
	~Light();
	C3D_Light *light;
	C3D_FVec position, color;
	virtual void update(C3D_Mtx *view);
	unsigned char index;
};