#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>

class Vertex;
class Face;
class HalfEdge;

class Vertex {
public:
    glm::vec3 pos;
    HalfEdge* edge;
    Vertex();
    Vertex(glm::vec3 p);
    Vertex(Vertex const& v);
    Vertex* newPoint;
};

class Face {
public:
    HalfEdge* edge;
    Vertex* facePoint;
};

class HalfEdge {
public:
    Vertex* start;
    HalfEdge* next;
    HalfEdge* prev;
    HalfEdge* pair;
    Face* face;
    Vertex* edgePoint;
};

class Mesh
{
public:
	~Mesh();
    std::vector<HalfEdge*> halfedges;
    std::vector<Vertex*> vertices;
    std::vector<Face*> faces;

    std::vector<glm::vec3> vertexTriagnleList;
    std::vector<glm::vec3> normals;

    void deleteFaces();
    void deleteEdges();
    void deleteVertices();
    void deleteMesh();

    void subdivision(std::string path, int steps);
    void assembleVertices();
    void calculateNormals();
    void draw();

    std::vector<std::vector<int>> vertexIndices;
    bool loadOBJ(const std::string& path);

    void initMesh();
    void makeFace(std::vector<Vertex*>& fvertices, std::vector<HalfEdge*>& tempEdges, std::vector<Face*>& tempFaces);
    HalfEdge* mostClockwise(HalfEdge* edge);


    void butterflyGenerateEdgePoints();
    void butterflyConnectNewMesh();
};
