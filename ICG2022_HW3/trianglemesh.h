#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "headers.h"
#include "material.h"

// VertexPTN Declarations.
struct VertexPTN
{
	VertexPTN() {
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		normal = glm::vec3(0.0f, 1.0f, 0.0f);
		texcoord = glm::vec2(0.0f, 0.0f);
	}
	VertexPTN(glm::vec3 p, glm::vec3 n, glm::vec2 uv) {
		position = p;
		normal = n;
		texcoord = uv;
	}
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
};

struct ExtentBound
{
	ExtentBound() {
		x_max = -FLT_MAX;
		x_min = FLT_MAX;
		y_max = -FLT_MAX;
		y_min = FLT_MAX;
		z_max = -FLT_MAX;
		z_min = FLT_MAX;
	}
	float x_max;
	float x_min;
	float y_max;
	float y_min;
	float z_max;
	float z_min;
};

// SubMesh Declarations.
struct SubMesh
{
	SubMesh() {
		material = nullptr;
		iboId = 0;
	}
	PhongMaterial* material;
	GLuint iboId;
	std::vector<unsigned int> vertexIndices;
};


// Record "P/T/N" indices as tuple.
typedef std::tuple<unsigned int, unsigned int, unsigned int> PTNTuple;

// TriangleMesh Declarations.
class TriangleMesh
{
public:
	// TriangleMesh Public Methods.
	TriangleMesh();
	~TriangleMesh();
	
	// Load the model from an *.OBJ file.
	bool LoadFromFile(const std::string& filePath, const bool normalized = true);
	
	// Show model information.
	void ShowInfo();

	// -------------------------------------------------------
	// Feel free to add your methods or data here.
	int GetNumVertices() const { return numVertices; }
	int GetNumTriangles() const { return numTriangles; }
	int GetNumSubMeshes() const { return (int)subMeshes.size(); }

	SubMesh GetSubMesh(int index) const { return subMeshes[index]; }

	void CreateVertexBuffer();

	glm::vec3 GetObjCenter() const { return objCenter; }
	glm::vec3 GetObjExtent() const { return objExtent; }

	GLuint GetVboId() const { return vboId; }
	// -------------------------------------------------------

private:
	// -------------------------------------------------------
	// Feel free to add your methods or data here.
	// // TriangleMesh Private Data.
	GLuint vboId;

	std::vector<VertexPTN> vertices;
	// For supporting multiple materials per object, move to SubMesh.
	// GLuint iboId;
	// std::vector<unsigned int> vertexIndices;
	std::vector<SubMesh> subMeshes;

	int numVertices;
	int numTriangles;
	glm::vec3 objCenter;
	glm::vec3 objExtent;

	ExtentBound extentBound;
	// Record "P/T/N" and assign an individual index to it.
	std::map<PTNTuple, unsigned int> existPTNs;
	// Record all materials
	std::vector<PhongMaterial*> materials;

	// Record all position of vertices
	std::vector<glm::vec3> positions;
	// Record all texcoord of vertices
	std::vector<glm::vec2> texcoords;
	// Record all normal of vertices
	std::vector<glm::vec3> normals;


	void UpdateExtentBound(const glm::vec3&);
	void SetCenter();
	void SetExtent();

	// Convert "P/T/N" to three int data respectively.
	void GetPTNData(std::string, int&, int&, int&);
	// Add new PTNTuple (if not exist before) and return its assigned index.
	unsigned int GetVertexIndex(const PTNTuple&);
	// Get material by name.
	PhongMaterial* GetMaterial(const std::string&);
	// Load mtl file.
	bool LoadMtlFile(const std::string&);
	// Build VertexPTNs according to TuplePTNs.
	void BuildVertices();
	void Normalize();

	void DeleteVertexBuffer();
	// -------------------------------------------------------
};


#endif
