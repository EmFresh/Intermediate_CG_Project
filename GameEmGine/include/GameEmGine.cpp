#include "EmGineAudioPlayer.h"
#include "GameEmGine.h"

#pragma region Static Variables
void(*m_compileShaders)();
std::function<void(FrameBuffer* gbuff, FrameBuffer* post)> m_customRender;
//std::function<void()>GameEmGine::m_render;
std::function<void(double)>m_gameLoop;
Camera* m_mainCamera;
std::vector<Camera*>m_cameras;
Shader
* m_gBufferShader, * m_postProcessShader,
* m_forwardRender;

FrameBuffer
* m_gBuffer, * m_postBuffer;


WindowCreator* m_window;	//must be init in the constructor
ColourRGBA m_colour{123,123,123};

std::unordered_map<std::string, FrameBuffer*> m_frameBuffers;
std::unordered_map<void*, Model*> m_models;

bool exitGame = false;
float m_fps;
short m_fpsLimit;
Scene* m_mainScene;
std::vector<Text*> m_text;
Coord2D<int> m_screenSize;
bool m_bloom;


//GLuint GameEmGine::colorCustom;
//int GameEmGine::LUTsize = 0;

bool GameEmGine::lutActive = false;


#pragma endregion

void GLAPIENTRY
GameEmGine::MessageCallback(GLenum source,
							GLenum type,
							GLuint id,
							GLenum severity,
							GLsizei length,
							const GLchar* message,
							const void* userParam)
{
	source, id, length, userParam;
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
}

Texture3D GameEmGine::tmpLUT;
Texture2D tmpRamp;
std::string LUTpath;

void GameEmGine::init(std::string name, int width, int height, int x, int y, int monitor, bool fullScreen, bool visable)
{
	createNewWindow(name, width, height, x, y, monitor, fullScreen, visable);
	AudioPlayer::init();
	InputManager::init();

	tmpRamp = ResourceManager::getTexture2D("textures/Texture Ramp.png");



	//LUTpath = "Texture/IWLTBAP_Aspen_-_Standard.cube";
	///////////////////////////////////////Bind Custom 3D Texture////////////////////////////////////////////
	//
	//tmpLUT = ResourceManager::getTextureLUT(LUTpath.c_str());
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	///////////////////////////////////////Bind Custom 2D Texture////////////////////////////////////////////
	//
	//tmpRamp = ResourceManager::getTexture2D("Texture/pinkRamp.png");
	//
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
}

void GameEmGine::createNewWindow(std::string name, int width, int height, int x, int y, int monitor, bool fullScreen, bool visable)
{
	glfwInit();


	printf("Creating The Window...\n");

	if(m_window)
		delete m_window;

	m_window = new WindowCreator(name, {width,height}, Coord2D<int>{x, y}, monitor, fullScreen, visable);
	m_window->m_onWindowResizeCallback = changeViewport;


	if(m_window)
		puts("Window Creation Successful\n");
	else
	{
		puts("Window Creation Unsuccessful\n");
		return;
	}

	m_mainCamera = new Camera(Camera::FRUSTUM, {(float)getWindowWidth(), (float)getWindowHeight(),(float)getWindowWidth()});



	shaderInit();

	printf("created the window\n");

	m_gBuffer = new FrameBuffer(5, "Main Buffer");
	m_postBuffer = new FrameBuffer(1, "Post Process Buffer");


	m_gBuffer->initDepthTexture(getWindowWidth(), getWindowHeight());
	m_gBuffer->initColourTexture(0, getWindowWidth(), getWindowHeight(), GL_RGB16F, GL_NEAREST, GL_CLAMP_TO_EDGE);
	m_gBuffer->initColourTexture(1, getWindowWidth(), getWindowHeight(), GL_RGB16F, GL_NEAREST, GL_CLAMP_TO_EDGE);
	m_gBuffer->initColourTexture(2, getWindowWidth(), getWindowHeight(), GL_RGB16F, GL_NEAREST, GL_CLAMP_TO_EDGE);
	m_gBuffer->initColourTexture(3, getWindowWidth(), getWindowHeight(), GL_RGB16F, GL_NEAREST, GL_CLAMP_TO_EDGE);
	m_gBuffer->initColourTexture(4, getWindowWidth(), getWindowHeight(), GL_RGB8, GL_NEAREST, GL_CLAMP_TO_EDGE);
	if(!m_gBuffer->checkFBO())
	{
		puts("FBO failed Creation");
		system("pause");
		return;
	}

	m_postBuffer->initDepthTexture(getWindowWidth(), getWindowHeight());
	m_postBuffer->initColourTexture(0, getWindowWidth(), getWindowHeight(), GL_RGBA8, GL_NEAREST, GL_CLAMP_TO_EDGE);
	if(!m_postBuffer->checkFBO())
	{
		puts("FBO failed Creation");
		system("pause");
		return;
	}



	//// During init, enable debug output
	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(MessageCallback, 0);

}

