#include "Shader.h"

std::unordered_map<GLuint, Shader*> Shader::m_shaders;
bool Shader::uniformErrors = true;
//GLuint *Shader::m_programs = new GLuint[0], *Shader::m_attribs = new GLuint[0], Shader::m_num;

Shader::Shader():m_vtsh(""), m_vtPath(""), m_fmPath(""), m_attribNum(0),
m_enabled(false), m_programID(0),
m_vertID(0),
m_fragID(0)
{
	glewInit();
}

Shader::~Shader()
{
	if(m_programID)
		glDeleteProgram(m_programID);
}

void Shader::refresh()
{
	for(auto& a : m_shaders)
		a.second->create(a.second->m_vtPath, a.second->m_fmPath);
}

void Shader::enableUniformErrors(bool err)
{
	uniformErrors = err;
}

bool Shader::create(const std::string& vertFilePath, const std::string& fragFilePath)
{
	if(compileShaders(vertFilePath, fragFilePath))
		return	linkShaders();
	return false;
}

bool Shader::create(const std::string& vertFilePath, const std::string& fragFilePath, const std::string& geoFilePath)
{
	if(compileShaders(vertFilePath, fragFilePath, geoFilePath))
		return	linkShaders();
	return false;
}

void Shader::createDefault()
{
	//create("Shaders/error.vert", "Shaders/error.frag");


	if(m_programID)
		glDeleteProgram(m_programID);

	if(m_fragID)
		glDeleteShader(m_fragID);
	if(m_vertID)
		glDeleteShader(m_vertID);

	m_programID = m_vertID = m_fragID = 0;


	m_programID = glCreateProgram();
	m_vertID = glCreateShader(GL_VERTEX_SHADER);
	m_fragID = glCreateShader(GL_FRAGMENT_SHADER);


	std::string tmpFileContent =
		"#version 420\n"

		"uniform mat4 uLocalModel;\n"
		"uniform mat4 uWorldModel;\n"
		"uniform mat4 uView;\n"
		"uniform mat4 uProj;\n"

		"layout(location = 0) in vec3 in_vert;\n"
		"layout(location = 1) in vec2 in_uv;\n"
		"layout(location = 2) in vec3 in_normal;\n"

		"out vec2 texcoord;\n"
		"out vec3 norm;\n"
		"out vec4 pos;\n"

		"void main()\n"
		"{\n"
		"texcoord = in_uv;\n"
		"norm = mat3(uView) * mat3(uWorldModel) * (mat3(uLocalModel) * in_normal);\n"

		"pos = (uView * uWorldModel * (uLocalModel * vec4(in_vert, 1.0f)));\n"

		"gl_Position = uProj * pos;\n"
		"}\n"
		;

	m_vtsh = tmpFileContent;
	cstring tmp = tmpFileContent.c_str();

	glShaderSource(m_vertID, 1, &tmp, nullptr);
	glCompileShader(m_vertID);

	GLint success = 0;
	glGetShaderiv(m_vertID, GL_COMPILE_STATUS, &success);

	if(success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(m_vertID, GL_INFO_LOG_LENGTH, &maxLength);

		char* errorLog = new char[maxLength];
		glGetShaderInfoLog(m_vertID, maxLength, &maxLength, errorLog);

		printf("Error in default Vert Shader:\n");
		printf("%s\n\n", errorLog);

		delete[] errorLog;
	}

	tmpFileContent =
		"#version 420\n"

		"in vec2 texcoord;\n"
		"in vec3 norm;\n"
		"in vec4 pos;\n"

		"out vec4 outColor1;\n"
		"out vec4 outColor2;\n"
		"out vec4 outColor3;\n"

		"void main()\n"
		"{\n"
		"outColor1 = vec4(0, 0, 0, 1);\n"
		"outColor2 = vec4(0, 0, 0, 1);\n"
		"outColor3 = vec4(1, 0, 1, 1);\n"
		"}\n"
		;
	tmp = tmpFileContent.c_str();

	glShaderSource(m_fragID, 1, &tmp, nullptr);
	glCompileShader(m_fragID);

	success = 0;
	glGetShaderiv(m_fragID, GL_COMPILE_STATUS, &success);

	if(success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(m_fragID, GL_INFO_LOG_LENGTH, &maxLength);

		char* errorLog = new char[maxLength];
		glGetShaderInfoLog(m_fragID, maxLength, &maxLength, errorLog);

		printf("Error in default Frag Shader:\n");
		printf("%s\n\n", errorLog);

		delete[] errorLog;
	}

	linkShaders();
	puts("Created default shader\n");

	//delete tmpFileContent;
}

bool Shader::compileShaders(const std::string& vertFilePath, const std::string& fragFilePath)
{
	m_vtPath = vertFilePath;
	m_fmPath = fragFilePath;

	cDir((char*)m_vtPath.c_str());
	cDir((char*)m_fmPath.c_str());

	glDeleteProgram(m_programID);

	m_programID = glCreateProgram();
	m_vertID = glCreateShader(GL_VERTEX_SHADER);
	m_fragID = glCreateShader(GL_FRAGMENT_SHADER);

	if(!compileShader(VERT_SHADER, vertFilePath, m_vertID))return false;
	if(!compileShader(FRAG_SHADER, fragFilePath, m_fragID))return false;

	return true;
}

