#include "Model.h" 
#include <ctime>

Model::Model(Model& model, cstring tag):
	Transformer(model, "MODEL"),
	m_tag(tag)
{
	create(model, tag);

}

Model::Model(primitiveMesh* mesh, cstring tag):
	Transformer("MODEL"),
	m_tag(tag)
{
	create(mesh, tag);
}

Model::Model(cstring path, cstring tag):
	Transformer("MODEL"),
	m_tag(tag)
{
	create(path, tag);
}

Model::~Model()
{
#if _DEBUG
	printf("Deleted Model\n");
#endif // _DEBUG
	if(!m_copy)
		meshCleanUp();
}

void Model::create(Model& model, cstring tag)
{
	*this = model;
	if(strlen(tag))
		m_tag = tag;
	m_copy = true;
	//boundingBoxInit();
	boundingBoxUpdate();
}

void Model::create(primitiveMesh* mesh, cstring tag)
{
	m_meshes.push_back(new Mesh());
	if(strlen(tag))
		m_tag = tag;
	if(m_meshes[0]->loadPrimitive(mesh))
	{
		m_shaderBB = ResourceManager::getShader("Shaders/BoundingBox.vtsh", "Shaders/BoundingBox.fmsh");


		float top = m_meshes[0]->top.y,
			bottom = m_meshes[0]->bottom.y,
			left = m_meshes[0]->left.x,
			right = m_meshes[0]->right.x,
			front = m_meshes[0]->front.z,
			back = m_meshes[0]->back.z;

		(m_topLeftBack = {left,top,back}),
			(m_topRightBack = {right,top,back}),
			(m_topLeftFront = {left,top,front}),
			(m_topRightFront = {right,top,front}),
			(m_bottomLeftBack = {left,bottom,back}),
			(m_bottomRightBack = {right,bottom,back}),
			(m_bottomLeftFront = {left,bottom,front}),
			(m_bottomRightFront = {right,bottom,front});


		boundingBoxInit();
		boundingBoxUpdate();
	}
}

void Model::create(cstring path, cstring tag)
{
	if(strlen(tag))
		m_tag = tag;
	if(loadModel(path))
	{
		m_shaderBB = ResourceManager::getShader("Shaders/BoundingBox.vtsh", "Shaders/BoundingBox.fmsh");


		float top = m_meshes[0]->top.y,
			bottom = m_meshes[0]->bottom.y,
			left = m_meshes[0]->left.x,
			right = m_meshes[0]->right.x,
			front = m_meshes[0]->front.z,
			back = m_meshes[0]->back.z;

		for(auto& a : m_meshes)
		{
			top = top < a->top.y ? a->top.y : top,
				bottom = bottom>a->bottom.y ? a->bottom.y : bottom,
				left = left > a->left.x ? a->left.x : left,
				right = right < a->right.x ? a->right.x : right,
				front = front< a->front.z ? a->front.z : front,
				back = back > a->back.z ? a->back.z : back;
		}

		(m_topLeftBack = {left,top,back}),
			(m_topRightBack = {right,top,back}),
			(m_topLeftFront = {left,top,front}),
			(m_topRightFront = {right,top,front}),
			(m_bottomLeftBack = {left,bottom,back}),
			(m_bottomRightBack = {right,bottom,back}),
			(m_bottomLeftFront = {left,bottom,front}),
			(m_bottomRightFront = {right,bottom,front});


		boundingBoxInit();
		boundingBoxUpdate();
	}
}



/// - Collision Function - ///

bool Model::collision2D(Model* box2, Coord3D<>ignore)
{
	return collision2D(this, box2, ignore);
}

