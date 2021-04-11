//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include <GameEmGine.h>
#include <memory>
#include "GameObjects.h"
#include "Song.h"
#include "Menu.h"
#include "BeatMapReader.h"

using std::vector;

static std::string lutPath = "textures/hot.cube";

class Test: public Scene
{
	enum Switches
	{
		DefaultScene = 0,
		Position,
		Normal,
		colour,
		lightAccumulation,
		post1,
		post2,
		post3
	};

#pragma region Variables

	float speed = 20, angle = 1, bloomThresh = 0.1f;
	Animation ani;

	Switches toggle = post1;

	Model models[10];
	Transformer trans[10];
	Model bigBoss[2];
	Model rocket;
	vector<Light> lights;

	Text testText;
	Light lit;
	bool moveLeft, moveRight, moveForward, moveBack, moveUp, moveDown,
		rotLeft, rotRight, rotUp, rotDown, tiltLeft, tiltRight,
		tab = false, lutActive = false, enableBloom = false, pause = false;
	Shader
		* m_lutNGrayscaleShader, * m_bloomHighPass,
		* m_blurHorizontal, * m_blurVertical,
		* m_blurrComposite, * m_sobel;
	FrameBuffer
		* m_buffer1, * m_buffer2,
		* m_greyscaleBuffer, * m_outline;
#pragma endregion

public:
	int blurPasses = 2;

	void init()
	{

	#pragma region Init Setup

		Game::setBackgroundColour(.15f, .15f, .15f);
		Game::translateCamera({0,0,-3});
		FrustumPeramiters frustum{65,(float)Game::getWindowWidth() / Game::getWindowHeight(),0.001f,500};

		Game::setCameraType(&frustum);
		Game::getMainCamera()->enableFPSMode();

		setSkyBox("Skyboxes/space/");
		enableSkyBox(true);
	#pragma endregion



		//Create post effects
		customPostEffects =
			[&](FrameBuffer* gbuff, FrameBuffer* postBuff, float dt)->void
		{
			m_buffer1->clear();
			m_buffer2->clear();

			static float timer = 0;
			Shader* filmGrain = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "shaders/filmgrain.fmsh");
			Shader* pixel = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "shaders/pixelation.fmsh");

