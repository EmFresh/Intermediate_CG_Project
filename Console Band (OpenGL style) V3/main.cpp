//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include <GameEmGine.h>
#include "Song.h"
#include "Menu.h"

static std::string lutPath = "textures/hot.cube";
class Test: public Scene
{

#pragma region Variables

	float speed = 0.1f, angle = 1;
	Animation ani;

	Model bigBoss[2];
	Model rocket;

	Text testText;
	Light lit;
	bool moveLeft, moveRight, moveForward, moveBack, moveUp, moveDown,
		rotLeft, rotRight, rotUp, rotDown, tiltLeft, tiltRight,
		tab = false, lutActive = false, enableBloom = false;
	Shader
		* m_lutNGrayscaleShader, * m_bloomHighPass,
		* m_blurHorizontal, * m_blurVertical,
		* m_blurrComposite, * m_sobel;
	FrameBuffer
		* m_buffer1, * m_buffer2,
		* m_greyscaleBuffer, * m_outline;
#pragma endregion

public:
#define SCREEN_RATIO 2

	void init()
	{
		Game::setBackgroundColour(.15f, .15f, .15f);
		Game::setCameraPosition({0,0,-3});
		FrustumPeramiters frustum{65,(float)Game::getWindowWidth() / Game::getWindowHeight(),0.001f,500};
		Game::setCameraType(&frustum);

		setSkyBox("Skyboxes/space/");
		enableSkyBox(true);

		m_bloomHighPass = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/BloomHighPass.fmsh");
		m_blurHorizontal = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/BlurHorizontal.fmsh");
		m_blurVertical = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/BlurVertical.fmsh");
		m_blurrComposite = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/BloomComposite.fmsh");

		m_lutNGrayscaleShader = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/GrayscalePost.fmsh");
		m_sobel = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "shaders/Sobel.fmsh");


		m_greyscaleBuffer = new FrameBuffer(1, "Greyscale");
		m_buffer1 = new FrameBuffer(1, "Test1");
		m_buffer2 = new FrameBuffer(1, "Test2");
		m_outline = new FrameBuffer(1, "Sobel Outline");


		m_greyscaleBuffer->initColourTexture(0, Game::getWindowWidth(), Game::getWindowHeight(), GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		if(!m_greyscaleBuffer->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}

		m_buffer1->initColourTexture(0, Game::getWindowWidth() / SCREEN_RATIO, Game::getWindowHeight() / SCREEN_RATIO, GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		if(!m_buffer1->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}
		m_buffer2->initColourTexture(0, Game::getWindowWidth() / SCREEN_RATIO, Game::getWindowHeight() / SCREEN_RATIO, GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);

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



		customPostEffect =
			[&](FrameBuffer* gbuff, FrameBuffer* postBuff)->void
		{
			m_greyscaleBuffer->clear();
			m_buffer1->clear();
			m_buffer2->clear();

			glViewport(0, 0, Game::getWindowWidth() / SCREEN_RATIO, Game::getWindowHeight() / SCREEN_RATIO);

			//binds the initial high pass to buffer 1
			m_buffer1->enable();
			m_bloomHighPass->enable();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, postBuff->getColorHandle(0));

			m_bloomHighPass->sendUniform("uTex", 0);
			m_bloomHighPass->sendUniform("uThresh", 0.15f);

			FrameBuffer::drawFullScreenQuad();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, GL_NONE);

			m_bloomHighPass->disable();
			m_buffer1->disable();

			//Takes the high pass and blurs it
			glViewport(0, 0, Game::getWindowWidth() / SCREEN_RATIO, Game::getWindowHeight() / SCREEN_RATIO);
			for(int a = 0; a < SCREEN_RATIO; a++)
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


			glClearDepth(1.f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


			//3D look up table being applied and grayscale
			postBuff->enable();
			m_lutNGrayscaleShader->enable();

			m_lutNGrayscaleShader->sendUniform("uTex", 0);//previous colour buffer
			m_lutNGrayscaleShader->sendUniform("customTexure", 6);//LUT
			m_lutNGrayscaleShader->sendUniform("lutSize", ResourceManager::getTextureLUT(lutPath.c_str()).lutSize);
			m_lutNGrayscaleShader->sendUniform("lutActive", lutActive);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_greyscaleBuffer->getColorHandle(0));//previous colour buffer
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_3D, ResourceManager::getTextureLUT(lutPath.c_str()).id);//LUT

