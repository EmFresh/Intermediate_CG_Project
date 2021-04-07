#include "MeshLoader.h"
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;


std::vector<std::shared_ptr<Mesh>> MeshLoader::m_meshes;
std::vector<std::pair<std::string, std::vector<Texture2D>>> MeshLoader::m_textures;

std::string substr(cstring str, cstring find)
{
	char tmp[CHAR_BUFF_SIZE];
	unsigned count = (unsigned)strlen(str);
	for(; count >= 0; --count)
		if(strstr(str + count, find))
			break;

	memset(tmp, 0, CHAR_BUFF_SIZE);
	memmove_s(tmp, CHAR_BUFF_SIZE, str, count + 1);
	return std::string(tmp);
}

std::vector<std::shared_ptr<Mesh>> MeshLoader::loadMesh(std::string path)
{
	if(!fs::exists("Models/BIN"))
		system("mkdir \"Models/BIN\"");

	cleanup();
	if(!load(path))return std::vector<std::shared_ptr<Mesh>>();

	for(auto& a : m_meshes)
		a->init();


	//for(auto& a : m_indicieData)
	//	a.second.clear();
	//m_indicieData.clear();

	return m_meshes;
}
#include <chrono>
#include <iomanip>
bool MeshLoader::load(std::string path)
{

	if(!strstr(path.c_str(), ".obj"))return false;

#pragma region Local Variables
	//std::vector < std::pair<std::string, std::vector<Coord3D<unsigned>[3]>>> faces;
	FILE* bin = nullptr;
	cDir((char*)path.c_str());

	//path.insert(path.begin(), '\"');
	//path.insert(path.end(), '\"');

	std::vector<Vec3>verts;
	std::vector<UV>uvs;
	std::vector<Vec3>norms;

	std::map<Indicie, GLuint> indicieMap;
	unsigned indicieCount = 0;

	bool initFace = true;
#pragma endregion
	std::string binPath = (path.substr(0, path.find('/') + 1) + "BIN") + path.substr(path.find_last_of('/'), path.find_first_of('.') - path.find_last_of('/') + 1) + "bin";
	auto lasttime = fs::last_write_time(binPath);
	auto lasttime2 = fs::last_write_time(path);
	if(!fs::exists(binPath) || (lasttime < lasttime2))
	{
		//	puts("Load from File\n");

	#pragma region Open Meshes 
		FILE* f;

		fopen_s(&f, path.c_str(), "r");
		if(!f)
		{
			printf("unknown file\n");
			return false;
		}

		loadMaterials(path.c_str());


		char inputBuff[CHAR_BUFF_SIZE];

		char* MeshCheck = nullptr;
		while(MeshCheck = fgets(inputBuff, CHAR_BUFF_SIZE, f),
			  //this part takes out the '\n' from the string
			  (inputBuff == nullptr ? "" : (inputBuff[strlen(inputBuff) - 1] = (inputBuff[strlen(inputBuff) - 1] == '\n' ? ' ' : inputBuff[strlen(inputBuff) - 1]), inputBuff)),
			  MeshCheck)
		{
			//checks for comments
			if(strchr(inputBuff, '#'))
				memset(strchr(inputBuff, '#'), '\0', sizeof(char));
			if(strstr(inputBuff, "mtllib"))
				//IDK what this is
				continue;
			if(strstr(inputBuff, "usemtl "))
			{

				char str[CHAR_BUFF_SIZE];
				sscanf_s(inputBuff, "usemtl %s", str, CHAR_BUFF_SIZE);
				m_meshes.back()->matNames.push_back(str);

				for(auto& a : m_textures)
					if(strcmp(str, a.first.c_str()) == 0)
					{
						auto& tmp = a.second;//refernce original
						m_meshes.back()->getTextures().insert(m_meshes.back()->getTextures().end(), tmp.begin(), tmp.end());
						m_meshes.back()->getReplaceTex().insert(m_meshes.back()->getReplaceTex().end(), tmp.size(), 0);
					}

				//	indicieMap.clear();
				//	indicieCount = 0;

			}

			else if(strstr(inputBuff, "vt "))
			{
				//UV data

				UV tmp;
				sscanf_s(inputBuff, "vt %f %f", &tmp.u, &tmp.v);
				uvs.push_back(tmp);
			}

			else if(strstr(inputBuff, "vn "))
			{
				//Normal data
				Vec3 tmp;
				sscanf_s(inputBuff, "vn %f %f %f", &tmp.x, &tmp.y, &tmp.z);
				norms.push_back(tmp);
			}

			else if(strstr(inputBuff, "o "))
			{
				char str[CHAR_BUFF_SIZE];
				sscanf_s(inputBuff, "o %s", str, CHAR_BUFF_SIZE);

				//object
				m_meshes.push_back(std::shared_ptr<Mesh>(new Mesh));
				m_meshes.back()->meshName = str;
				initFace = true;
				indicieMap.clear();
				indicieCount = 0;
			}

			else if(strstr(inputBuff, "s "))
				continue;
			else if(strstr(inputBuff, "f "))//Collect Face Data
			{
				//Face data

				Indicie tmp[3];

				char check = 0;
				unsigned counter = 0, count = 0;
				while(bool(check = inputBuff[counter++]))
					count += (check == '/');

				count = unsigned((float)count * .5f);
				std::string	faceTmp[2][2]
				{{ " %d/%d/%d"," %*d/%*d/%*d" },
					{ " %d//%d"," %*d//%*d" }};

				std::vector<std::string > format = std::vector<std::string>(count);
				std::string formatStr;
				std::function<void(int)> reformat = [&](int type)
				{
					for(unsigned a = 0; a < count; a++)
						if(a < 3)//read 3 indicies
							format[a] = faceTmp[type][0];
						else
							format[a] = faceTmp[type][1];
				};
				short type = 0;
				reformat(type);
				formatStr = "f";

				for(unsigned a = 0; a < count; a++)
					formatStr += format[a];



				if(9 != sscanf_s(inputBuff, formatStr.c_str(),
				   //Coord         UV        Normal
				   &tmp[0][0], &tmp[0][1], &tmp[0][2],
				   &tmp[1][0], &tmp[1][1], &tmp[1][2],
				   &tmp[2][0], &tmp[2][1], &tmp[2][2]))
				{
					reformat(++type);
					formatStr = "f";
					for(unsigned a = 0; a < count; a++)
						formatStr += format[a];

					sscanf_s(inputBuff, formatStr.c_str(),
							 //Coord        Normal
							 &tmp[0][0], &tmp[0][2],
							 &tmp[1][0], &tmp[1][2],
							 &tmp[2][0], &tmp[2][2]);
				}


				for(unsigned b = 0; b < 3; ++b)
				{
					bool inuv = (bool)tmp[b].uv;
					tmp[b].correct();
					auto thing = indicieMap.find(tmp[b]);

					if(thing == indicieMap.end())//new indicie
					{
						indicieMap[tmp[b]] = indicieCount;
						Vertex3D tmp2;
						tmp2.coord = verts[tmp[b].coord];
						tmp2.norm = norms[tmp[b].norm];
						if(inuv)
							tmp2.uv = uvs[tmp[b].uv];

						m_meshes.back()->getUnpackedData().push_back(tmp2);
						m_meshes.back()->getIndicieData().push_back(indicieCount++);
					}
					else//reacouring indicie
						m_meshes.back()->getIndicieData().push_back(indicieMap[tmp[b]]);
				}

				//tmp.correct();
				//m_unpackedData.push_back(Vertex3D());
				//m_unpackedData.back().coord = verts[tmp[0][0]];
				//m_indicieData.back().second.push_back(tmp);

				for(unsigned a = 1; a < count - 2; a++)
				{
					formatStr = "f";
					swap(format[a], format[(a + 2)]);
					for(unsigned i = 0; i < count; i++)
						formatStr += format[i];
					if(type == 0)
						sscanf_s(inputBuff, formatStr.c_str(),
								 //Coord         UV        Normal
								 &tmp[0][0], &tmp[0][1], &tmp[0][2],
								 &tmp[1][0], &tmp[1][1], &tmp[1][2],
								 &tmp[2][0], &tmp[2][1], &tmp[2][2]);
					else
						sscanf_s(inputBuff, formatStr.c_str(),
								 //Coord       Normal
								 &tmp[0][0], &tmp[0][2],
								 &tmp[1][0], &tmp[1][2],
								 &tmp[2][0], &tmp[2][2]);
					for(unsigned b = 0; b < 3; ++b)
					{
						bool inuv = (bool)tmp[b].uv;

						tmp[b].correct();
						auto thing = indicieMap.find(tmp[b]);

						if(thing == indicieMap.end())//new indicie
						{
							indicieMap[tmp[b]] = indicieCount;
							Vertex3D tmp2;
							tmp2.coord = verts[tmp[b].coord];
							tmp2.norm = norms[tmp[b].norm];
							if(inuv)
								tmp2.uv = uvs[tmp[b].uv];

							m_meshes.back()->getUnpackedData().push_back(tmp2);
							m_meshes.back()->getIndicieData().push_back(indicieCount++);
						}
						else//reacouring indicie
							m_meshes.back()->getIndicieData().push_back(indicieMap[tmp[b]]);
					}


				}

			}

			else if(strstr(inputBuff, "v "))//Collects Vertex Data
			{
				//Vertex Data

				Vec3 tmp;
				sscanf_s(inputBuff, "v %f %f %f", &tmp.x, &tmp.y, &tmp.z);
				verts.push_back(tmp);
				if(initFace)
				{
					m_meshes.back()->front = m_meshes.back()->back =
						m_meshes.back()->left = m_meshes.back()->right =
						m_meshes.back()->top = m_meshes.back()->bottom = tmp;

					initFace = false;
				}
				else
				{
					m_meshes.back()->front = tmp.z > m_meshes.back()->front.z ? tmp : m_meshes.back()->front;
					m_meshes.back()->back = tmp.z < m_meshes.back()->back.z ? tmp : m_meshes.back()->back;
					m_meshes.back()->left = tmp.x < m_meshes.back()->left.x ? tmp : m_meshes.back()->left;
					m_meshes.back()->right = tmp.x > m_meshes.back()->right.x ? tmp : m_meshes.back()->right;
					m_meshes.back()->top = tmp.y > m_meshes.back()->top.y ? tmp : m_meshes.back()->top;
					m_meshes.back()->bottom = tmp.y < m_meshes.back()->bottom.y ? tmp : m_meshes.back()->bottom;
				}
			}
		}
		fclose(f);
	#pragma endregion

	#pragma region PACK DATA

		std::string tmper;
		//open bin file
		fopen_s(&bin, (tmper = ((path.substr(0, path.find('/') + 1) + "BIN") + path.substr(path.find_last_of('/'), path.find_first_of('.') - path.find_last_of('/') + 1) + "bin")).c_str(), "wb");

		unsigned meshSize = m_meshes.size(), dataSize = 0, writen = 0;

		//Ammount of Meshes (size of int)
		writen = fwrite(&meshSize, 1, sizeof(unsigned), bin);

		for(unsigned int a = 0; a < meshSize; a++)
		{
			//Mesh Name (size of name length + 1 then string)
			dataSize = m_meshes[a]->meshName.length() + 1;
			writen = fwrite(&dataSize, sizeof(unsigned), 1, bin);
			writen = fwrite(m_meshes[a]->meshName.data(), 1, dataSize, bin);

			//Amount of Vertecies (size of unsigned)
			dataSize = m_meshes[a]->getUnpackedData().size();
			writen = fwrite(&dataSize, sizeof(unsigned), 1, bin);

			//Store Vertex Data (size of vertex3D * vertex amount)
			writen = fwrite(m_meshes[a]->getUnpackedData().data(), 1, sizeof(Vertex3D) * dataSize, bin);

			//Amount of Indicies (size of unsigned)
			dataSize = m_meshes[a]->getIndicieData().size();
			writen = fwrite(&dataSize, sizeof(unsigned), 1, bin);

			//Indicie Data (size of unsigned * indice amount)
			writen = fwrite(m_meshes[a]->getIndicieData().data(), sizeof(unsigned) * dataSize, 1, bin);

			//Amount of Materials (size of unsigned)
			dataSize = m_meshes[a]->matNames.size();
			writen = fwrite(&dataSize, sizeof(unsigned), 1, bin);

			//Material Names
			for(auto& b : m_meshes[a]->matNames)
			{
				unsigned size = b.length() + 1;
				writen = fwrite(&size, 1, sizeof(unsigned), bin);
				writen = fwrite(b.data(), 1, size, bin);
			}
			m_meshes[a]->matNames.clear();

			//Bounds Data 
			writen = fwrite(&m_meshes[a]->top/*first bound*/, sizeof(Vec3), 6, bin);


		}
	#pragma endregion

	}
	else
	{
		//	puts("Load from BIN\n");

		loadMaterials(path.c_str());

	#pragma region UNPACK DATA

		std::string tmper;
		//open bin file
		fopen_s(&bin, (tmper = ((path.substr(0, path.find('/') + 1) + "BIN") + path.substr(path.find_last_of('/'), path.find_first_of('.') - path.find_last_of('/') + 1) + "bin")).c_str(), "rb");

		unsigned meshSize = 0, dataSize = 0;

		//ammount of meshes
		dataSize = fread(&meshSize, sizeof(char), sizeof(unsigned), bin);

		//m_meshes[0]->getUnpackedData();

		for(unsigned int a = 0; a < meshSize; a++)
		{
			m_meshes.push_back(std::shared_ptr<Mesh>(new Mesh));

			//Mesh Name (size of name length + 1 then string)
			fread(&dataSize, sizeof(unsigned), 1, bin);
			cstring name = new char[dataSize];
			fread((void*)name, dataSize, 1, bin);
			m_meshes[a]->meshName = name;

			//Amount of Vertecies (size of unsigned)
			fread(&dataSize, sizeof(unsigned), 1, bin);
			m_meshes[a]->getUnpackedData().resize(dataSize);

			//Store Vertex Data (size of vertex3D * vertex amount)
			fread(m_meshes[a]->getUnpackedData().data(), 1, sizeof(Vertex3D) * dataSize, bin);

			//Amount of Indicies (size of unsigned)
			fread(&dataSize, sizeof(unsigned), 1, bin);
			m_meshes[a]->getIndicieData().resize(dataSize);

			//Indicie Data (size of unsigned * indice amount)
			fread(m_meshes[a]->getIndicieData().data(), sizeof(unsigned) * dataSize, 1, bin);

			//Amount of Materials (size of unsigned)
			fread(&dataSize, sizeof(unsigned), 1, bin);
			m_meshes[a]->matNames.resize(dataSize);

			//Material Names
			for(unsigned b = 0; b < dataSize; ++b)
			{
				static unsigned size;
				fread(&size, 1, sizeof(unsigned), bin);
				cstring str = new char[size];
				fread((void*)str, 1, size, bin);

				for(auto& c : m_textures)
					if(strcmp(str, c.first.c_str()) == 0)
					{
						auto& tmp = c.second;//refernce original
						m_meshes[a]->getTextures().insert(m_meshes[a]->getTextures().end(), tmp.begin(), tmp.end());
						m_meshes[a]->getReplaceTex().insert(m_meshes[a]->getReplaceTex().end(), c.second.size(), 0);
					}

				delete str;
			}

			//bounds data
			fread(&m_meshes[a]->top, sizeof(Vec3), 6, bin);


		}
	#pragma endregion

	}

	if(bin)
		fclose(bin);

	return true;
}

