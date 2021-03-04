#define _USE_MATH_DEFINES
#include <cmath>
#include "LightManager.h"
#include "WindowCreator.h"

#pragma region Static Variables
std::vector<Light*> m_lights;
FrameBuffer* m_framebuffer;
FrameBuffer* m_shadows;

Shader* m_shader;
Camera* m_cam;
//LightInfo LightSource::m_info;
//unsigned LightSource::m_size;
#pragma endregion


void LightManager::addLight(Light* lit)
{
	if(!lit)return;

	std::vector<Light*>::iterator ref;
	if((ref = std::find(m_lights.begin(), m_lights.end(), lit)) == m_lights.end())
		m_lights.push_back(lit);
}

void LightManager::removeLight(Light* lit)
{
	if(lit)
		m_lights.erase(std::find(m_lights.begin(), m_lights.end(), lit));
}

void LightManager::removeLight(unsigned index)
{
	if(index < m_lights.size())
		m_lights.erase(m_lights.begin() + index);
}

Light* LightManager::getLight(unsigned index)
{
	return m_lights[index];
}



void LightManager::setCamera(Camera* cam)
{
	m_cam = cam;
}

void LightManager::setShader(Shader* shad)
{
	m_shader = shad;
}

void LightManager::shadowRender(unsigned w, unsigned h, FrameBuffer* to, FrameBuffer* gBuff, std::unordered_map<void*, Model*>& models)
{
	static Camera cam(Camera::ORTHOGRAPHIC, {(float)80,(float)80,70});
	static glm::mat4 lsm(1);



	glViewport(0, 0, w, h);
	for(uint a = 0; a < m_lights.size(); ++a)
	{
		if(!m_lights[a]->shadowEnable)continue;

		if(m_lights[a]->type == Light::TYPE::DIRECTIONAL)
		{

			cam.translate(-m_lights[a]->getPosition());

			//initialize shadow buffer
			if(!m_shadows)
			{
				m_shadows = new FrameBuffer(0, "shadow buffer");
				m_shadows->initDepthTexture(w, h);
				if(!m_shadows->checkFBO())
				{
					puts("shadow buffer fbo not init.");
					system("pause");
					return;
				}
			}

			Shader* shad = ResourceManager::getShader("Shaders/ShadowDepth.vtsh", "Shaders/ShadowDepth.fmsh");
			m_shadows->resizeDepth(w, h);
			m_shadows->clear();

			shad->enable();
			shad->sendUniform("lightSpaceMatrix", lsm =
							  cam.getProjectionMatrix() *
							  glm::lookAt(glm::vec3(m_lights[a]->getWorldRotationMatrix() *
							  glm::vec4(m_lights[a]->getForward().toVec3(), 1))
							  * glm::vec3{40,40,70}, glm::vec3(), glm::vec3(0, 1, 0)));
			shad->disable();

			// use this for point lights
			for(int b = 0; b < 6; b++)
			{


				//	switch(b)
				//	{
				//	case 0:
				//		cam.rotate(90 * Coord3D<>{1, 0, 0});
				//		break;
				//	case 1:
				//		cam.rotate(90 * Coord3D<>{-1, 0, 0});
				//		break;
				//	case 2:
				//		cam.rotate(90 * Coord3D<>{0, 1, 0});
				//		break;
				//	case 3:
				//		cam.rotate(90 * Coord3D<> {0, -1, 0});
				//		break;
				//	case 4:
				//		cam.rotate({0, 0, 0});
				//		break;
				//	case 5:
				//		cam.rotate(180 * Coord3D<>{0, -1, 0});
				//		break;
				//	}
			}

			//get shadow view
			glCullFace(GL_FRONT);
			m_shadows->enable();
			cam.render(shad, models, false, true);
			m_shadows->disable();
			glCullFace(GL_BACK);


			//render shadow 
			glViewport(0, 0, WindowCreator::getScreenWidth(), WindowCreator::getScreenHeight());

			to->moveDepthToBackBuffer(to->getDepthWidth(), to->getDepthHeight());
			to->enable();
			glClear(GL_DEPTH_BUFFER_BIT);

			Shader* m_shadowCompShader = ResourceManager::getShader("shaders/Main Buffer.vtsh", "shaders/Shadow Composite.fmsh");
			
			glm::vec4 dir(0, 0, 1, 1.0f);
			dir = m_lights[a]->getWorldRotationMatrix() * (m_lights[a]->getLocalRotationMatrix() * dir);

			m_shadowCompShader->enable();
			m_shadowCompShader->sendUniform("uScene", 0);
			m_shadowCompShader->sendUniform("uPosition", 1);
			m_shadowCompShader->sendUniform("uNormOP", 2);
			m_shadowCompShader->sendUniform("uShadow", 3);
			m_shadowCompShader->sendUniform("uLightDirection",dir);
			m_shadowCompShader->sendUniform("uLightViewProj", lsm);
			m_shadowCompShader->sendUniform("uShadowEnable", true);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, to->getColorHandle(0));
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gBuff->getColorHandle(0));
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gBuff->getColorHandle(2));
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, m_shadows->getDepthHandle());

			FrameBuffer::drawFullScreenQuad();

			//un-bind textures
			for(int b = 0; b < 4; ++b)
				glActiveTexture(GL_TEXTURE0 + b),
				glBindTexture(GL_TEXTURE_2D, GL_NONE);

			m_shadowCompShader->disable();
			to->disable();
			to->takeFromBackBufferDepth(to->getDepthWidth(), to->getDepthHeight());

		}

	}
}