void GameEmGine::run()
{

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	glEnable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	while(!glfwWindowShouldClose(m_window->getWindow()) && !exitGame)//update loop
	{
		glClearColor((float)m_colour.r / 255, (float)m_colour.g / 255, (float)m_colour.b / 255, (float)m_colour.a / 255);//BG colour
		//glClearColor(0.f, 0.f, 0.f, 0.f);
		glCullFace(GL_BACK);

		InputManager::update();
		update();

		if(true)//fps calculation
		{
			calculateFPS();
			char str[20];
			sprintf_s(str, "fps: %.2f", m_fps);

			glClear(GL_DEPTH_BUFFER_BIT);

			static Text fps;
			static OrthoPeramiters ortho{0,(float)getWindowWidth(),(float)getWindowHeight(),0,0,500};
			static Camera cam(&ortho);
			cam.update();

			fps.setColour(1, 0, 0);
			fps.setText(str);
			fps.textSize(35);

			fps.translate(0, fps.getHeight(), 0);
			fps.rotate(180, 0, 0);

			static std::unordered_map<void*, Model*> tmp;
			tmp[&fps] = (Model*)&fps;

			glClearDepth(1.f);
			glClear(GL_DEPTH_BUFFER_BIT);
			cam.render(nullptr, tmp, true);

			//glfwSetWindowTitle(m_window->getWindow(), (m_window->getTitle() + "--> " + str).c_str());
		}

		glfwSwapBuffers(m_window->getWindow());
		glFlush();
		fpsLimiter();
	}
	glfwInit();
	glfwTerminate();
}

void GameEmGine::exit()
{
	exitGame = true;
}

void GameEmGine::setFPSLimit(short limit)
{
	m_fpsLimit = limit;
}

short GameEmGine::getFPSLimit()
{
	return m_fpsLimit;
}

void GameEmGine::vsync(bool enable)
{
	glfwSwapInterval(enable);
}

void GameEmGine::updateControllerConnections()
{
	InputManager::controllerUpdate();
}

int GameEmGine::controllersConnected()
{
	return InputManager::controllersConnected();
}

bool GameEmGine::isControllerConnected(int m_index)
{
	return InputManager::isControllerConnected(m_index);
}

XinputDevice* GameEmGine::getController(int m_index)
{
	return InputManager::getController(m_index);
}

WindowCreator* GameEmGine::getWindow()
{
	return m_window;
}

void GameEmGine::shaderInit()
{
	m_gBufferShader = ResourceManager::getShader("Shaders/DeferredRender.vtsh", "Shaders/DeferredRender.fmsh");

	m_postProcessShader = ResourceManager::getShader("Shaders/Main Buffer.vtsh", "Shaders/PassThrough.frag");
	m_forwardRender = ResourceManager::getShader("Shaders/DeferredRender.vtsh", "Shaders/ForwardRender.fmsh");

	Shader::enableUniformErrors(false);
}

void GameEmGine::calculateFPS()
{
	static const int SAMPLE = 15;
	static short count = 0;
	static float frameTimes[SAMPLE]{};

	frameTimes[count++] = 1 / float(glfwGetTime());
	if(count == SAMPLE)
	{
		count = 0;
		m_fps = 0;
		for(auto& a : frameTimes)
			m_fps += a;
		m_fps /= SAMPLE;
	}

	glfwSetTime(0);
}

void GameEmGine::fpsLimiter()
{
	static bool enter = false;
	static clock_t frameStart = 0;


	//way 1: 
	if(enter)
		if(m_fpsLimit > 0)
			while((CLOCKS_PER_SEC / m_fpsLimit) > (clock() - frameStart));

	////way 2: puts the thread to sleep 
	//if(enter)
	//	if(m_fpsLimit > 0)
	//		sleep((CLOCKS_PER_SEC / m_fpsLimit) - (clock() - frameStart));

	frameStart = clock();

	enter = true;
}