			switch(toggle)
			{
			case Switches::DefaultScene:
				break;
			case Switches::Position:
				gbuff->copySingleColourToBuffer(postBuff->getColourWidth(0), postBuff->getColourHeight(0), postBuff, 0);
				break;
			case Switches::Normal:
				gbuff->copySingleColourToBuffer(postBuff->getColourWidth(0), postBuff->getColourHeight(0), postBuff, 2);
				break;
			case Switches::colour:
				gbuff->copySingleColourToBuffer(postBuff->getColourWidth(0), postBuff->getColourHeight(0), postBuff, 4);
				break;
			case Switches::lightAccumulation:
				gbuff->copySingleColourToBuffer(postBuff->getColourWidth(0), postBuff->getColourHeight(0), postBuff, 5);
				break;
			case post1:
			#pragma region Film Grain

				postBuff->enable();
				filmGrain->enable();

				filmGrain->sendUniform("ucolorMap", 0);
				filmGrain->sendUniform("utime", timer += dt);
				postBuff->getColorTexture(0).bindTexture(0);

				FrameBuffer::drawFullScreenQuad();

				filmGrain->disable();
				postBuff->disable();
			#pragma endregion
				break;
			case post2:
			#pragma region Bloom
				glViewport(0, 0, Game::getWindowWidth() / 2, Game::getWindowHeight() / 2);

				//binds the initial high pass to buffer 1
				m_buffer1->enable();
				m_bloomHighPass->enable();

				postBuff->getColorTexture(0).bindTexture(0);

				m_bloomHighPass->sendUniform("uTex", 0);
				m_bloomHighPass->sendUniform("uThresh", bloomThresh);

				FrameBuffer::drawFullScreenQuad();

				Texture2D::unbindTexture(0);

				m_bloomHighPass->disable();
				m_buffer1->disable();

				//Takes the high pass and blurs it
				//glViewport(0, 0, Game::getWindowWidth() / 2, Game::getWindowHeight() / 2);
				for(int a = 0; a < blurPasses; a++)
				{
					m_buffer2->enable();
					m_blurHorizontal->enable();
					m_blurHorizontal->sendUniform("uTex", 0);
					m_blurHorizontal->sendUniform("uPixleSize", 1.0f / Game::getWindowHeight());
					glBindTexture(GL_TEXTURE_2D, m_buffer1->getColorHandle(0));
					FrameBuffer::drawFullScreenQuad();

					glBindTexture(GL_TEXTURE_2D, GL_NONE);
					m_blurHorizontal->disable();


					m_buffer1->enable();
					m_blurVertical->enable();
					m_blurVertical->sendUniform("uTex", 0);
					m_blurVertical->sendUniform("uPixleSize", 1.0f / Game::getWindowWidth());
					glBindTexture(GL_TEXTURE_2D, m_buffer2->getColorHandle(0));
					FrameBuffer::drawFullScreenQuad();

					glBindTexture(GL_TEXTURE_2D, GL_NONE);
					m_blurVertical->disable();
				}

				FrameBuffer::disable();//return to base frame buffer

				glViewport(0, 0, Game::getWindowWidth(), Game::getWindowHeight());

				m_greyscaleBuffer->enable();
				m_blurrComposite->enable();
				glActiveTexture(GL_TEXTURE0);
				m_blurrComposite->sendUniform("uScene", 0);
				glBindTexture(GL_TEXTURE_2D, postBuff->getColorHandle(0));

				glActiveTexture(GL_TEXTURE1);
				m_blurrComposite->sendUniform("uBloom", 1);
				glBindTexture(GL_TEXTURE_2D, m_buffer1->getColorHandle(0));

				m_blurrComposite->sendUniform("uBloomEnable", enableBloom);
				FrameBuffer::drawFullScreenQuad();

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, GL_NONE);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, GL_NONE);
				m_blurrComposite->disable();
				m_greyscaleBuffer->disable();
				m_greyscaleBuffer->copyColourToBuffer(postBuff->getDepthWidth(), postBuff->getDepthWidth(), postBuff);
			#pragma endregion
				break;
			case post3:
			#pragma region Pixelation

				postBuff->enable();
				pixel->enable();

				pixel->sendUniform("uTex", 0);
				postBuff->getColorTexture(0).bindTexture(0);

				FrameBuffer::drawFullScreenQuad();

				pixel->disable();
				postBuff->disable();
			#pragma endregion
				break;
			}

		};


	#pragma region Scene Setup

		//models[0].create("models/solar system/Sun/Sun.obj", "Sun");
		//models[1].create("models/solar system/Mercury/Mercury.obj", "Mercury");
		//models[2].create("models/solar system/Venus/Venus.obj", "Venus");
		//models[3].create("models/solar system/Earth/Earth.obj", "Earth");
		//models[4].create("models/solar system/Mars/Mars.obj", "Mars");
		//models[5].create("models/solar system/Jupiter/Jupiter.obj", "Jupiter");
		//models[6].create("models/solar system/Saturn/Saturn.obj", "Saturn");
		//models[7].create("models/solar system/Uranus/Uranus.obj", "Uranus");
		//models[8].create("models/solar system/Neptune/Neptune.obj", "Neptune");
		//
		//for(int a = 0; a < 9; ++a)
		//{
		//	if(a)
		//		models[a].setParent(&trans[a]);
		//
		//	Game::addModel(&models[a]);
		//	models[a].setScale(.1f);
		//	models[a].translateBy({25.f * a,0,0});
		//}
		//models[1].scaleBy(.2f);
		//models[2].scaleBy(.2f);
		//models[3].scaleBy(.2f);
		//models[4].scaleBy(.2f);
		//models[5].scaleBy(.8f);
		//models[6].scaleBy(.7f);
		//models[7].scaleBy(.5f);
		//models[8].scaleBy(.4f);
		//
		////(161.874771, 74.611961, 82.858345)
		//Game::setCameraPosition({161.874771f, 74.611961f, -82.858345f});
		//Game::getMainCamera()->enableFPSMode();


		rocket.create("Models/rocket-ship/rocket ship.obj", "ship");
		Game::addModel(&rocket);

		models[1].create(new PrimitivePlane(Vec3{50, 0, 50}*5), "moon");
		models[1].replaceTexture(0, 0, ResourceManager::getTexture2D("Textures/moon.jpg").id);
		Game::addModel(&models[1]);

		models[2].create(new PrimitiveCube({10,10,10}/*, 10, 10*/), "trans Box");
		models[2].setColour(0, .5f, 0, .75f);
		models[2].translate(5, 10, 3);
		models[2].rotate(40, 106, 33);

		//models[2].setTransparent(true);
		Game::addModel(&models[2]);

		lit.setLightType(Light::TYPE::DIRECTIONAL);
		//lit.setParent(Game::getMainCamera());
		lit.setDiffuse({155,0,0});
		LightManager::addLight(&lit);

		//static Light tester;
		//tester.setLightType(Light::TYPE::DIRECTIONAL);
		//tester.rotate({45,-90,0});
		//LightManager::addLight(&tester);
	#pragma endregion

	#pragma region Light Setup

		for(auto& light : lights)
		{
			light.setLightType(Light::POINT);

			light.setDiffuse(ColourRGBA(rand() % 256, rand() % 256, rand() % 256));
			light.translate(rand() % 20 + rand() % 1000 * .001,
							rand() % 5 + rand() % 1000 * .001,
							rand() % 20 + rand() % 1000 * .001);

		
		}

	#pragma endregion


		//Key binds
		keyPressed =
			[&](int key, int mod)->void
		{

			if(key == 'R')
			{
				Game::getMainCamera()->reset();
				Game::translateCamera({0,0,-3});
			}
			static bool sky = true, frame = false;

			if(key == 'N')
				rocket.setWireframe(frame = !frame);

			if(key == GLFW_KEY_SPACE)
				pause = !pause;
			//enableSkyBox(sky = !sky);

			if(key == GLFW_KEY_F5)
				Shader::refresh();

			//static int count;
			if(key == GLFW_KEY_TAB)
				tab = !tab;//	std::swap(model[0], model[count++]);

			for(int a = 0; a < 8; ++a)
				if(key == GLFW_KEY_1 + a)
					toggle = (Switches)a;

			static bool fps = 0;
			if(key == 'F')
				rocket.enableFPSMode(fps = !fps);

			if(key == 'A')
				moveLeft = true;

			if(key == 'D')
				moveRight = true;

			if(key == 'W')
				moveForward = true;

			if(key == 'S')
				moveBack = true;

			if(key == 'Q')
				moveDown = true;

			if(key == 'E')
				moveUp = true;


			if(key == GLFW_KEY_PAGE_UP)
				tiltLeft = true;

			if(key == GLFW_KEY_PAGE_DOWN)
				tiltRight = true;

			if(key == GLFW_KEY_LEFT)
				rotLeft = true;

			if(key == GLFW_KEY_RIGHT)
				rotRight = true;

			if(key == GLFW_KEY_UP)
				rotUp = true;

			if(key == GLFW_KEY_DOWN)
				rotDown = true;
		};

		keyReleased =
			[&](int key, int mod)->void
		{
			if(key == 'A')
				moveLeft = false;

			if(key == 'D')
				moveRight = false;

			if(key == 'W')
				moveForward = false;

			if(key == 'S')
				moveBack = false;

			if(key == 'Q')
				moveDown = false;

			if(key == 'E')
				moveUp = false;


			if(key == GLFW_KEY_PAGE_UP)
				tiltLeft = false;

			if(key == GLFW_KEY_PAGE_DOWN)
				tiltRight = false;

			if(key == GLFW_KEY_LEFT)
				rotLeft = false;

			if(key == GLFW_KEY_RIGHT)
				rotRight = false;

			if(key == GLFW_KEY_UP)
				rotUp = false;

			if(key == GLFW_KEY_DOWN)
				rotDown = false;

			//	puts(Game::getMainCamera()->getLocalPosition().toString());
		};

		//EmGineAudioPlayer::createAudioStream("songs/still alive.mp3");
		//EmGineAudioPlayer::getAudioControl()[0][0]->channel->set3DMinMaxDistance(20, 200);
		//
		//EmGineAudioPlayer::play(true);
	}

	void cameraMovement(float dt)
	{
		// Movement
		if(moveLeft)
			Game::translateCameraBy({-speed * dt,0,0});
		if(moveRight)
			Game::translateCameraBy({speed * dt,0,0});
		if(moveForward)
			Game::translateCameraBy({0,0,speed * dt});
		if(moveBack)
			Game::translateCameraBy({0,0,-speed * dt});
		if(moveUp)
			Game::translateCameraBy({0,speed * dt,0});
		if(moveDown)
			Game::translateCameraBy({0,-speed * dt,0});

		// Rotation
		if(tiltLeft)
			Game::rotateCameraBy({0,0,-angle});
		if(tiltRight)
			Game::rotateCameraBy({0,0,angle});
		if(rotLeft)
			Game::rotateCameraBy({0,-angle,0});
		if(rotRight)
			Game::rotateCameraBy({0,angle,0});
		if(rotUp)
			Game::rotateCameraBy({angle,0,0});
		if(rotDown)
			Game::rotateCameraBy({-angle,0,0});
	}

	void lightMovement(float dt)
	{
		// Movement
		if(moveLeft)
			lit.translateBy({-speed * dt,0.f,0.f});
		if(moveRight)
			lit.translateBy({speed * dt,0,0});
		if(moveForward)
			lit.translateBy({0,0,speed * dt});
		if(moveBack)
			lit.translateBy({0,0,-speed * dt});
		if(moveUp)
			lit.translateBy({0,speed * dt,0});
		if(moveDown)
			lit.translateBy({0,-speed * dt,0});

		// Rotation
		if(rotLeft)
			lit.rotateBy({0,-angle,0});
		if(rotRight)
			lit.rotateBy({0,angle,0});
		if(tiltLeft)
			lit.rotateBy({0,0,-angle});
		if(tiltRight)
			lit.rotateBy({0,0,angle});
		if(rotDown)
			lit.rotateBy({-angle,0,0});
		if(rotUp)
			lit.rotateBy({angle,0,0});


	}

	void update(double dt)
	{
		if(!tab)
			cameraMovement((float)dt);
		else
			lightMovement((float)dt);

		float maxSpeed = 10;

		//if(!pause)
		//{
		//
		//	trans[1].rotateBy({0,maxSpeed * 1.0f * (float)dt,0});
		//	trans[2].rotateBy({0,maxSpeed * 0.9f * (float)dt,0});
		//	trans[3].rotateBy({0,maxSpeed * 0.8f * (float)dt,0});
		//	trans[4].rotateBy({0,maxSpeed * 0.7f * (float)dt,0});
		//	trans[5].rotateBy({0,maxSpeed * 0.6f * (float)dt,0});
		//	trans[6].rotateBy({0,maxSpeed * 0.5f * (float)dt,0});
		//	trans[7].rotateBy({0,maxSpeed * 0.4f * (float)dt,0});
		//	trans[8].rotateBy({0,maxSpeed * 0.3f * (float)dt,0});
		//	for(int a = 0; a < 9; ++a)
		//		models[a].rotateBy(0, (10 - a) * 5 * dt, 0);
		//}

		//auto tmpOBJPos = models[0].getLocalPosition();
		//EmGineAudioPlayer::getAudioControl()[0][0]->listener->pos = *(FMOD_VEC3*)&tmpOBJPos;
		//
		//auto tmpPos = Game::getMainCamera()->getLocalPosition();
		//auto tmpUp = Game::getMainCamera()->getUp();
		//auto tmpForward = Game::getMainCamera()->getForward();
		//EmGineAudioPlayer::getAudioSystem()->set3DListenerAttributes(0, (FMOD_VEC3*)&tmpPos, nullptr, (FMOD_VEC3*)&tmpForward, (FMOD_VEC3*)&tmpUp);
		//
		//EmGineAudioPlayer::update();


	}

	void onSceneExit() {}
};