void LightManager::update()
{

	if(m_framebuffer)
		m_framebuffer->enable();
	m_shader->enable();

	for(unsigned a = 0; a < m_lights.size(); a++)
	{

		if(!m_lights[a]->lightEnable)
		{
			m_shader->sendUniform("LightEnable", false);
			continue;
		}
		m_shader->sendUniform("LightEnable", true);

		m_shader->sendUniform("AmbiantEnable", m_lights[a]->ambiantEnable);
		m_shader->sendUniform("DiffuseEnable", m_lights[a]->diffuseEnable);
		m_shader->sendUniform("SpecularEnable", m_lights[a]->specularEnable);



		glm::vec4 pos(0, 0, 0, 1.0f);
		glm::vec4 dir(0, 0, 1, 1.0f);


		pos = m_lights[a]->getWorldTranslationMatrix() * (m_lights[a]->getLocalTranslationMatrix() * glm::vec4(m_lights[a]->getPosition().toVec3(), 1));
		//	pos = m_cam->getProjectionMatrix() * pos;
		//	pos.z *= -1;
			//if(pos.w)
			//	pos /= pos.w;
			//else
			//	pos = {0,0,0,1};


			//pos = glm::normalize(pos);
		m_shader->sendUniform("LightPosition", pos);


		dir = m_lights[a]->getWorldRotationMatrix() * (m_lights[a]->getLocalRotationMatrix() * dir);
		dir = normalize(dir);

		pos = {0, 0, 0, 1.0f};
		pos = inverse(m_cam->getViewMatrix()) * pos;
		m_shader->sendUniform("uViewPos", pos);

		m_shader->sendUniform("LightType", (int)m_lights[a]->type);

		m_shader->sendUniform("LightAmbient", Coord3D<>{m_lights[a]->ambient[0] / 255.0f, m_lights[a]->ambient[1] / 255.0f, m_lights[a]->ambient[2] / 255.0f});

		m_shader->sendUniform("LightDiffuse", Coord3D<>{ m_lights[a]->diffuse[0] / 255.0f, m_lights[a]->diffuse[1] / 255.0f, m_lights[a]->diffuse[2] / 255.0f});

		m_shader->sendUniform("LightSpecular", Coord3D<>{ m_lights[a]->specular[0] / 255.0f, m_lights[a]->specular[1] / 255.0f, m_lights[a]->specular[2] / 255.0f});

		m_shader->sendUniform("LightDirection", Coord3D<>(dir));

		m_shader->sendUniform("LightAngleConstraint", m_lights[a]->angleConstraint);

		m_shader->sendUniform("LightSpecularExponent", m_lights[a]->specularExponent);

		m_shader->sendUniform("Attenuation_Constant", m_lights[a]->attenuationConst);

		m_shader->sendUniform("Attenuation_Linear", m_lights[a]->attenuationLinear);

		m_shader->sendUniform("Attenuation_Quadratic", m_lights[a]->attenuationQuadratic);

		m_shader->sendUniform("toonActiveSpec", m_lights[a]->rampActiveDiff);
		m_shader->sendUniform("toonActiveDiff", m_lights[a]->rampActiveSpec);


		FrameBuffer::drawFullScreenQuad();
	}
	m_shader->sendUniform("LightEnable", false);

	m_shader->disable();
	if(m_framebuffer)
		m_framebuffer->disable();

}

void LightManager::clear()
{
	m_lights.clear();
}

void LightManager::setFramebuffer(FrameBuffer* buff)
{
	m_framebuffer = buff;
}

void Light::setLightType(TYPE a_type)
{
	*(TYPE*)&this->type = a_type;
}

void Light::setAmbient(ColourRGBA ambi)
{
	ambient = ambi;
}

void Light::setDiffuse(ColourRGBA diff)
{
	diffuse = diff;
}

void Light::setSpecular(ColourRGBA spec)
{
	specular = spec;
}

void Light::setRamp(Texture2D* ramp)
{
	m_ramp = ramp;
}


void Light::setSpecularExponent(float specEx)
{
	*(float*)&specularExponent = specEx;
}

void Light::setAttenuationConstant(float attenConst)
{
	*(float*)&attenuationConst = attenConst;
}

void Light::setAttenuationLinear(float attenLinear)
{
	*(float*)&attenuationLinear = attenLinear;
}

void Light::setAttenuationQuadratic(float attenQuad)
{
	*(float*)&attenuationQuadratic = attenQuad;
}

void Light::setAngleConstraint(float angle)
{
	angleConstraint = float(M_PI / 180 * angle);
}

void Light::enableLight(bool enable)
{
	lightEnable = enable;
}

void Light::enableAmbiant(bool enable)
{
	ambiantEnable = enable;
}

void Light::enableDiffuse(bool enable)
{
	diffuseEnable = enable;
}

void Light::enableSpecular(bool enable)
{
	specularEnable = enable;
}