bool Model::collision2D(Model* box1, Model* box2, Coord3D<>RPos)
{

	RPos.normalize();
	RPos = (Coord3D<>{1, 1, 1} - RPos);

	RPos = (box1->m_center - box2->m_center) * RPos;
	Coord3D<> AxisX{1,0,0}, AxisY{0,1,0}, AxisZ{0,0,1};

	glm::mat4
		* rotLocal1 = (glm::mat4*)&box1->getLocalRotationMatrix(),
		* rotLocal2 = (glm::mat4*)&box2->getLocalRotationMatrix(),

		* rotWorld1 = (glm::mat4*)&box1->getWorldRotationMatrix(),
		* rotWorld2 = (glm::mat4*)&box2->getWorldRotationMatrix();

	static Coord3D<> AxisX1, AxisY1, AxisZ1, AxisX2, AxisY2, AxisZ2;
	AxisX1 = (*rotWorld1 * (*rotLocal1 * glm::vec4(*(glm::vec3*)&AxisX, 1)));
	AxisY1 = (*rotWorld1 * (*rotLocal1 * glm::vec4(*(glm::vec3*)&AxisY, 1)));
	AxisZ1 = (*rotWorld1 * (*rotLocal1 * glm::vec4(*(glm::vec3*)&AxisZ, 1)));

	AxisX2 = (*rotWorld2 * (*rotLocal2 * glm::vec4(*(glm::vec3*)&AxisX, 1)));
	AxisY2 = (*rotWorld2 * (*rotLocal2 * glm::vec4(*(glm::vec3*)&AxisY, 1)));
	AxisZ2 = (*rotWorld2 * (*rotLocal2 * glm::vec4(*(glm::vec3*)&AxisZ, 1)));

	return !(
		getSeparatingPlane(RPos, AxisX1, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisY1, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisZ1, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisX2, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisY2, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisZ2, *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisX1, AxisX2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisX1, AxisY2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisX1, AxisZ2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisY1, AxisX2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisY1, AxisY2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisY1, AxisZ2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisZ1, AxisX2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisZ1, AxisY2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisZ1, AxisZ2), *box1, *box2));

}


///~ 3D Collision Function ~///

bool Model::collision3D(Model* k)
{
	return collision3D(this, k);
}

bool Model::collision3D(Model* box1, Model* box2)
{
	static Coord3D<> RPos;
	RPos = box1->m_center - box2->m_center;
	Coord3D<> AxisX{1,0,0}, AxisY{0,1,0}, AxisZ{0,0,1};

	glm::mat4
		* rotLocal1 = (glm::mat4*)&box1->getLocalRotationMatrix(),
		* rotLocal2 = (glm::mat4*)&box2->getLocalRotationMatrix(),

		* rotWorld1 = (glm::mat4*)&box1->getWorldRotationMatrix(),
		* rotWorld2 = (glm::mat4*)&box2->getWorldRotationMatrix();

	static Coord3D<> AxisX1, AxisY1, AxisZ1, AxisX2, AxisY2, AxisZ2;
	AxisX1 = (*rotWorld1 * (*rotLocal1 * glm::vec4(*(glm::vec3*)&AxisX, 1)));
	AxisY1 = (*rotWorld1 * (*rotLocal1 * glm::vec4(*(glm::vec3*)&AxisY, 1)));
	AxisZ1 = (*rotWorld1 * (*rotLocal1 * glm::vec4(*(glm::vec3*)&AxisZ, 1)));

	AxisX2 = (*rotWorld2 * (*rotLocal2 * glm::vec4(*(glm::vec3*)&AxisX, 1)));
	AxisY2 = (*rotWorld2 * (*rotLocal2 * glm::vec4(*(glm::vec3*)&AxisY, 1)));
	AxisZ2 = (*rotWorld2 * (*rotLocal2 * glm::vec4(*(glm::vec3*)&AxisZ, 1)));

	return !(
		getSeparatingPlane(RPos, AxisX1, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisY1, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisZ1, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisX2, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisY2, *box1, *box2) ||
		getSeparatingPlane(RPos, AxisZ2, *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisX1, AxisX2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisX1, AxisY2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisX1, AxisZ2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisY1, AxisX2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisY1, AxisY2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisY1, AxisZ2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisZ1, AxisX2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisZ1, AxisY2), *box1, *box2) ||
		getSeparatingPlane(RPos, Coord3D<>::crossProduct(AxisZ1, AxisZ2), *box1, *box2));
}