#pragma region GDW GAME STUFF

//Game Design Doc
/*
 PROJECT RHYTHM TOWER

	GamePlay:
		- enemies are spawned based on bpm
		- tower shots are based on specific music rhythm beat
		- there will be a "DOUBLE TIME" section where enemies spawn by half
		bpm and move twice as fast. the towers will not get a speed boost
		- if enemies make it through they do damage based on their beat type on every one of their beats
		- would be cool to add screen shake

	Object Types:
		Enemies:
			- normal: enemies show up on quarter beats 1 2 3 4
			- fast: enemies show up on beats between normal enemies. they are twice as fast but have half health
			- faster (maybe?): that show up on beats 2 and 3 of a quarter
			- tank: enemies that show up every 2(maybe 4) full bars twice or 4 times the health of normal enemies
		Towers:
			- normal: shoots on every quarter beat
			- fast: shoots on the eighth beats per bar
			- Slow: shoots once per 4 beats dose big damage
	Music:
		- simple, repeatable music that can be put on loop
		- should only have one instrument
		- towers add layers to music
*/

class GDWGAME: public Scene
{
	enum Switches
	{
		DefaultScene = 0,
		Position,
		Normal,
		colour,
		lightAccumulation,
		post1,
		post2,
		post3
	};

#pragma region Variables
	Light lit;

