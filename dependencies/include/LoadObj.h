#ifndef LOADOBJ_H
#define LOADOBJ_H

class baseModel 
{
public:
	baseModel();
	~baseModel();

	void LoadFromFile(const char* fileName);
	std::vector<float> GetVertexData();
	int GetVertexCount();

private:
	struct Position { float x, y, z; };
	struct Color { float r, g, b; };
	struct texCoord { float x, y, z; };

	void LoadMaterialFile(const char* filename);
	bool StartWith(std::string& line, const char* text);
	void AddVertexData(int v1, int v2, int v3, const char* mtl, int n1, int n2, int n3, std::vector<Position> & vertices, std::vector<Normal>)
};
#endif