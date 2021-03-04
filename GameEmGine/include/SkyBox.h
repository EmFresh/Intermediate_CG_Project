#pragma once
#include <string>
#include "mesh.h"
#include "ResourceManager.h"
#include "Camera.h"
class SkyBox
{
public:
	SkyBox();
	SkyBox(std::string file);
	~SkyBox() ;

	void render();

	void setCubeMap(cstring path);
	void setCamera(Camera* cam);

private:
	void init();

	PrimitiveCube* m_box;
	Texture3D* m_sky;
	
	Camera* m_camera;
	GLuint m_vaoID, m_vboID, m_eleID;

	static SkyBox* m_instance;
};

