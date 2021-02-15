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

	float speed = 20, angle = 1, bloomThresh = 0.15f;
	Animation ani;

	Model models[10];
	Transformer trans[10];
	Model bigBoss[2];
	Model rocket;

	Text testText;
	Light lit;
	bool moveLeft, moveRight, moveForward, moveBack, moveUp, moveDown,
		rotLeft, rotRight, rotUp, rotDown, tiltLeft, tiltRight,
		tab = false, lutActive = false, enableBloom = false,pause=false;
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

		m_buffer1->initColourTexture(0, Game::getWindowWidth() / blurPasses, Game::getWindowHeight() / blurPasses, GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		if(!m_buffer1->checkFBO())
		{
			puts("FBO failed Creation");
			system("pause");
			return;
		}
		m_buffer2->initColourTexture(0, Game::getWindowWidth() / blurPasses, Game::getWindowHeight() / blurPasses, GL_RGB8, GL_LINEAR, GL_CLAMP_TO_EDGE);

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
		models[0].create("models/solar system/Sun/Sun.obj", "Sun");
		models[1].create("models/solar system/Mercury/Mercury.obj", "Mercury");
		models[2].create("models/solar system/Venus/Venus.obj", "Venus");
		models[3].create("models/solar system/Earth/Earth.obj", "Earth");
		models[4].create("models/solar system/Mars/Mars.obj", "Mars");
		models[5].create("models/solar system/Jupiter/Jupiter.obj", "Jupiter");
		models[6].create("models/solar system/Saturn/Saturn.obj", "Saturn");
		models[7].create("models/solar system/Uranus/Uranus.obj", "Uranus");
		models[8].create("models/solar system/Neptune/Neptune.obj", "Neptune");

		for(int a = 0; a < 9; ++a)
		{
			if(a)
				models[a].setParent(&trans[a]);

			Game::addModel(&models[a]);
			models[a].setScale(.1f);
			models[a].translateBy({25.f * a,0,0});
		}
		models[1].scaleBy(.2f);
		models[2].scaleBy(.2f);
		models[3].scaleBy(.2f);
		models[4].scaleBy(.2f);
		models[5].scaleBy(.8f);
		models[6].scaleBy(.7f);
		models[7].scaleBy(.5f);
		models[8].scaleBy(.4f);

		//(161.874771, 74.611961, 82.858345)
		Game::setCameraPosition({161.874771f, 74.611961f, -82.858345f});
		Game::getMainCamera()->enableFPSMode();

		lit.setLightType(Light::TYPE::POINT);
		lit.setParent(Game::getMainCamera());
		lit.setDiffuse({155,0,0});
		LightManager::addLight(&lit);


		//Key binds
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
				for(int a = 0; a < 9; ++a)
				{
					models[a].enableTexture(!models[a].isTextureEnabled());
					if(models[a].isTextureEnabled())
						models[a].setColour(1, 1, 1);
					else
						models[a].setColour(.35f, 0, .45f);
				}


			case GLFW_KEY_KP_4:
				bloomThresh -= .05;
				break;

			case GLFW_KEY_KP_6:
				bloomThresh += .05;
				break;

			case GLFW_KEY_COMMA:
				blurPasses -= 1;
				if(!blurPasses)blurPasses = 1;
				break;
			case GLFW_KEY_PERIOD:
				blurPasses += 1;
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
				pause = !pause;
				//enableSkyBox(sky = !sky);

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

			puts(Game::getMainCamera()->getPosition().toString());
		};

		EmGineAudioPlayer::createAudioStream("songs/still alive.mp3");
		EmGineAudioPlayer::getAudioControl()[0][0]->channel->set3DMinMaxDistance(20, 200);

		EmGineAudioPlayer::play(true);
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
		cameraMovement(dt);
		float maxSpeed = 10;

		if(!pause)
		{

		trans[1].rotateBy({0,maxSpeed * 1.0f * (float)dt,0});
		trans[2].rotateBy({0,maxSpeed * 0.9f * (float)dt,0});
		trans[3].rotateBy({0,maxSpeed * 0.8f * (float)dt,0});
		trans[4].rotateBy({0,maxSpeed * 0.7f * (float)dt,0});
		trans[5].rotateBy({0,maxSpeed * 0.6f * (float)dt,0});
		trans[6].rotateBy({0,maxSpeed * 0.5f * (float)dt,0});
		trans[7].rotateBy({0,maxSpeed * 0.4f * (float)dt,0});
		trans[8].rotateBy({0,maxSpeed * 0.3f * (float)dt,0});
		for(int a = 0; a < 9; ++a)
			models[a].rotateBy(0, (10 - a) * 5 * dt, 0);
		}

		auto tmpOBJPos = models[0].getPosition();
		EmGineAudioPlayer::getAudioControl()[0][0]->listener->pos = *(FMOD_VEC3*)&tmpOBJPos;
		
		auto tmpPos = Game::getMainCamera()->getPosition();
		auto tmpUp = Game::getMainCamera()->getUp();
		auto tmpForward = Game::getMainCamera()->getForward();
		EmGineAudioPlayer::getAudioSystem()->set3DListenerAttributes(0, (FMOD_VEC3*)&tmpPos, nullptr,(FMOD_VEC3*)&tmpForward, (FMOD_VEC3*)&tmpUp);

		EmGineAudioPlayer::update();


	}
};

int main()
{
	Game::init("Assignment 1", 1900, 1060);

	Test test;
	//Song song;//just another scene... move along
	Game::setScene(&test);
	Game::run();

	return 0;
}