			FrameBuffer::drawFullScreenQuad();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, GL_NONE);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_3D, GL_NONE);

			m_lutNGrayscaleShader->disable();
			postBuff->disable();
		};


		// Scene setup 
		bigBoss[0].create("Models/BOSS/slam/bsl1.obj", "Boss1");
		puts("is it 1?");
		bigBoss[0].translate(-15, 0, 0);

		ani.setAnimationSpeed(1);
		ani.addDir("Models/BOSS/slam/");
		bigBoss[0].addAnimation("IDK", &ani);

		bigBoss[0].setAnimation("IDK");
		ani.play();
		ani.repeat(true);


		rocket.create("Models/rocket-ship/rocket ship.obj", "ship");
		puts("is it 2?");
		bigBoss[1].create(bigBoss[0], "Boss2");
		puts("is it 3?");

		bigBoss[1].translate(15, 0, 0);
		bigBoss[0].rotate(0, 90, 0);
		bigBoss[1].rotate(0, -90, 0);

		//floor 
		static Model floor, transCube;
		floor.create(new PrimitivePlane({100,0,100}), "floor");
		transCube.create(new primitiveCube({20,20,20}), "floor");
		transCube.setTransparent(true);
		transCube.setColour(.2, .6, .05, .2);
		Game::addModel(&transCube);
		puts("is it 4?");

		Game::addModel(&floor);
		floor.replaceTexture(0, 0, ResourceManager::getTexture2D("Textures/moon.jpg").id);



		lit.setLightType(Light::TYPE::POINT);
		lit.setParent(Game::getMainCamera());
		lit.setSpecular({0,0,145});

		LightManager::addLight(&lit);

		//testText.setText("Maybe this Works?");
		//testText.setColour(1, 0, 0);
		//testText.textSize(20);
		//testText.toTexture(50);		

		Game::addModel(&bigBoss[0]);
		Game::addModel(&bigBoss[1]);


		//Game::addText(&testText);

		Game::getMainCamera()->enableFPSMode();
		Game::addModel(&rocket);

		//forward.translate(Game::getMainCamera()->getForward());
		//model[0].replaceTexture(0, 0, testText.getTexture());

		keyPressed =
			[&](int key, int mod)->void
		{

			switch(key)
			{
			case GLFW_KEY_1:
				lit.enableDiffuse(false);
				lit.enableSpecular(false);
				enableBloom = (false);
				//lutActive = false;
				break;
			case GLFW_KEY_2:
				lit.enableDiffuse(true);
				lit.enableSpecular(false);
				enableBloom = (false);
				//lutActive = false;
				break;
			case GLFW_KEY_3:
				lit.enableDiffuse(false);
				lit.enableSpecular(true);
				enableBloom = (false);
				//lutActive = false;
				break;
			case GLFW_KEY_4:
				lit.enableDiffuse(true);
				lit.enableSpecular(true);
				enableBloom = (false);
				//lutActive = false;
				break;
			case GLFW_KEY_5:
				lit.enableDiffuse(true);
				lit.enableSpecular(true);
				enableBloom = (true);
				//lutActive = false;
				break;


			case GLFW_KEY_6:
				lit.rampActiveDiff = !lit.rampActiveDiff;
				break;
			case GLFW_KEY_7:
				lit.rampActiveSpec = !lit.rampActiveSpec;
				break;
			case GLFW_KEY_8:
				if(lutPath == "textures/hot.cube")
					lutActive = !lutActive;
				else
					lutActive = true;

				lutPath = "textures/hot.cube";
				break;
			case GLFW_KEY_9:
				if(lutPath == "textures/cold.cube")
					lutActive = !lutActive;
				else
					lutActive = true;
				lutPath = "textures/cold.cube";
				break;
			case GLFW_KEY_0:
				if(lutPath == "textures/ye.cube")
					lutActive = !lutActive;
				else
					lutActive = true;
				lutPath = "textures/ye.cube";
				break;
			}


			if(key == 'R')
			{
				Game::getMainCamera()->reset();
				Game::setCameraPosition({0,0,-3});
			}
			static bool sky = true, frame = false;
			if(key == 'N')
				rocket.setWireframe(frame = !frame);
			if(key == GLFW_KEY_SPACE)
				enableSkyBox(sky = !sky);

			if(key == GLFW_KEY_F5)
				Shader::refresh();

			//static int count;
			if(key == GLFW_KEY_TAB)
				tab = !tab;//	std::swap(model[0], model[count++]);

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
		};

		//	auto list = Component::getComponentList();
		//	printf("there are %d items in the list\n", list.size());
		//	puts("here they are: ");
		//	for(auto& a : list)
		//		for(uint b = 0; b < a.second; ++b)
		//			puts(a.first.c_str());
	}

	void cameraMovement()
	{
		// Movement
		if(moveLeft)
			Game::translateCameraBy({-speed,0,0});
		if(moveRight)
			Game::translateCameraBy({speed,0,0});
		if(moveForward)
			Game::translateCameraBy({0,0,speed});
		if(moveBack)
			Game::translateCameraBy({0,0,-speed});
		if(moveUp)
			Game::translateCameraBy({0,speed,0});
		if(moveDown)
			Game::translateCameraBy({0,-speed,0});

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

	void objectMovement()
	{
		// Movement
		if(moveLeft)
			bigBoss[0].translateBy({-speed,0.f,0.f});
		if(moveRight)
			bigBoss[0].translateBy({speed,0,0});
		if(moveForward)
			bigBoss[0].translateBy({0,0,speed});
		if(moveBack)
			bigBoss[0].translateBy({0,0,-speed});
		if(moveUp)
			bigBoss[0].translateBy({0,speed,0});
		if(moveDown)
			bigBoss[0].translateBy({0,-speed,0});

		// Rotation
		if(rotLeft)
			bigBoss[0].rotateBy({0,-angle,0});
		if(rotRight)
			bigBoss[0].rotateBy({0,angle,0});
		if(rotDown)
			bigBoss[0].rotateBy({-angle,0,0});
		if(rotUp)
			bigBoss[0].rotateBy({angle,0,0});


	}

	void update(double dt)
	{
		cameraMovement();
		float speed = 5;
		ani.setAnimationSpeed((float)speed / ani.getTotalFrames());
		ani.stop();
		bigBoss[0].translate(lerp(Coord3D<>{-60, 0, 0}, Coord3D<>{-15, 0, 0}, .6f));
		bigBoss[1].translate(lerp(Coord3D<>{60, 0, 0}, Coord3D<>{15, 0, 0}, fmodf(clock() / (float)CLOCKS_PER_SEC + 1, speed) / speed));
	}
};

int main()
{
	Game::init("Assignment 1", 1920, 1080);

	Test test;
	//Song song;
	Game::setScene(&test);
	Game::run();

	return 0;
}