bool Model::getSeparatingPlane(const Coord3D<>& RPos, const Coord3D<>& plane, Model& box1, Model& box2)
{

	//RPos;
	Coord3D<> AxisX{1,0,0}, AxisY{0,1,0}, AxisZ{0,0,1};

	glm::mat4
		* trans1 = (glm::mat4*)&box1.getLocalRotationMatrix(),
		* trans2 = (glm::mat4*)&box2.getLocalRotationMatrix();

	Coord3D<> AxisX1 = (*trans1 * glm::vec4(*(glm::vec3*)&AxisX, 1));
	Coord3D<> AxisY1 = (*trans1 * glm::vec4(*(glm::vec3*)&AxisY, 1));
	Coord3D<> AxisZ1 = (*trans1 * glm::vec4(*(glm::vec3*)&AxisZ, 1));

	Coord3D<> AxisX2 = (*trans2 * glm::vec4(*(glm::vec3*)&AxisX, 1));
	Coord3D<> AxisY2 = (*trans2 * glm::vec4(*(glm::vec3*)&AxisY, 1));
	Coord3D<> AxisZ2 = (*trans2 * glm::vec4(*(glm::vec3*)&AxisZ, 1));

	//float w, h;
	//glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);

	return (fabs(Coord3D<>::dotProduct(RPos, plane)) >
			(
			fabs(Coord3D<>::dotProduct((AxisX1 * (box1.m_width / 2)), plane)) +
			fabs(Coord3D<>::dotProduct((AxisY1 * (box1.m_height / 2)), plane)) +
			fabs(Coord3D<>::dotProduct((AxisZ1 * (box1.m_depth / 2)), plane)) +

			fabs(Coord3D<>::dotProduct((AxisX2 * (box2.m_width / 2)), plane)) +
			fabs(Coord3D<>::dotProduct((AxisY2 * (box2.m_height / 2)), plane)) +
			fabs(Coord3D<>::dotProduct((AxisZ2 * (box2.m_depth / 2)), plane))
			));
}

