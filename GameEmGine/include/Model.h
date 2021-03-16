#pragma once
#include <GL\glew.h>
#include <GLFW/glfw3.h>	  
#include <vector>
#include <string>
#include <unordered_map>
#include "Camera.h"
#include "Shader.h"
#include "Utilities.h"
#include "Transformer.h"
#include "Physics3D.h"
#include "MeshLoader.h"
#include "Animation.h"
//#include "FrameBuffer.h"

class Camera;
class Animation;

class Model: public Transformer
{
public:
	Model():Transformer("MODEL") {};
	Model(Model& model, cstring tag = "");
	Model(PrimitiveMesh* model, cstring tag = "");
	Model(cstring path, cstring tag = "");
	virtual ~Model();


	void create(Model& model, cstring tag = "");
	void create(PrimitiveMesh* mesh, cstring tag = "");
	void create(cstring path, cstring tag = "");

	bool collision2D(Model* k, Coord3D<float> ignore);

	bool collision3D(Model* k);

	static bool collision2D(Model* l, Model* k, Coord3D<float> ignore);

	static bool collision3D(Model* l, Model* k);

	static bool getSeparatingPlane(const Vec3& RPos, const Vec3& Plane, Model& box1, Model& box2);


	virtual void render(Shader& shader, Camera* cam);


	//sets RGBA colour with float values ranging from 0 -> 1
	void setColour(float r, float g, float b, float a);

	//sets RGB colour with float values ranging from 0 -> 1
	void setColour(float r, float g, float b);

	//sets RGBA colour with values ranging from 0 -> 255
	void setColour(ColourRGBA colour);

	ColourRGBA getColour();


	bool loadModel(cstring path);

	void enableBoundingBox(bool enable);

	void addAnimation(std::string tag, Animation* animation);

	void editVerts(Model* first, Model* second);


	float getWidth();
	float getHeight();
	float getDepth();
	Vec3 getSize();

	Vec3 getCenter();


	Animation* getAnimation(cstring tag);
	Animation* getCurrentAnimation();
	void  setAnimation(cstring tag);

	void addMesh(Mesh*);
	Mesh* getMesh(const unsigned index);
	Shader* getShader();

	void replaceTexture(int mesh, int index, GLuint tex);

	void enableTexture(bool enable);
	bool isTextureEnabled();
	void setToRender(bool render);
	void setTransparent(bool trans);
	bool isTransparent();
	void setCastShadow(bool cast);
	bool isCastingShadow();
	void setWireframe(bool wire);
	void print();
	std::vector<Vec3> getBounds();
protected:
	ColourRGBA m_colour;
	void meshCleanUp();

private:
	void boundingBoxInit();
	void boundingBoxUpdate();
	void drawBoundingBox();

	bool m_useTex = true;
	bool m_render = true;
	bool m_transparent = false;
	bool m_wireframe = false;
	bool m_shadowCast = true;
	cstring m_tag;

	std::unordered_map< std::string, Animation*>m_animations;
	std::string m_animation;
	std::vector<Mesh*> m_meshes;

	Camera* m_camera;

	GLuint m_BBVaoID = 0, m_BBVboID = 0;
	//Transformer m_transform;

	Vec3
		m_topLeftBack,
		m_topRightBack,
		m_topLeftFront,
		m_topRightFront,
		m_bottomLeftBack,
		m_bottomRightBack,
		m_bottomLeftFront,
		m_bottomRightFront,
		m_center;

	float m_width, m_height, m_depth;
	Shader* m_shader, * m_shaderBB;
	Vertex3D m_vertBBDat[12 * 3];

	bool m_enableBB = false, m_copy = false;

	//std::unordered_map<std::string, FrameBuffer*> m_frameBuffers;
	//std::vector<Model*> m_children;
	//Model* m_parent;
	//std::vector <Texture2D> loadedTextures;
	//std::string dir;



};
