#pragma once
#include "Mesh.h"

class MeshLoader
{
public:
	static std::vector<std::shared_ptr<Mesh>> loadMesh(std::string path);

private:

	MeshLoader();

	//adds indices to the existing structure
	template<class _iter>
	void indiceAdder(_iter begin, _iter end, std::vector<Indicie>& origIndi, std::vector<Vertex3D>& origVerts, std::vector<Vertex3D> newVerts)
	{
		for(auto& i = begin; i < end; ++i)
		{
			//if the vertice dose not exist add to list
			if((std::find(origVerts.begin(), origVerts.end(), newVerts[*i])) == origVerts.end())
				origVerts.push_back(newVerts[*i]),
				origIndi.push_back(origVerts.size() - 1);

		}
	}

	static void loadMaterials(cstring path);

	static bool load(std::string path);

	static void cleanup();

	static std::vector<std::shared_ptr<Mesh>> m_meshes;
	static std::vector<std::pair<std::string, std::vector<Texture2D>>> m_textures;
	//static std::vector<std::pair<std::string, std::vector<unsigned>>> m_indicieData;
	//static std::vector<std::vector<Vertex3D>> m_unpackedData;
	//static std::vector<std::pair<std::string,std::vector<Texture2D>>> m_textures;
	//static std::vector<std::vector<GLuint>> m_replaceTex;

};

