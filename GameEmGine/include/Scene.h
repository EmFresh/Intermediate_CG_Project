#pragma once
#include <functional>
#include "SkyBox.h"

/*
Tricking the  system to think the
GameEmgine class is created
*/
class Scene
{
public:
	virtual ~Scene() { m_parent = nullptr;}

	////shader initialization
	//virtual void shaderInit() = 0;

	/// <summary>
	/// Called once before the scene is created
	/// </summary>
	virtual void init() = 0;

	/// <summary>
	/// updates within game loop
	/// </summary>
	/// <param name="dt:">time since last frame</param>
	virtual void update(double dt) = 0;

	/// <summary>
	/// Is called whenever a new scene is set or scene has been destroyed 
	/// </summary>
	virtual void onSceneExit() = 0;


	/// <summary>
	/// the post buffer will be blitted at the end of the function call to the main buffer
	/// </summary>
	/// <param name="gbuff:">contains the data for 
	/// position, trans position, normal, trans normal, 
	/// scene colour, and Light Accumulation (in that order)</param>
	/// <param name="postBuff:">this is the buffer that contains the base scene 
	/// and will be blitted to the screen in the end</param>
	std::function<void(FrameBuffer* gbuff, FrameBuffer* postBuff, float dt)>  customPostEffects;

	void setParent(const Scene* parent) { if(parent != this)m_parent = (Scene*)parent; };
	const Scene* getParent() { return m_parent; }
	const SkyBox& getSkyBox() { return m_skybox; }
	bool skyBoxEnabled = false;

	std::function<void(void)>render;

	std::function<void(int state, int button, int mod)>
		keyInput,
		mouseInput;

	//instance key is pressed 
	std::function<void(int key, int mod)>
		keyPressed;

	//instance key is released
	std::function<void(int key, int mod)>
		keyReleased;

	//instance button is pressed 
	std::function<void(int button, int mod)>
		mousePressed;

	//instance button is released
	std::function<void(int button, int mod)>
		mouseReleased;

protected:
	void enableSkyBox(bool enable) { skyBoxEnabled = enable; }

	void setSkyBox(cstring path) { m_skybox.setCubeMap(path); }


	SkyBox m_skybox;
	Scene* m_parent = nullptr;

};

