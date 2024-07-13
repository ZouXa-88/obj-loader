#include "trianglemesh.h"

// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	// -------------------------------------------------------
	// Add your initialization code here.
	vboId = 0;
	numTriangles = 0;
	numVertices = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	objExtent = glm::vec3(0.0f, 0.0f, 0.0f);
	// -------------------------------------------------------
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	// -------------------------------------------------------
	// Add your release code here.
	DeleteVertexBuffer();

	for (int i = 0; i < materials.size(); i++) {
		delete materials[i];
	}
	// -------------------------------------------------------
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string& filePath, const bool normalized)
{	
	// Parse the OBJ file.
	// ---------------------------------------------------------------------------
    // Add your implementation here (HW1 + read *.MTL).
	std::ifstream fileIn(filePath);
	if (!fileIn.is_open()) {
		std::cerr << "Can't open the file: \"" << filePath << "\"." << std::endl;
		return false;
	}

	std::string buf;
	int intData1, intData2, intData3;
	float floatData1, floatData2, floatData3;
	SubMesh* currentSubMesh = nullptr;
	while (std::getline(fileIn, buf)) {
		std::stringstream ss(buf);
		std::string type;
		ss >> type;
		if (type == "v") {
			ss >> floatData1 >> floatData2 >> floatData3;
			glm::vec3 newPosition(floatData1, floatData2, floatData3);
			positions.push_back(newPosition);
			UpdateExtentBound(newPosition);
			numVertices++;
		}
		else if (type == "vn") {
			ss >> floatData1 >> floatData2 >> floatData3;
			normals.push_back(glm::vec3(floatData1, floatData2, floatData3));
		}
		else if (type == "vt") {
			ss >> floatData1 >> floatData2;
			texcoords.push_back(glm::vec2(floatData1, floatData2));
		}
		else if (type == "mtllib") {
			std::string mtlFileName;
			ss >> mtlFileName;
			// Assume .obj and .mtl are in the same directory.
			std::string mtlFilePath = filePath.substr(0, filePath.rfind("\\") + 1) + mtlFileName;

			if (!LoadMtlFile(mtlFilePath)) {
				fileIn.close();
				return false;
			}
		}
		else if (type == "usemtl") {
			subMeshes.push_back(SubMesh());
			currentSubMesh = &(subMeshes.back());

			std::string materialName;
			ss >> materialName;

			currentSubMesh->material = GetMaterial(materialName);
			if (currentSubMesh->material == nullptr) {
				std::cerr << "Material: " + materialName + " not found." << std::endl;
				fileIn.close();
				return false;
			}
		}
		else if (type == "f") {
			if (currentSubMesh == nullptr) {
				std::cerr << "No material assignment." << std::endl;
				fileIn.close();
				return false;
			}

			std::vector<unsigned int> vertexIndices;
			std::string ptn;
			// read "P/T/N"
			while (ss >> ptn) {
				GetPTNData(ptn, intData1, intData2, intData3);
				PTNTuple newPTNTuple(
					std::abs(intData1) - 1,
					std::abs(intData2) - 1,
					std::abs(intData3) - 1
				);
				vertexIndices.push_back(GetVertexIndex(newPTNTuple));
			}
			// Cut into triangles.
			for (std::vector<unsigned int>::iterator it = vertexIndices.begin() + 1;
				it + 1 != vertexIndices.end(); it++) {
				currentSubMesh->vertexIndices.push_back(vertexIndices.front());
				currentSubMesh->vertexIndices.push_back(*it);
				currentSubMesh->vertexIndices.push_back(*(it + 1));
				numTriangles++;
			}
		}
	}
	// Build vertexPTNs.
	BuildVertices();
	fileIn.close();
    // ---------------------------------------------------------------------------

	// Update objCenter and objExtent data.
	SetCenter();
	SetExtent();

	// Normalize the geometry data.
	if (normalized) {
		// -----------------------------------------------------------------------
		// Add your normalization code here (HW1).
		Normalize();
		// -----------------------------------------------------------------------
	}
	return true;
}

// Show model information.
void TriangleMesh::ShowInfo()
{

	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Total " << subMeshes.size() << " subMeshes loaded" << std::endl;
	for (unsigned int i = 0; i < subMeshes.size(); ++i) {
		const SubMesh& g = subMeshes[i];
		std::cout << "SubMesh " << i << " with material: " << g.material->GetName() << std::endl;
		std::cout << "Num. triangles in the subMesh: " << g.vertexIndices.size() / 3 << std::endl;
	}
	std::cout << "Model Center: " << objCenter.x << ", " << objCenter.y << ", " << objCenter.z << std::endl;
	std::cout << "Model Extent: " << objExtent.x << " x " << objExtent.y << " x " << objExtent.z << std::endl;
}

void TriangleMesh::CreateVertexBuffer()
{
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPTN) * vertices.size(), &(vertices.front()), GL_STATIC_DRAW);

	for (int i = 0; i < subMeshes.size(); i++) {
		glGenBuffers(1, &(subMeshes[i].iboId));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMeshes[i].iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * subMeshes[i].vertexIndices.size(), &(subMeshes[i].vertexIndices.front()), GL_STATIC_DRAW);
	}
}