bool Shader::compileShaders(const std::string& vertFilePath, const std::string& fragFilePath, const std::string& geoFilePath)
{
	m_vtPath = vertFilePath;
	m_fmPath = fragFilePath;

	glDeleteProgram(m_programID);

	m_programID = glCreateProgram();
	m_vertID = glCreateShader(GL_VERTEX_SHADER);
	m_fragID = glCreateShader(GL_FRAGMENT_SHADER);
	m_geomID = glCreateShader(GL_GEOMETRY_SHADER);

	if(!compileShader(VERT_SHADER, vertFilePath, m_vertID))return false;
	if(!compileShader(FRAG_SHADER, fragFilePath, m_fragID))return false;
	if(!compileShader(GEOM_SHADER, geoFilePath, m_geomID))return false;



	return true;
}

bool Shader::linkShaders()
{
	glAttachShader(m_programID, m_vertID);

	if(m_geomID)
		glAttachShader(m_programID, m_geomID);

	glAttachShader(m_programID, m_fragID);

	glLinkProgram(m_programID);

	GLint isLinked = 0;
	glGetProgramiv(m_programID, GL_LINK_STATUS, (int*)&isLinked);

	if(isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* infoLog = new char[maxLength];
		glGetProgramInfoLog(m_programID, maxLength, &maxLength, infoLog);

		// We don't need the program anymore.
		glDeleteProgram(m_programID);

		// Don't leak shaders either.
		glDeleteShader(m_vertID);
		glDeleteShader(m_fragID);

		//Reset to zero.
		m_programID = m_vertID = m_fragID = 0;

		// Use the infoLog as you see fit.
		printf("Could not link shaders \"%s\" and \"%s\"\n",m_vtPath.c_str(),m_fmPath.c_str());
		puts(infoLog);
		puts("\n");
		delete[] infoLog;

		createDefault();
		//system("pause");
		// In this simple program, we'll just leave
		return false;
	}

	glDetachShader(m_programID, m_vertID);
	glDetachShader(m_programID, m_fragID);
	if(m_geomID)
		glDetachShader(m_programID, m_geomID);

	glDeleteShader(m_vertID);
	glDeleteShader(m_fragID);
	if(m_geomID)
		glDeleteShader(m_geomID);


	m_vertID = m_fragID = 0;

	m_shaders[m_programID] = this;

	return true;
}

void Shader::addAtribute(const std::string attributeName, short m_index)
{
	glBindAttribLocation(m_programID, m_index, attributeName.c_str());
	linkShaders();
}

GLint Shader::getAttribLocation(const std::string attributeName)
{
	return glGetAttribLocation(m_programID, attributeName.c_str());
}

GLint Shader::getUniformLocation(cstring uniform)
{
	GLint uni = glGetUniformLocation(m_programID, uniform);
	if(uniformErrors)
		if(uni < 0)
		{
			printf("could not find uniform variable \"%s\" \n"
				   "within \"%s\" or \"%s\" shaders\n\n", uniform, m_vtPath.substr(m_vtPath.find_last_of('/') + 1).c_str(), m_fmPath.substr(m_fmPath.find_last_of('/') + 1).c_str());
		}

	return uni;
}

void Shader::sendUniform(cstring uniform, glm::mat4 val, bool transpose)
{
	GLint uni = getUniformLocation(uniform);
	glUniformMatrix4fv(uni, 1, transpose, &val[0][0]);
}

void Shader::sendUniform(cstring uniform, glm::vec4 val)
{
	GLint uni = getUniformLocation(uniform);
	glUniform4fv(uni, 1, &val[0]);
}

void Shader::sendUniform(cstring uniform, Coord3D<> val)
{
	GLint uni = getUniformLocation(uniform);
	glUniform3fv(uni, 1, &val.x);
}

void Shader::sendUniform(cstring uniform, float x, float y, float z)
{
	GLint uni = getUniformLocation(uniform);
	glUniform3f(uni, x, y, z);
}

void Shader::sendUniform(cstring uniform, float x, float y, float z, float w)
{
	GLint uni = getUniformLocation(uniform);
	glUniform4f(uni, x, y, z, w);
}

void Shader::sendUniform(cstring uniform, float val)
{
	GLint uni = getUniformLocation(uniform);
	glUniform1f(uni, val);
}

void Shader::sendUniform(cstring uniform, unsigned val)
{
	GLint uni = getUniformLocation(uniform);
	glUniform1ui(uni, val);
}

void Shader::sendUniform(cstring uniform, int val)
{
	GLint uni = getUniformLocation(uniform);
	glUniform1i(uni, val);
}

void Shader::sendUniform(cstring uniform, bool val)
{
	sendUniform(uniform, (int)val);
}


void Shader::enable()
{
	glUseProgram(m_programID);
}

void Shader::disable()
{

	glUseProgram(0);

}

bool Shader::compileShader(Shaders shadType, const std::string filePath, GLuint id)
{
	std::ifstream
		shader(filePath);

	std::string fileContent = "", line;

	while(std::getline(shader, line))
		fileContent += line + '\n';
	shader.close();

	if(shadType == VERT_SHADER) //stores vtsh
		m_vtsh = fileContent;

	cstring tmpFileContent = fileContent.c_str();
	glShaderSource(id, 1, &tmpFileContent, nullptr);
	glCompileShader(id);
	//delete tmpFileContent;

	GLint success = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if(success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

		char* errorLog = new char[maxLength];
		glGetShaderInfoLog(id, maxLength, &maxLength, errorLog);

		printf("error in file: %s\n", filePath.c_str());
		printf("%s\n\n", errorLog);

		delete[] errorLog;
		createDefault();
		return false;
	}
	return true;
}

void Shader::findAtributes()
{
	unsigned short count = 0;
	for(unsigned a = 0; a < m_vtsh.size(); a++)
	{
		if(m_vtsh.substr(a, 3) == "in ")
		{
			count++;
			a += 3;
		}
	}
	m_attribNum = count;
}