void Model::render(Shader& shader, Camera* cam)
{
	float colour[4]{(float)m_colour.r / 255,(float)m_colour.g / 255,(float)m_colour.b / 255,(float)m_colour.a / 255};
	m_camera = cam;
	m_shader = &shader;
	shader.enable();

	shader.sendUniform("uLocalModel", getLocalTransformation());
	shader.sendUniform("uWorldModel", getWorldTransformation());
	shader.sendUniform("colourMod", reclass(glm::vec4, colour));
	shader.sendUniform("flip", true);

	shader.disable();

	if(m_animations[m_animation])
		m_animations[m_animation]->update(&shader, this);

	// update the position of the object
	boundingBoxUpdate();

	if(m_render)
	{
		if(m_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//render the meshes
		for(auto& a : m_meshes)
			a->render(shader, m_useTex);

		if(m_enableBB)
			drawBoundingBox();

		static Shader* shader2;
		//render child meshes
		for(auto& a : getChildren())
			if(a->getCompType() == "MODEL")
				reclass(Model*, a)->render(shader, cam);
			else if(a->getCompType() == "TEXT")
				shader2 = ResourceManager::getShader("shaders/freetype.vtsh", "shaders/freetype.fmsh"),
				reclass(Text*, a)->render(*shader2, cam);


		resetUpdated();
		if(m_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Model::drawBoundingBox()
{

	m_shaderBB->enable();
	m_shaderBB->sendUniform("colourMod", {0, 0, 0, 1});
	//m_shaderBB->disable();

	glBindVertexArray(m_BBVaoID);
	glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
	glBindVertexArray(0);

	m_shaderBB->disable();
}

void Model::setColour(float r, float g, float b, float a)
{
	m_colour.set((GLubyte)(255 * r), (GLubyte)(255 * g), (GLubyte)(255 * b), (GLubyte)(255 * a));
}

void Model::setColour(float r, float g, float b)
{
	m_colour.set((GLubyte)(255 * r), (GLubyte)(255 * g), (GLubyte)(255 * b));
}

void Model::setColour(ColourRGBA colour)
{
	m_colour = colour;
}

ColourRGBA Model::getColour()
{
	return m_colour;
}

bool Model::loadModel(cstring path)
{
	for(auto& a : m_meshes)
		delete a;
	m_meshes.clear();
	m_meshes = MeshLoader::loadMesh(path);
	return !!m_meshes.size();
}

void Model::enableBoundingBox(bool enable)
{
	m_enableBB = enable;
}

void Model::addAnimation(std::string tag, Animation* animation)
{
	m_animations[tag] = animation;
}

void Model::editVerts(Model* first, Model* second)
{
	for(unsigned a = 0; a < first->m_meshes.size(); a++)
		m_meshes[a]->editVerts(first->m_meshes[a], second->m_meshes[a]);

}

float Model::getWidth()
{
	return m_width;
}

float Model::getHeight()
{
	return m_height;
}

float Model::getDepth()
{

	return m_depth;
}

Coord3D<> Model::getSize()
{

	return {m_width,m_height,m_depth};
}

Coord3D<> Model::getCenter()
{

	return m_center;
}

void Model::boundingBoxUpdate()
{
	if(m_enableBB && m_shaderBB)
	{
		m_shaderBB->enable();
		m_shaderBB->sendUniform("uLocalModel", getLocalTransformation());
		m_shaderBB->sendUniform("uWorldModel", getWorldTransformation());
		glUniformMatrix4fv(m_shaderBB->getUniformLocation("uView"), 1, GL_FALSE, &(m_camera->getViewMatrix()[0][0]));
		glUniformMatrix4fv(m_shaderBB->getUniformLocation("uProj"), 1, GL_FALSE, &(m_camera->getProjectionMatrix()[0][0]));
		m_shaderBB->disable();
	}

	//float
	//	right = m_meshes[0]->right.x,
	//	left = m_meshes[0]->left.x,
	//	top = m_meshes[0]->top.y,
	//	bottom = m_meshes[0]->bottom.y,
	//	front = m_meshes[0]->front.z,
	//	back = m_meshes[0]->back.z;
	//
	//for(auto& a : m_meshes)
	//	right = right < a->right.x ? a->right.x : right,
	//	left = left > a->left.x ? a->left.x : left,
	//	top = top < a->top.y ? a->top.y : top,
	//	bottom = bottom>a->bottom.y ? a->bottom.y : bottom,
	//	front = front< a->front.z ? a->front.z : front,
	//	back = back > a->back.z ? a->back.z : back;



	std::vector<glm::vec4> bounds =
	{
	{*(glm::vec3*)&m_bottomRightBack,1},
	{*(glm::vec3*)&m_bottomLeftBack,1},
	{*(glm::vec3*)&m_topLeftBack,1},
	{*(glm::vec3*)&m_bottomLeftBack,1},
	{*(glm::vec3*)&m_topLeftFront,1},
	{*(glm::vec3*)&m_topLeftBack,1}
	};


	for(auto& a : bounds)
		a = getWorldScaleMatrix() * (getLocalScaleMatrix() * a);


	m_width = abs(bounds[0].x - bounds[1].x);
	m_height = abs(bounds[2].y - bounds[3].y);
	m_depth = abs(bounds[4].z - bounds[5].z);


	bounds =
	{
	{*(glm::vec3*)&m_bottomRightBack,1},
	{*(glm::vec3*)&m_bottomLeftBack,1},
	{*(glm::vec3*)&m_topLeftBack,1},
	{*(glm::vec3*)&m_bottomLeftBack,1},
	{*(glm::vec3*)&m_topLeftFront,1},
	{*(glm::vec3*)&m_topLeftBack,1}
	};


	for(auto& a : bounds)
		a = getWorldTranslationMatrix() * (getLocalTransformation() * a);

	m_center =
		(Coord3D<>(
		bounds[0].x + bounds[1].x,
		bounds[2].y + bounds[3].y,
		bounds[4].z + bounds[5].z) / 2);
}

Animation* Model::getAnimation(cstring tag)
{
	return m_animations[tag];
}

Animation* Model::getCurrentAnimation()
{
	return m_animations[m_animation];
}

void Model::setAnimation(cstring tag)
{
	m_animation = tag;
}

void Model::addMesh(Mesh* mesh)
{
	m_meshes.push_back(mesh);
}

Mesh* Model::getMesh(const unsigned index)
{
	return m_meshes[index];
}

Shader* Model::getShader()
{
	return m_shader;
}

void Model::replaceTexture(int mesh, int index, GLuint texID)
{
	m_meshes[mesh]->replaceTexture(index, texID);
}

void Model::enableTexture(bool enable)
{
	m_useTex = enable;
}

bool Model::isTextureEnabled()
{
	return m_useTex;
}

void Model::setToRender(bool render)
{
	m_render = render;
}

void Model::setTransparent(bool trans)
{
	m_transparent = trans;
}

void Model::setWireframe(bool wire)
{
	m_wireframe = wire;
}

bool Model::isTransparent()
{
	return m_transparent;
}

void Model::boundingBoxInit()
{
	if(!m_BBVaoID)
		glGenVertexArrays(1, &m_BBVaoID);
	if(!m_BBVboID)
		glGenBuffers(1, &m_BBVboID);



	Vertex3D
		topLeftBack{m_topLeftBack},
		topRightBack{m_topRightBack},
		topLeftFront{m_topLeftFront},
		topRightFront{m_topRightFront},
		bottomLeftBack{m_bottomLeftBack},
		bottomRightBack{m_bottomRightBack},
		bottomLeftFront{m_bottomLeftFront},
		bottomRightFront{m_bottomRightFront};


	Vertex3D tmp[12 * 3]{
		//top
		topLeftBack,topRightBack,topRightFront,
		topLeftBack,topRightFront,topLeftFront,
		//bottom
		 bottomRightFront, bottomRightBack,bottomLeftBack,
		bottomLeftFront,bottomRightFront,bottomLeftBack,
		//front
		topLeftFront,topRightFront,bottomRightFront,
		topLeftFront,bottomRightFront,bottomLeftFront,
		//back
		bottomRightBack, topRightBack, topLeftBack,
		bottomLeftBack, bottomRightBack, topLeftBack,
		//left
		topLeftBack,topLeftFront,bottomLeftFront,
		topLeftBack,bottomLeftFront,bottomLeftBack,
		//right
		bottomRightFront,topRightFront,topRightBack,
		bottomRightBack,bottomRightFront,topRightBack
	};

	memcpy_s(m_vertBBDat, sizeof(Vertex3D) * 12 * 3, tmp, sizeof(Vertex3D) * 12 * 3);

	glBindVertexArray(m_BBVaoID);
	glBindBuffer(GL_ARRAY_BUFFER, m_BBVboID);
	glBufferData(GL_ARRAY_BUFFER, 12 * 3 * sizeof(Vertex3D), m_vertBBDat, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	//vertex     atributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, coord));

	//UV         atributes
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, uv));

	//normal     atributes
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, norm));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Model::print()
{
	printf(
		"Tag: %s\n"
		"Width: %f\n"
		"Height: %f\n"
		"Depth: %f\n"
		"Center: (%f, %f, %f)\n"
		, m_tag, m_width, m_height, m_depth, m_center.x, m_center.y, m_center.z);
}

std::vector<Coord3D<>> Model::getBounds()
{
	return std::vector<Coord3D<>>{m_topLeftBack,
		m_topRightBack,
		m_topLeftFront,
		m_topRightFront,
		m_bottomLeftBack,
		m_bottomRightBack,
		m_bottomLeftFront,
		m_bottomRightFront};
}

void Model::meshCleanUp()
{
	for(auto& a : m_meshes)
		if(a)
			delete a;
	m_meshes.clear();
}