	float speed = 20, angle = 1, bloomThresh = 0.1f;

	bool moveLeft, moveRight, moveForward, moveBack, moveUp, moveDown,
		rotLeft, rotRight, rotUp, rotDown, tiltLeft, tiltRight,
		tab = false, lutActive = false, enableBloom = false, pause = false;

	Switches toggle = post1;
	uint blurPasses = 3;

	Model _map;
	std::vector<std::shared_ptr<Enemy>> enemies;
	std::vector<std::shared_ptr<Tower>> towers;

	std::vector<std::shared_ptr< WayPoint>> points;

	std::vector<Beat>beats;

	Shader
		* m_bloomHighPass,
		* m_blurHorizontal,
		* m_blurVertical,
		* m_blurrComposite,
		* m_lutNGrayscaleShader,
		* m_sobel
		;
#pragma endregion

public:
	void cameraMovement(float dt)
	{
		//// Movement
		if(moveLeft)
			Game::translateCameraBy({-speed * dt,0,0});
		if(moveRight)
			Game::translateCameraBy({speed * dt,0,0});
		if(moveForward)
			Game::translateCameraBy({0,0,speed * dt});
		if(moveBack)
			Game::translateCameraBy({0,0,-speed * dt});
		if(moveUp)
			Game::translateCameraBy({0,speed * dt,0});
		if(moveDown)
			Game::translateCameraBy({0,-speed * dt,0});

		// Rotation
		if(tiltLeft)
			Game::rotateCameraBy({0,0,-angle});
		if(tiltRight)
			Game::rotateCameraBy({0,0,angle});
		if(rotLeft)
			Game::rotateCameraBy({0,-angle,0});
		if(rotRight)
			Game::rotateCameraBy({0,angle,0});
		if(rotUp)
			Game::rotateCameraBy({angle,0,0});
		if(rotDown)
			Game::rotateCameraBy({-angle,0,0});
	}

