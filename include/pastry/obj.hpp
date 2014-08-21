#pragma once

#include <Eigen/Dense>
#include <string>
#include <vector>

namespace pastry
{
	struct ObjVertexIndices
	{
		int v, uv, vn;
	};

	struct ObjFace
	{
		ObjVertexIndices a, b, c;
	};

	struct ObjMesh
	{
		std::string name;
		std::vector<Eigen::Vector3f> v;
		std::vector<Eigen::Vector2f> uv;
		std::vector<Eigen::Vector3f> vn;
		std::vector<ObjFace> f;
	};

	ObjMesh LoadObjMesh(const std::string& fn);

	struct ObjVertex
	{
		float vx, vy, vz;
		float u, v;
		float nx, ny, nz;
	};

	std::vector<ObjVertex> GetVertexData(const ObjMesh& mesh);

}