void GameEmGine::setScene(Scene* scene)
{
	m_models.clear();
	m_frameBuffers.clear();
	LightManager::clear();
	m_frameBuffers[m_gBuffer->getTag()] = m_gBuffer;
	scene->setParent(m_mainScene);//set the parent to the previous scene
	m_mainScene = scene;
	scene->init();
	InputManager::setKeyPressedCallback(scene->keyPressed);
	InputManager::setKeyReleasedCallback(scene->keyReleased);
	InputManager::setKeyAllCallback(scene->keyInput);
	InputManager::mouseButtonPressedCallback(scene->mousePressed);
	InputManager::mouseButtonReleasedCallback(scene->mouseReleased);
	InputManager::mouseButtonAllCallback(scene->mouseInput);

	customRenderCallback([&](FrameBuffer* gbuff, FrameBuffer* post)->void{if(m_mainScene->customPostEffect)m_mainScene->customPostEffect(gbuff, post);  });

	//m_render = scene->render;
	m_gameLoop = [&](double a)->void {m_mainScene->update(a); };
}

void GameEmGine::setBackgroundColour(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	m_colour = {GLubyte(r * 255),GLubyte(g * 255),GLubyte(b * 255),GLubyte(a * 255)};
}

int GameEmGine::getWindowWidth()
{
	return m_window->getScreenWidth();
}

int GameEmGine::getWindowHeight()
{
	return m_window->getScreenHeight();
}

Coord2D<int> GameEmGine::getWindowSize()
{
	return m_window->getScreenSize();
}

Camera* GameEmGine::getMainCamera()
{
	return m_mainCamera;
}

bool GameEmGine::mouseCollision(Model* model)
{
	static PrimitiveCube smallCube({.01f});
	static Model mouse(&smallCube, "Mouse");

	addModel(&mouse);
	mouse.setColour(0, 1, 0);

	Camera* cam = getMainCamera();
	//	glm::mat4 tmp = glm::inverse(cam->getProjectionMatrix());
	Coord3D<> mPos = {InputManager::getMousePosition(),0};
	glm::vec4 direction((mPos * 2 / (Coord3D<>{(float)getWindowSize().x, (float)getWindowSize().y, 500} - 1)).toVec3(), 1);
	direction = {direction.x,-direction.y,1,1};


	direction = glm::inverse(cam->getViewMatrix() * cam->getProjectionMatrix()) * direction;
	direction = glm::normalize(direction);

	//position = position * 2 - 1;
	//position /= position.w;

	//position.z = cam->getPosition().z;
	mouse.translate(cam->getPosition());

	return mouse.collision2D(model, reclass(Coord3D<>, direction));
}

void GameEmGine::setCameraType(Camera::CAM_TYPE type)
{
	m_mainCamera->setType(type, nullptr);
}

void GameEmGine::setCameraType(ProjectionPeramiters* proj)
{
	m_mainCamera->setType(proj);
}

void GameEmGine::translateCameraBy(Coord3D<> pos)
{
	m_mainCamera->translateBy(pos);
}

void GameEmGine::setCameraPosition(Coord3D<> pos)
{
	m_mainCamera->translate(pos);
}

void GameEmGine::rotateCameraBy(Coord3D<> direction)
{
	m_mainCamera->rotateBy(direction);
}

void GameEmGine::setCameraRotation(Coord3D<> direction)
{
	m_mainCamera->rotate(direction);
}

void GameEmGine::addModel(Model* model)
{
	m_models[model] = model;
}

void GameEmGine::addText(Text* text)
{
	addModel(reclass(Model*, text));
}

void GameEmGine::removeModel(Model* model)
{
	if(model)
		m_models.erase(model);
}

void GameEmGine::removeText(Text* text)
{
	removeModel(reclass(Model*, text));
}

void GameEmGine::addCamera(Camera* cam)
{
	cam;

	//realloc(m_cameras, sizeof(Camera3D*)*++_numCameras);
	//m_cameras[_numCameras - 1] = cam;
}

void GameEmGine::enableBloom(bool bloom)
{
	m_bloom = bloom;
}

void GameEmGine::customRenderCallback(std::function<void(FrameBuffer*, FrameBuffer*)>render)
{
	m_customRender = render;
}

