#include "LightManager.h"

#pragma region Static Variables
std::vector<Light*>LightManager::m_lights;
FrameBuffer* LightManager::m_framebuffer;
std::vector<std::vector<FrameBuffer*>>LightManager::m_shadows;

Shader* LightManager::m_shader;
Camera* LightManager::m_cam;
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

std::vector<FrameBuffer*> LightManager::shadowBuffers(unsigned w, unsigned h, std::unordered_map<void*, Model*>& models, unsigned index)
{
	
	if(m_lights[index]->type == Light::TYPE::POINT)
	{

		glViewport(0, 0, w, h);


		static Camera cam;
		cam.init({(float)w,(float)h,500}, Camera::ORTHOGRAPHIC);

		cam.translate(m_lights[index]->getPosition());

		for(int a = 0; a < 6; a++)
		{
			//	m_shadows[index][a]->resizeColour(0, w, h);
			m_shadows[index][a]->resizeDepth(w, h);

			m_shadows[index][a]->enable();
			//m_shader->enable();
			Shader* shad = ResourceManager::getShader("Shaders/ShadowDepth.vtsh", "Shaders/ShadowDepth.fmsh");
			shad->enable();
			shad->sendUniform("lightSpaceMatrix", m_cam->getProjectionMatrix() * glm::lookAt(m_lights[index]->getPosition().toVec3(),
							  glm::vec3(0.0f, 0.0f, 0.0f),
							  glm::vec3(0.0f, 1.0f, 0.0f)));



			switch(a)
			{
			case 0:
				cam.rotate(90 * Coord3D<>{1, 0, 0});
				break;
			case 1:
				cam.rotate(90 * Coord3D<>{-1, 0, 0});
				break;
			case 2:
				cam.rotate(90 * Coord3D<>{0, 1, 0});
				break;
			case 3:
				cam.rotate(90 * Coord3D<> {0, -1, 0});
				break;
			case 4:
				cam.rotate({0, 0, 0});
				break;
			case 5:
				cam.rotate(180 * Coord3D<>{0, -1, 0});

				break;
			default:
				break;
			}



			cam.render(shad, models);

			//Shader* shad = ResourceManager::getShader("Shaders/ShadowShader.vtsh", "Shaders/ShadowShader.frag");
			//shad->enable();
			//shad->sendUniform("depthMap", 0);
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, m_shadows[index][a]->getDepthHandle());
			//FrameBuffer::drawFullScreenQuad();
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, GL_NONE);
			FrameBuffer::disable();
		}

		//m_shader->disable();
	}
	return m_shadows[index];
}

void LightManager::update()
{
	

	if(m_framebuffer)
	{
		m_framebuffer->enable();
		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, m_framebuffer->getColorHandle(0));
	}
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

		m_shader->sendUniform("LightDiffuse", m_lights[a]->diffuse[0] / 255.0f, m_lights[a]->diffuse[1] / 255.0f, m_lights[a]->diffuse[2] / 255.0f);

		m_shader->sendUniform("LightSpecular", m_lights[a]->specular[0] / 255.0f, m_lights[a]->specular[1] / 255.0f, m_lights[a]->specular[2] / 255.0f);

		m_shader->sendUniform("LightDirection", dir);

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
	(ColourRGBA)ambient = ambi;
}

void Light::setDiffuse(ColourRGBA diff)
{
	(ColourRGBA)diffuse = diff;
}

void Light::setSpecular(ColourRGBA spec)
{
	(ColourRGBA)specular = spec;
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