	~GDWGAME()
	{

	}

#include <ctime>
	void init()
	{
	#pragma region Init Shaders & Framebuffers 

		m_bloomHighPass = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/BloomHighPass.fmsh");
		m_blurHorizontal = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/BlurHorizontal.fmsh");
		m_blurVertical = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/BlurVertical.fmsh");
		m_blurrComposite = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/BloomComposite.fmsh");

		m_lutNGrayscaleShader = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/GrayscalePost.fmsh");
		m_sobel = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "shaders/Sobel.fmsh");
		static Shader& m_toonPost = *ResourceManager::getShader("Shaders/Main Buffer.vtsh", "shaders/PosterizeToon.fmsh");

		static FrameBuffer* m_greyscaleBuffer = new FrameBuffer(1, "Greyscale");
		static FrameBuffer* m_buffer1 = new FrameBuffer(1, "Test1");
		static FrameBuffer* m_buffer2 = new FrameBuffer(1, "Test2");
		static FrameBuffer* m_outline = new FrameBuffer(1, "Sobel Outline");
		static FrameBuffer* m_screenshake = new FrameBuffer(1, "Screen Shake");


		m_greyscaleBuffer->initColourTexture(0, Game::getWindowWidth(), Game::getWindowHeight(), GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		if(!m_greyscaleBuffer->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}

		m_buffer1->initColourTexture(0, Game::getWindowWidth() / 2, Game::getWindowHeight() / 2, GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		if(!m_buffer1->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}
		m_buffer2->initColourTexture(0, Game::getWindowWidth() / 2, Game::getWindowHeight() / 2, GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);

		if(!m_buffer2->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}
		m_outline->initColourTexture(0, Game::getWindowWidth(), Game::getWindowHeight(), GL_RGB8, GL_NEAREST, GL_CLAMP_TO_EDGE);
		if(!m_outline->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}
		m_screenshake->initColourTexture(0, Game::getWindowWidth(), Game::getWindowHeight(), GL_RGB8, GL_NEAREST, GL_CLAMP_TO_EDGE);
		if(!m_screenshake->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}


	#pragma endregion


		//Create post effects
		customPostEffects =
			[&](FrameBuffer* gbuff, FrameBuffer* postBuff, float dt)->void
		{
			m_greyscaleBuffer->clear();
			m_buffer1->clear();
			m_buffer2->clear();
			m_screenshake->clear();

			static float timer = 0;
			static Shader* filmGrain = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "shaders/filmgrain.fmsh");

			switch(toggle)
			{
			case post1:
			#pragma region Post 1
				//glViewport(0, 0, Game::getWindowWidth(), Game::getWindowHeight());

				//Film Grain
				postBuff->enable();
				filmGrain->enable();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, postBuff->getColorHandle(0));

				filmGrain->sendUniform("colorMap", 0);
				filmGrain->sendUniform("time", timer += dt);

				FrameBuffer::drawFullScreenQuad();

				filmGrain->disable();
				postBuff->disable();
			#pragma endregion
				break;
			case post2:
			#pragma region Bloom
				glViewport(0, 0, Game::getWindowWidth() / 2, Game::getWindowHeight() / 2);

				//binds the initial high pass to buffer 1
				m_buffer1->enable();
				m_bloomHighPass->enable();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, postBuff->getColorHandle(0));

				m_bloomHighPass->sendUniform("uTex", 0);
				m_bloomHighPass->sendUniform("uThresh", bloomThresh);

				FrameBuffer::drawFullScreenQuad();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, GL_NONE);

				m_bloomHighPass->disable();
				m_buffer1->disable();

				//Takes the high pass and blurs it
				//glViewport(0, 0, Game::getWindowWidth() / 2, Game::getWindowHeight() / 2);
				for(int a = 0; a < blurPasses; a++)
				{
					m_buffer2->enable();
					m_blurHorizontal->enable();
					m_blurHorizontal->sendUniform("uTex", 0);
					m_blurHorizontal->sendUniform("uPixleSize", 1.0f / Game::getWindowHeight());
					glBindTexture(GL_TEXTURE_2D, m_buffer1->getColorHandle(0));
					FrameBuffer::drawFullScreenQuad();

					glBindTexture(GL_TEXTURE_2D, GL_NONE);
					m_blurHorizontal->disable();


					m_buffer1->enable();
					m_blurVertical->enable();
					m_blurVertical->sendUniform("uTex", 0);
					m_blurVertical->sendUniform("uPixleSize", 1.0f / Game::getWindowWidth());
					glBindTexture(GL_TEXTURE_2D, m_buffer2->getColorHandle(0));
					FrameBuffer::drawFullScreenQuad();

					glBindTexture(GL_TEXTURE_2D, GL_NONE);
					m_blurVertical->disable();
				}

				FrameBuffer::disable();//return to base frame buffer

				m_greyscaleBuffer->setViewport(0, 0, 0);

				m_greyscaleBuffer->enable();
				m_blurrComposite->enable();
				glActiveTexture(GL_TEXTURE0);
				m_blurrComposite->sendUniform("uScene", 0);
				glBindTexture(GL_TEXTURE_2D, postBuff->getColorHandle(0));

