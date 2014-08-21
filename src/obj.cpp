#include <pastry/obj.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

namespace pastry
{
	ObjVertexIndices ParseObjVertexIndices(const std::string& str)
	{
		static std::vector<std::string> tokens;
		boost::split(tokens, str, boost::is_any_of("/"));
		return ObjVertexIndices{
			tokens[0].empty() ? 0 : boost::lexical_cast<int>(tokens[0]),
			tokens[1].empty() ? 0 : boost::lexical_cast<int>(tokens[1]),
			tokens[2].empty() ? 0 : boost::lexical_cast<int>(tokens[2])};
	}

	ObjMesh LoadImpl(const std::string& fn)
	{
		std::ifstream ifs(fn);
		ObjMesh mesh;
		mesh.v.emplace_back(Eigen::Vector3f::Zero());
		mesh.uv.emplace_back(Eigen::Vector2f::Zero());
		mesh.vn.emplace_back(Eigen::Vector3f::Zero());
		std::string line;
		std::vector<std::string> tokens;
		std::vector<std::string> tokens2;
		while(std::getline(ifs, line)) {
			boost::split(tokens, line, boost::is_any_of(" "));
			const std::string& head = tokens[0];
			if(head == "#") {
				continue;
			}
			else if(head == "o") {
				mesh.name = tokens[1];
			}
			else if(head == "v") {
				mesh.v.emplace_back(
					boost::lexical_cast<float>(tokens[1]),
					boost::lexical_cast<float>(tokens[2]),
					boost::lexical_cast<float>(tokens[3]));
			}
			else if(head == "uv") {
				mesh.uv.emplace_back(
					boost::lexical_cast<float>(tokens[1]),
					boost::lexical_cast<float>(tokens[2]));
			}
			else if(head == "vn") {
				mesh.vn.emplace_back(
					boost::lexical_cast<float>(tokens[1]),
					boost::lexical_cast<float>(tokens[2]),
					boost::lexical_cast<float>(tokens[3]));
			}
			else if(head == "f") {
				mesh.f.push_back({
					ParseObjVertexIndices(tokens[1]),
					ParseObjVertexIndices(tokens[2]),
					ParseObjVertexIndices(tokens[3])});
			}
			else {
				std::cerr << "Unknown line in obj file starting with '" << head << "'" << std::endl;
			}
		}
		std::cout << "Loaded ObjMesh { "
			<< "name: " << mesh.name
			<< ", v: " << mesh.v.size()
			<< ", uv: " << mesh.uv.size()
			<< ", vn: " << mesh.vn.size()
			<< ", f: " << mesh.f.size()
			<< "}" << std::endl;
		return mesh;
	}

	class ObjLoader
	{
	public:
		ObjMesh Load(const std::string& fn)
		{
			auto it = cache_.find(fn);
			if(it == cache_.end()) {
				ObjMesh mesh = LoadImpl(fn);
				cache_[fn] = mesh;
				return mesh;
			}
			else {
				return it->second;
			}
		}

	private:
		std::map<std::string,ObjMesh> cache_;
	};

	ObjMesh LoadObjMesh(const std::string& fn)
	{
		static ObjLoader s_loader;
		return s_loader.Load(fn);
	}

	ObjVertex GetObjVertex(const ObjMesh& mesh, const ObjVertexIndices& v)
	{
		return {
			mesh.v[v.v][0], mesh.v[v.v][1], mesh.v[v.v][2],
			mesh.uv[v.uv][0], mesh.uv[v.uv][1],
			mesh.vn[v.vn][0], mesh.vn[v.vn][1], mesh.vn[v.vn][2]
		};
	}

	std::vector<ObjVertex> GetVertexData(const ObjMesh& mesh)
	{
		std::vector<ObjVertex> result;
		result.reserve(mesh.f.size() * 3 * 8);
		for(const auto& f : mesh.f) {
			result.push_back(GetObjVertex(mesh, f.a));
			result.push_back(GetObjVertex(mesh, f.c));
			result.push_back(GetObjVertex(mesh, f.b));
		}
		return result;
	}
}