void TriangleMesh::UpdateExtentBound(const glm::vec3& newPosition)
{
	if (newPosition.x > extentBound.x_max)
		extentBound.x_max = newPosition.x;
	if (newPosition.x < extentBound.x_min)
		extentBound.x_min = newPosition.x;
	if (newPosition.y > extentBound.y_max)
		extentBound.y_max = newPosition.y;
	if (newPosition.y < extentBound.y_min)
		extentBound.y_min = newPosition.y;
	if (newPosition.z > extentBound.z_max)
		extentBound.z_max = newPosition.z;
	if (newPosition.z < extentBound.z_min)
		extentBound.z_min = newPosition.z;
}

void TriangleMesh::SetCenter()
{
	objCenter = glm::vec3(
		(float)(extentBound.x_max + extentBound.x_min) / 2,
		(float)(extentBound.y_max + extentBound.y_min) / 2,
		(float)(extentBound.z_max + extentBound.z_min) / 2
	);
}

void TriangleMesh::SetExtent()
{
	objExtent = glm::vec3(
		extentBound.x_max - extentBound.x_min,
		extentBound.y_max - extentBound.y_min,
		extentBound.z_max - extentBound.z_min
	);
}

void TriangleMesh::GetPTNData(std::string ptn, int& data1, int& data2, int& data3)
{
	int firstSlashPos = ptn.find("/");
	int secondSlashPos = ptn.rfind("/");
	data1 = std::stoi(ptn);
	data2 = std::stoi(ptn.substr(firstSlashPos + 1));
	data3 = std::stoi(ptn.substr(secondSlashPos + 1));
}

unsigned int TriangleMesh::GetVertexIndex(const PTNTuple& newTuple)
{
	if (existPTNs.find(newTuple) == existPTNs.end()) {
		return existPTNs[newTuple] = existPTNs.size();
	}
	return existPTNs[newTuple];
}

PhongMaterial* TriangleMesh::GetMaterial(const std::string& materialName)
{
	for (int i = 0; i < materials.size(); i++) {
		if (materials[i]->GetName() == materialName)
			return materials[i];
	}
	return nullptr;
}

bool TriangleMesh::LoadMtlFile(const std::string& filePath)
{
	std::ifstream fileIn(filePath);
	if (!fileIn.is_open()) {
		std::cerr << "Can't open mtl file \"" << filePath << "\"." << std::endl;
		return false;
	}
	
	PhongMaterial* currentMaterial = nullptr;
	std::string buf;
	float floatData1, floatData2, floatData3;
	while (fileIn >> buf) {
		// Look for new material declaration.
		if (buf == "newmtl") {
			PhongMaterial* material = new PhongMaterial();
			materials.push_back(material);
			currentMaterial = material;

			std::string materialName;
			fileIn >> materialName;
			currentMaterial->SetName(materialName);
		}
		else if (buf == "Ns") {
			fileIn >> floatData1;
			currentMaterial->SetNs(floatData1);
		}
		else if (buf == "Ka") {
			fileIn >> floatData1 >> floatData2 >> floatData3;
			currentMaterial->SetKa(glm::vec3(floatData1, floatData2, floatData3));
		}
		else if (buf == "Kd") {
			fileIn >> floatData1 >> floatData2 >> floatData3;
			currentMaterial->SetKd(glm::vec3(floatData1, floatData2, floatData3));
		}
		else if (buf == "Ks") {
			fileIn >> floatData1 >> floatData2 >> floatData3;
			currentMaterial->SetKs(glm::vec3(floatData1, floatData2, floatData3));
		}
		else if (buf == "map_Kd") {
			std::string texFileName;
			fileIn >> texFileName;
			std::string texFilePath = filePath.substr(0, filePath.rfind("\\") + 1) + texFileName;
			
			ImageTexture* imageTexture = new ImageTexture(texFilePath);
			currentMaterial->SetMapKd(imageTexture);
		}
	}
	
	fileIn.close();
	return true;
}

void TriangleMesh::BuildVertices()
{
	vertices.resize(existPTNs.size());
	for (auto existPTN : existPTNs) {
		vertices[existPTN.second] = VertexPTN(
			positions[std::get<0>(existPTN.first)],
			normals[std::get<2>(existPTN.first)],
			texcoords[std::get<1>(existPTN.first)]
		);
	}
}

void TriangleMesh::Normalize()
{
	const float scalar = (float)1 / std::max({ objExtent.x, objExtent.y, objExtent.z });
	glm::mat4 T = glm::translate(glm::mat4(1.0f), -objCenter);
	glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scalar, scalar, scalar));
	glm::mat4 normalizeMatrix = S * T;

	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].position = normalizeMatrix * glm::vec4(vertices[i].position, 1.0f);
	}
}

void TriangleMesh::DeleteVertexBuffer()
{
	glDeleteBuffers(1, &vboId);
	for (SubMesh subMesh : subMeshes) {
		glDeleteBuffers(1, &(subMesh.iboId));
	}
}