				glActiveTexture(GL_TEXTURE1);
				m_blurrComposite->sendUniform("uBloom", 1);
				glBindTexture(GL_TEXTURE_2D, m_buffer1->getColorHandle(0));

				m_blurrComposite->sendUniform("uBloomEnable", enableBloom);
				FrameBuffer::drawFullScreenQuad();

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, GL_NONE);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, GL_NONE);
				m_blurrComposite->disable();
				m_greyscaleBuffer->disable();
				m_greyscaleBuffer->copyColourToBuffer(postBuff->getDepthWidth(), postBuff->getDepthWidth(), postBuff);
			#pragma endregion
				break;
			case post3:
			#pragma region Post 3

			#pragma endregion
				break;
			}

		#pragma region LUT/Grayscale


			//3D look up table being applied and grayscale
			postBuff->enable();
			m_lutNGrayscaleShader->enable();

			m_lutNGrayscaleShader->sendUniform("uTex", 0);//previous colour buffer
			m_lutNGrayscaleShader->sendUniform("customTexure", 1);//LUT
			m_lutNGrayscaleShader->sendUniform("lutSize", ResourceManager::getTextureLUT(lutPath.c_str()).lutSize);
			m_lutNGrayscaleShader->sendUniform("lutActive", lutActive);

			postBuff->getColorTexture(0).bindTexture(0);//previous colour buffer
			ResourceManager::getTextureLUT(lutPath.c_str()).bindTexture(1);//LUT

			FrameBuffer::drawFullScreenQuad();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, GL_NONE);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_3D, GL_NONE);

			m_lutNGrayscaleShader->disable();
			postBuff->disable();
		#pragma endregion

		#pragma region Toon Shading

			//Shading
			postBuff->enable();
			m_toonPost.enable();

			m_toonPost.sendUniform("usceneTex", 0);
			m_toonPost.sendUniform("gamma", 0.8f);
			m_toonPost.sendUniform("numColors", 3);

			postBuff->getColorTexture(0).bindTexture(0);

			postBuff->drawFullScreenQuad();

			m_toonPost.disable();
			postBuff->disable();

			//Outline
			postBuff->enable();
			m_sobel->enable();

			m_sobel->sendUniform("uNormalMap", 0);
			m_sobel->sendUniform("uSceneTex", 1);
			m_sobel->sendUniform("uDepthMap", 2);
			m_sobel->sendUniform("uPixelSize", Vec2{1 / (float)postBuff->getColourWidth(0),1 / (float)postBuff->getColourHeight(0)});

			gbuff->getColorTexture(2).bindTexture(0);
			postBuff->getColorTexture(0).bindTexture(1);
			Texture2D::bindTexture(2, gbuff->getDepthHandle());

			postBuff->drawFullScreenQuad();

			m_sobel->disable();
			postBuff->disable();
		#pragma endregion



		#pragma region Screen Shake

		#define shake(amount) ((rand() % amount) * (rand() % 2 ? 1 : -1))

			static bool center = true;
			if(!center)
				m_screenshake->setViewport(shake(10), shake(10), 0);

			center = !center;

			Shader* shaker = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "shaders/Main Buffer.fmsh");

			m_screenshake->enable();
			shaker->enable();
			shaker->sendUniform("uTex", 0);
			postBuff->getColorTexture(0).bindTexture(0);

			m_screenshake->drawFullScreenQuad();

			shaker->disable();
			m_screenshake->disable();

			m_screenshake->copySingleColourToBuffer(postBuff->getColourWidth(0), postBuff->getColourHeight(0), postBuff);
		#pragma endregion
		};

		_map.create(new PrimitivePlane(Vec3(40.0f, 0.0f, 40.0f)));
		_map.replaceTexture(0, 0, ResourceManager::getTexture2D("Textures/play rug.jpg").id);
		Game::addModel(&_map);
		int count = 0;

		//Audio
		AudioPlayer::init();

		if(!AudioPlayer::createAudioStream("Music/song 1/Main Beet (midevil).wav"))
			puts("Audio Not Playing");
		beats = BeatMapReader::loadBeatMap("Music/song 1/beat.bmap");
		AudioPlayer::play(true);

		AudioPlayer::setMasterVolume(.5f);

		//enemy waypoints 
		points.resize(33);
		for(auto& point : points)
		{
			point = std::shared_ptr<WayPoint>(new WayPoint());
			point->create(new PrimitiveSphere(.5, .5, 10, 10, {0,.25,0}));
			point->setColour(1, 0.5, 0.5);
			//point->translate(!(count % 2) ? -_map.getWidth()*.5 : _map.getWidth() * .5, 0, _map.getDepth() * .5 - ((float)count/points.size() * _map.getDepth()));
			point->translate(Vec3(_map.getWidth(), 0, _map.getDepth()));
			point->setScale(1);
			Game::addModel(point.get());
			++count;
		}

	#pragma region This is HELL!!

		float height = 0.1f;
		points[0]->translate(Vec3((-_map.getWidth() / 2), height, (_map.getDepth() / 2 - (_map.getDepth() / 6)) - 1));
		points[1]->translate(Vec3((-_map.getWidth() / 2) + 6, height, (_map.getDepth() / 2 - (_map.getDepth() / 6)) - 4));
		points[2]->translate(Vec3((-_map.getWidth() / 2) + 13, height, (_map.getDepth() / 2 - (_map.getDepth() / 6)) - 4));
		points[3]->translate(Vec3((-_map.getWidth() / 2) + 16, height, (_map.getDepth() / 2 - (_map.getDepth() / 6))));
		points[4]->translate(Vec3((-_map.getWidth() / 2) + 23, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) + 2)));
		points[5]->translate(Vec3((-_map.getWidth() / 2) + 25, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 4)));
		points[6]->translate(Vec3((-_map.getWidth() / 2) + 29, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 6)));
		points[7]->translate(Vec3((-_map.getWidth() / 2) + 32, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 9)));
		points[8]->translate(Vec3((-_map.getWidth() / 2) + 32.5f, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 14)));
		points[9]->translate(Vec3((-_map.getWidth() / 2) + 36, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 18)));
		points[10]->translate(Vec3((-_map.getWidth() / 2) + 39, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 22)));
		points[11]->translate(Vec3((-_map.getWidth() / 2) + 39, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 25)));
		points[12]->translate(Vec3((-_map.getWidth() / 2) + 30, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 25)));
		points[13]->translate(Vec3((-_map.getWidth() / 2) + 27, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 26)));
		points[14]->translate(Vec3((-_map.getWidth() / 2) + 25, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 24)));
		points[15]->translate(Vec3((-_map.getWidth() / 2) + 27, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 21)));
		points[16]->translate(Vec3((-_map.getWidth() / 2) + 29, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 23)));
		points[17]->translate(Vec3((-_map.getWidth() / 2) + 27, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 26)));
		points[18]->translate(Vec3((-_map.getWidth() / 2) + 25, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 24)));
		points[19]->translate(Vec3((-_map.getWidth() / 2) + 25, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 21)));
		points[20]->translate(Vec3((-_map.getWidth() / 2) + 24, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 18)));
		points[21]->translate(Vec3((-_map.getWidth() / 2) + 23, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 15)));
		points[22]->translate(Vec3((-_map.getWidth() / 2) + 22, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 12)));
		points[23]->translate(Vec3((-_map.getWidth() / 2) + 19, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 13)));
		points[24]->translate(Vec3((-_map.getWidth() / 2) + 14, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 8)));
		points[25]->translate(Vec3((-_map.getWidth() / 2) + 10, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 13)));
		points[26]->translate(Vec3((-_map.getWidth() / 2) + 10, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 15)));
		points[27]->translate(Vec3((-_map.getWidth() / 2) + 15, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 14)));
		points[28]->translate(Vec3((-_map.getWidth() / 2) + 15, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 18)));
		points[29]->translate(Vec3((-_map.getWidth() / 2) + 13, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 21)));
		points[30]->translate(Vec3((-_map.getWidth() / 2) + 13, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 28)));
		points[31]->translate(Vec3((-_map.getWidth() / 2) + 14, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 31)));
		points[32]->translate(Vec3((-_map.getWidth() / 2) + 16, height, (_map.getDepth() / 2 - (_map.getDepth() / 6) - 35)));

	#pragma endregion

		//Towers
		towers.resize(3);
		count = 0;
		std::srand(std::time(nullptr));
		for(auto& tower : towers)
		{
			//change to do different towers
			int rando = std::rand() % 3;
			switch(rando)
			{
			case 0:
				tower = std::shared_ptr<QuarterTower>(new QuarterTower());
				break;
			case 1:
				tower = std::shared_ptr<EigthTower>(new EigthTower());
				break;
			case 2:
				tower = std::shared_ptr<WholeTower>(new WholeTower());
				break;
			}

			tower->create("Models/woodtower/woodtower.obj");
			tower->setScale(0.2f);

			tower->translateBy(0, 0, -5.f * count++);

			tower->setEnemyList(&enemies);
			tower->setSongBPM(beats[0].bpm);
			std::vector<Model*>  tmp;


			Game::addModel(tower.get());
		}


		////base enemies
		//enemies.resize(4);
		//for(auto& enemyBase : enemies)
		//{
		//	enemyBase = std::shared_ptr<QuarterEnemy>(new QuarterEnemy());
		//
		//	enemyBase->create("Models/ae-86/ae-86.obj");
		//	enemyBase->setColour(0.5, 0.5, 1);
		//	enemyBase->setScale(0.6f);
		//	enemyBase->translate(points[0]->getLocalPosition());
		//	Game::addModel(enemyBase.get());
		//}
		//enemies[0]->setWayPoints(points);


		//Lights & Positions
		Game::translateCamera({0.0f, 45.0f, -20.0f});
		Game::rotateCamera({-70.0f, 0.0f, 0.0f});
		Game::getMainCamera()->enableFPSMode(true);


		lit.setLightType(Light::TYPE::DIRECTIONAL);
		lit.rotate(-15, 0, 0);
		LightManager::addLight(&lit);
		setSkyBox("Skyboxes/skybox/");
		enableSkyBox(true);


		keyPressed =
			[&](int key, int mod)->void
		{



			if(key == 'R')
			{
				Game::getMainCamera()->reset();
				Game::translateCamera({0,0,-3});
			}

			static bool sky = true, frame = false;
			if(key == GLFW_KEY_SPACE)
				pause = !pause;


			if(key == GLFW_KEY_F5)
				Shader::refresh();

			//static int count;
			if(key == GLFW_KEY_TAB)
				tab = !tab;

			if(key == 'A')
				moveLeft = true;

			if(key == 'D')
				moveRight = true;

			if(key == 'W')
				moveForward = true;

			if(key == 'S')
				moveBack = true;

			if(key == 'Q')
				moveDown = true;

			if(key == 'E')
				moveUp = true;


			if(key == GLFW_KEY_PAGE_UP)
				tiltLeft = true;

			if(key == GLFW_KEY_PAGE_DOWN)
				tiltRight = true;

			if(key == GLFW_KEY_LEFT)
				rotLeft = true;

			if(key == GLFW_KEY_RIGHT)
				rotRight = true;

			if(key == GLFW_KEY_UP)
				rotUp = true;

			if(key == GLFW_KEY_DOWN)
				rotDown = true;
		};

		keyReleased =
			[&](int key, int mod)->void
		{
			if(key == 'A')
				moveLeft = false;

			if(key == 'D')
				moveRight = false;

			if(key == 'W')
				moveForward = false;

			if(key == 'S')
				moveBack = false;

			if(key == 'Q')
				moveDown = false;

			if(key == 'E')
				moveUp = false;


			if(key == GLFW_KEY_PAGE_UP)
				tiltLeft = false;

			if(key == GLFW_KEY_PAGE_DOWN)
				tiltRight = false;

			if(key == GLFW_KEY_LEFT)
				rotLeft = false;

			if(key == GLFW_KEY_RIGHT)
				rotRight = false;

			if(key == GLFW_KEY_UP)
				rotUp = false;

			if(key == GLFW_KEY_DOWN)
				rotDown = false;

			puts(Game::getMainCamera()->getLocalPosition().toString());
		};
	}

	void update(double dt)
	{
		cameraMovement((float)dt);

		static double timer = 0;
		static int beatOn = 0, lastBeatOn = 0;

		timer = AudioPlayer::getTimePosition(0) * .001;//seconds
		static double bps = 60 / beats[0].bpm * .5;//eighth notes

		if((beatOn = (int)(timer / bps)) != lastBeatOn)
		{
			lastBeatOn = beatOn;

			bool rest = rand() % 2;


			if(!(beatOn % 16) && !rest)//two whole bars(eight eightnote counts per bar)
				enemies.push_back(std::shared_ptr<WholeEnemy>(new WholeEnemy()));
			else if(!(beatOn % 2) && !rest)
				enemies.push_back(std::shared_ptr<QuarterEnemy>(new QuarterEnemy()));
			else if(!((beatOn) % 1) && !rest)
				enemies.push_back(std::shared_ptr<EighthEnemy>(new EighthEnemy()));

			static Model enemy("Models/enemy 1/enemy_nonGDW3.obj");

			if(!rest)
			{
				enemies.back()->create(enemy);
				enemies.back()->setScale(1.6f);
				enemies.back()->setColour(1, 1, 1);
				//enemies.back()->;

				Vec3 pos = /*map1.getWayPoints()[0]->getPosition()*/ points[0]->getLocalPosition();
				enemies.back()->translate(pos + Vec3{-2,0,0});
				enemies.back()->setActive(true);
				enemies.back()->setWayPoints(/*map1.getWayPoints()*/points);

				Game::addModel(enemies.back().get());
			}
		}

		//updates
		for(auto& enemy : enemies)
			enemy->update((float)dt);

		for(auto& tower : towers)
			tower->update((float)dt);

		//enemy clean up
		for(auto& a : enemies)
		{
			if(!a.get())continue;
			a->update((float)dt);

			if(a->getHealth() <= 0)
			{
				Game::removeModel(a.get());
				enemies.erase(std::find(enemies.begin(), enemies.end(), a));
			}

			if(!a->isActive())
			{
				Game::removeModel(a.get());
				enemies.erase(std::find(enemies.begin(), enemies.end(), a));
			}
		}

		Tower::bulletUpdate((float)dt);
	}

	void onSceneExit() {}
};
#pragma endregion

int main()
{
	Game::init("Da Game", 1620, 780);
	Test test;

	//Song song;//just another scene... move along
	Game::setScene(&test);
	Game::run();

	return 0;
}