void MeshLoader::cleanup()
{
	//for(auto a : m_meshes)
	//	if(a)
	//		delete a;
	m_meshes.clear();
	m_textures.clear();
}

void MeshLoader::loadMaterials(cstring path)
{
	FILE* f;
	cDir((char*)path);
	fopen_s(&f, path, "r");

	if(!f)
	{
		printf("unknown material\n");
		return;
	}

	char str[CHAR_BUFF_SIZE];
	char* MeshCheck;
	std::string tmpDir;
	while(MeshCheck = fgets(str, CHAR_BUFF_SIZE, f),
		  //this part takes out the '\n' from the string
		  (str == nullptr ? "" : (str[strlen(str) - 1] = (str[strlen(str) - 1] == '\n' ? '\0' : str[strlen(str) - 1]), str)),
		  MeshCheck)
		if(strstr(str, "mtllib"))
		{
			//find material
			char str2[CHAR_BUFF_SIZE];
			memcpy_s(str2, CHAR_BUFF_SIZE, str + 7, CHAR_BUFF_SIZE - 7);

			//find material location
			char tmp[CHAR_BUFF_SIZE];
			unsigned count = unsigned(strrchr(path, '/') - path) + 1;
			memset(tmp, 0, CHAR_BUFF_SIZE);
			memmove_s(tmp, CHAR_BUFF_SIZE, path, count);
			tmpDir = std::string(tmp) + std::string(str2);

			//set material path
			path = tmpDir.c_str();
			break;
		}

	fclose(f);

	fopen_s(&f, path, "r");


	if(f)
		while(MeshCheck = fgets(str, CHAR_BUFF_SIZE, f),
			  //this part takes out the '\n' from the string
			  (str == nullptr ? "" : (str[strlen(str) - 1] = (str[strlen(str) - 1] == '\n' ? '\0' : str[strlen(str) - 1]), str)),
			  MeshCheck)
		{

			if(strchr(str, '#'))
				memset(strchr(str, '#'), '\0', sizeof(char));

			if(strstr(str, "newmtl "))
			{
				char str2[CHAR_BUFF_SIZE];
				sscanf_s(str, "newmtl %s", str2, CHAR_BUFF_SIZE);
				m_textures.push_back({std::string(str2), std::vector<Texture2D>()});
				//m_replaceTex.push_back(std::vector<GLuint>());
				if(strstr(str, "None"))
					return;
			}

			else if(strstr(str, "illum "))
			{
				continue;
			}

			else if(strstr(str, "map_Kd "))
			{
				char str2[CHAR_BUFF_SIZE];
				//sscanf_s(str, "map_Kd %s", &str2, (unsigned)_countof(str2));
				memcpy_s(str2, CHAR_BUFF_SIZE, str + 7, CHAR_BUFF_SIZE - 7);

				//path.resizeDepth(path.c_str());
				cDir(str2);
				std::string tmpStr(substr(path, "/") + str2);
				m_textures.back().second.push_back(ResourceManager::getTexture2D(tmpStr.c_str()));
				m_textures.back().second.back().type = TEXTURE_TYPE2D::DIFFUSE;
				//m_replaceTex.back().push_back(0);

			}

			else if(strstr(str, "map_Ks "))
			{
				char str2[CHAR_BUFF_SIZE];
				memcpy_s(str2, CHAR_BUFF_SIZE, str + 7, CHAR_BUFF_SIZE - 7);

				//path.resizeDepth(path.c_str());
				cDir(str2);
				std::string tmpStr(substr(path, "/") + str2);
				m_textures.back().second.push_back(ResourceManager::getTexture2D(tmpStr.c_str()));
				m_textures.back().second.back().type = TEXTURE_TYPE2D::SPECULAR;
				//m_replaceTex.back().push_back(0);

			}

			else if(strstr(str, "Ns "))
				continue;

			else if(strstr(str, "Ka "))
			{
				float a[3];
				sscanf_s(str, "Ka %f %f %f", &a[0], &a[1], &a[2]);
				for(auto& b : m_textures.back().second)
					b.colour.a = (GLubyte)(255 * a[0] * a[1] * a[2]);
			}

			else if(strstr(str, "Kd "))
			{
				float r, g, b;
				sscanf_s(str, "Kd %f %f %f", &r, &g, &b);
				for(auto& a : m_textures.back().second)
					if(a.type == TEXTURE_TYPE2D::DIFFUSE)
						a.colour.set(r, g, b);
			}

			else if(strstr(str, "Ks "))
			{
				float r, g, b;
				sscanf_s(str, "Ks %f %f %f", &r, &g, &b);
				for(auto& a : m_textures.back().second)
					if(a.type == TEXTURE_TYPE2D::DIFFUSE)
						a.colour.set(r, g, b);
				continue;
			}

			else if(strstr(str, "Ke "))
				continue;

			else if(strstr(str, "Ni "))
				continue;

			else if(strstr(str, "d "))
			{
				continue;
			}
		}

	if(f)
		fclose(f);
}