void GameEmGine::update()
{

	glClearDepth(1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);
	m_gBuffer->clear();//buffer must be black
	//glClearColor((float)m_colour.r / 255, (float)m_colour.g / 255, (float)m_colour.b / 255, (float)m_colour.a / 255);//BG colour


	glClearColor((float)m_colour.r / 255, (float)m_colour.g / 255, (float)m_colour.b / 255, (float)m_colour.a / 255);//BG colour
	
	m_postBuffer->clear();

	m_mainCamera->update();

	m_gBufferShader->enable();
	glUniformMatrix4fv(m_gBufferShader->getUniformLocation("uView"), 1, GL_FALSE, &(m_mainCamera->getViewMatrix()[0][0]));
	glUniformMatrix4fv(m_gBufferShader->getUniformLocation("uProj"), 1, GL_FALSE, &(m_mainCamera->getProjectionMatrix()[0][0]));
	m_gBufferShader->disable();

	m_forwardRender->enable();
	glUniformMatrix4fv(m_forwardRender->getUniformLocation("uView"), 1, GL_FALSE, &(m_mainCamera->getViewMatrix()[0][0]));
	glUniformMatrix4fv(m_forwardRender->getUniformLocation("uProj"), 1, GL_FALSE, &(m_mainCamera->getProjectionMatrix()[0][0]));
	m_forwardRender->disable();


	LightManager::setCamera(m_mainCamera);
	((SkyBox*)&m_mainScene->getSkyBox())->setCamera(m_mainCamera);


	glViewport(0, 0, getWindowWidth(), getWindowHeight());

	//Opaque renders 
	m_gBuffer->enable();
	m_mainCamera->render(m_gBufferShader, m_models, false);
	m_gBuffer->disable();

	//send depth info before rendering transparent objects
	m_gBuffer->moveDepthToBuffer(getWindowWidth(), getWindowHeight(), m_postBuffer->getFrameBufferID());

	//sky box
	m_gBuffer->enable();
	if(m_mainScene->skyBoxEnabled)
		(*(SkyBox*)&m_mainScene->getSkyBox()).render();
	m_gBuffer->disable();

	//transparent renders
	m_gBuffer->enable();
	m_mainCamera->render(m_gBufferShader, m_models, true);
	m_gBuffer->disable();

	{
		//store data for later post process
		m_postBuffer->enable();
		m_postProcessShader->enable();

		//bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorHandle(0));
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorHandle(1));
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorHandle(2));
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorHandle(3));
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorHandle(4));
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, tmpRamp.id);

		m_postProcessShader->sendUniform("uPosOP", 0);
		m_postProcessShader->sendUniform("uPosTrans", 1);
		m_postProcessShader->sendUniform("uNormOP", 2);
		m_postProcessShader->sendUniform("uNormTrans", 3);
		m_postProcessShader->sendUniform("uScene", 4);
		m_postProcessShader->sendUniform("uRamp", 5);

		glDisable(GL_DEPTH_TEST);

		//get the flat shaded scene
		m_postProcessShader->sendUniform("LightType", Light::TYPE::NONE);
		FrameBuffer::drawFullScreenQuad();

		//Apply lighting
		LightManager::setShader(m_postProcessShader);
		LightManager::setFramebuffer(m_postBuffer);
		LightManager::update();


		glEnable(GL_DEPTH_TEST);

		//un-bind textures
		for(int a = 0; a < 5; ++a)
			glActiveTexture(GL_TEXTURE0 + a),
			glBindTexture(GL_TEXTURE_2D, GL_NONE);

		m_postProcessShader->disable();
		m_postBuffer->disable();
	}

	//Apply shadows
	LightManager::shadowRender(1024, 1024, m_postBuffer, m_gBuffer, m_models);
	//glViewport(0, 0, getWindowWidth(), getWindowHeight());

	//post effects
	if(m_customRender)
		m_customRender(m_gBuffer, m_postBuffer);

	m_postBuffer->moveColourToBackBuffer(getWindowWidth(), getWindowHeight());
	m_postBuffer->moveDepthToBackBuffer(getWindowWidth(), getWindowHeight());

	if(m_gameLoop != nullptr)
		m_gameLoop(glfwGetTime());

	glfwPollEvents();//updates the event handlers
}

void GameEmGine::changeViewport(GLFWwindow*, int w, int h)
{
	if(!(w && h))return;
	m_screenSize = {w,h};
	glViewport(0, 0, w, h);

	switch(m_mainCamera->getType())
	{
	case Camera::FRUSTUM:

		FrustumPeramiters* tmp = (FrustumPeramiters*)m_mainCamera->getProjectionData();
		if(tmp)
			tmp->aspect = (float)w / h;
		m_mainCamera->setType(m_mainCamera->getType(), tmp);
		break;
	}


	//Framebuffer Resizing 
	m_gBuffer->resizeDepth(w, h);
	m_gBuffer->resizeColour(0, w, h);
	m_gBuffer->resizeColour(1, w, h);
	m_gBuffer->resizeColour(2, w, h);

	m_postBuffer->resizeDepth(w, h);
	m_postBuffer->resizeColour(0, w, h);
	//m_greyscaleBuffer->resizeColour(0, w, h);

	//m_buffer1->resizeColour(0, unsigned((float)w / SCREEN_RATIO), unsigned((float)h / SCREEN_RATIO));
	//m_buffer2->resizeColour(0, unsigned((float)w / SCREEN_RATIO), unsigned((float)h / SCREEN_RATIO));

}