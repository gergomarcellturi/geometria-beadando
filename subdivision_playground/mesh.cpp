#include "mesh.h"

Vertex::Vertex()
{
    edge = nullptr;
}

Vertex::Vertex(glm::vec3 p)
{
    edge = nullptr; pos = p;
}

Vertex::Vertex(Vertex const& v)
{
    this->pos = v.pos; this->edge = v.edge;
}

Mesh::~Mesh()
{
    deleteMesh();
}

void Mesh::deleteFaces()
{
    Face* f;
    while (!faces.empty()) {
        f = faces.back();
        faces.pop_back();
        delete f;
    }
}

void Mesh::deleteEdges()
{
    HalfEdge* e;
    while (!halfedges.empty()) {
        e = halfedges.back();
        halfedges.pop_back();
        delete e;
    }
}

void Mesh::deleteVertices()
{
    Vertex* v;
    while (!vertices.empty()) {
        v = vertices.back();
        vertices.pop_back();
        delete v;
    }
}

void Mesh::deleteMesh()
{
    deleteFaces();
    deleteEdges();
    deleteVertices();
}

void Mesh::subdivision(std::string path, int steps)
{
    deleteMesh();
    loadFile(path);
    initMesh();

    for (int i = 0; i < steps; i++)
    {
        butterflyGenerateEdgePoints();
        butterflyConnectNewMesh();
    }

    assembleVertices();
    calculateNormals();
}

void Mesh::assembleVertices()
{
    vertexTriagnleList.clear();
    for (std::vector<Face*>::iterator f = faces.begin();  f != faces.end(); f++)
    {
        HalfEdge* e = (*f)->edge;
        do
        {
            vertexTriagnleList.push_back(e->start->pos);
            e = e->next;
        } while (e != (*f)->edge);
    }
}

void Mesh::calculateNormals()
{
    normals.clear();

    for (int i = 0; i < vertexTriagnleList.size(); i += 3)
    {
        const glm::vec3 a = vertexTriagnleList[i + 2] - vertexTriagnleList[i];
        const glm::vec3 b = vertexTriagnleList[i + 1] - vertexTriagnleList[i];
        const glm::vec3 normal = glm::normalize(glm::cross(a, b)) * glm::vec3(-1, -1, -1);
        normals.push_back(normal);
        normals.push_back(normal);
        normals.push_back(normal);
    }
}

void Mesh::draw()
{
    glDrawArrays(GL_TRIANGLES, 0, vertexTriagnleList.size());
}

bool Mesh::loadFile(const std::string& filename)
{
    std::string::size_type idx;
    idx = filename.rfind('.');

    if (idx != std::string::npos) {
        std::string extension = filename.substr(idx + 1);
        if (extension == "obj") {
            return loadObject(filename);
        }
        else {
            std::cerr << "ERROR: unable to load file " << filename
                << "  -- unknown extension." << std::endl;
            std::cerr << "Input only (.obj) files" << std::endl;
        }
    }
    return false;
}

bool Mesh::loadObject(const std::string& filename)
{
    std::ifstream ifs(filename.c_str());
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> temp_normals;
    std::vector<glm::vec3> temp_textures;

    if (ifs.is_open())
    {
        int t0[3], t1[3], t2[3], t3[3];
        float v0, v1, v2;
        std::string line;
        char temp[3];

        while (std::getline(ifs, line))
        {
            if (line[0] == 'v' && line[1] == ' ')
            {
                sscanf(line.c_str(), "%s %f %f %f", temp, &v0, &v1, &v2);
                glm::vec3 vector(v0, v1, v2);
                temp_vertices.push_back(vector);
                Vertex* v = new Vertex(vector);
                vertices.push_back(v);
            }
            else if (line[0] == 'v' && line[1] == 'n')
            {
                sscanf(line.c_str(), "%s %f %f %f", temp, &v0, &v1, &v2);
                glm::vec3 vector(v0, v1, v2);
                temp_normals.push_back(vector);
            }
            else if (line[0] == 'v' && line[1] == 't')
            {
                sscanf(line.c_str(), "%s %f %f %f", temp, &v0, &v1, &v2);
                glm::vec3 vector(v0, v1, v2);
                temp_textures.push_back(vector);
            }
            else if (line[0] == 'f')
            {
                t0[0] = t0[1] = t0[2] = t1[0] = t1[1] = t1[2] = t2[0] = t2[1] = t2[2] = t3[0] = t3[1] = t3[2] = -1;

                if (line.find("//") != std::string::npos)
                {
                    sscanf(line.c_str(), "%s %d//%d %d//%d %d//%d %d//%d",
                        temp, &t0[0], &t0[2],
                        &t1[0], &t1[2],
                        &t2[0], &t2[2],
                        &t3[0], &t3[2]);

                }
                else if (line.find("/") != std::string::npos)
                {
                    int total = sscanf(line.c_str(), "%s %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                        temp, &t0[0], &t0[1], &t0[2],
                        &t1[0], &t1[1], &t1[2],
                        &t2[0], &t2[1], &t2[2],
                        &t3[0], &t3[1], &t3[2]);

                    if (total < 10)
                    {
                        sscanf(line.c_str(), "%s %d/%d %d/%d %d/%d %d/%d", temp, &t0[0],
                            &t0[1], &t1[0], &t1[1], &t2[0], &t2[1], &t3[0], &t3[1]);
                    }

                }
                else
                {
                    sscanf(line.c_str(), "%s %d %d %d %d", temp, &t0[0], &t1[0], &t2[0], &t3[0]);
                }

                std::vector<int> face;
                face.push_back(t0[0] - 1);
                face.push_back(t1[0] - 1);
                face.push_back(t2[0] - 1);

                vertexIndices.push_back(face);
            }
        }
    }
    return true;
}

void Mesh::initMesh()
{
    std::vector<HalfEdge*> tempEdges;
    std::vector<Face*> tempFaces;
    std::vector<std::vector<int>>::iterator face;
    for (face = vertexIndices.begin(); face != vertexIndices.end(); face++) {
        std::vector<Vertex*> fvertices;
        for (int i = 0; i < face->size(); i++) {
            fvertices.push_back(vertices[(*face)[i]]);
        }
        makeFace(fvertices, tempEdges, tempFaces);
    }
    halfedges.clear();
    faces.clear();
    halfedges = tempEdges;
    faces = tempFaces;
}

void Mesh::makeFace(std::vector<Vertex*>& fvertices, std::vector<HalfEdge*>& tempEdges, std::vector<Face*>& tempFaces)
{
    Face* face = new Face();
    std::vector<HalfEdge*> edges;
    for (int i = 0; i < (int)fvertices.size(); i++)
    {
        edges.push_back(new HalfEdge());
    }

    int num = (int)fvertices.size();
    for (int i = 0; i < num; i++)
    {
        edges[i]->start = fvertices[i];
        edges[i]->face = face;
        tempEdges.push_back(edges[i]);
        edges[i]->next = edges[(i + 1) % num];
        edges[i]->prev = edges[((i + (num - 1)) % num)];
        if (fvertices[i]->edge == nullptr) {
            fvertices[i]->edge = edges[i];
        }
    }
    face->edge = edges[0];
    tempFaces.push_back(face);

    for (size_t i = 0; i < num; ++i) {
        HalfEdge* e = mostClockwise(fvertices[i]->edge);
        if (e && e->start->pos == edges[i]->next->start->pos
            && e->next->start->pos == edges[i]->start->pos) {
            edges[i]->pair = e;
            e->pair = edges[i];
        }
    }
}

HalfEdge* Mesh::mostClockwise(HalfEdge* edge)
{
    if (edge == nullptr) { return nullptr; }
    HalfEdge* e = edge->prev;
    while (e && e->pair) {
        e = e->pair->prev;
        if (e == edge->prev) { break; }
    }
    return e;
}

void Mesh::butterflyGenerateEdgePoints()
{
    std::vector<Vertex*> tempVertices;
    for (std::vector<HalfEdge*>::iterator i = halfedges.begin(); i < halfedges.end(); i++)
    {
        HalfEdge** edge = &(*i);

        if ((*edge)->edgePoint)
            continue;

        Vertex* edgePoint = new Vertex();

        if ((*edge)->pair == nullptr)
        {
            edgePoint->pos = ((*edge)->start->pos + (*edge)->next->start->pos) * 0.5f;
            (*edge)->edgePoint = edgePoint;
        }
        else if ((*edge)->pair)
        {

            HalfEdge* edge1 = (*edge);
            HalfEdge* edge2 = (*edge)->next;

            HalfEdge* edge3 = (*edge)->next->next;
            HalfEdge* edge4 = (*edge)->pair->next->next;

            glm::vec3 a1 = edge1->start->pos;
            glm::vec3 a2 = edge2->start->pos;

            glm::vec3 b1 = edge3->start->pos;
            glm::vec3 b2 = edge4->start->pos;

            glm::vec3 c1, c2, c3, c4;

            if (edge2->pair && edge3->pair && edge1->pair && edge1->pair->next->next->pair && edge1->pair->next->pair)
            {

                c2 = edge2->pair->next->next->start->pos;
                c1 = edge3->pair->next->next->start->pos;

                c3 = edge1->pair->next->next->pair->next->next->start->pos;
                c4 = edge1->pair->next->pair->next->next->start->pos;

                edgePoint->pos = (a1 + a2) * (1.0f / 2.0f) + (b1 + b2) * (1.0f / 8.0f) - (c1 + c2 + c3 + c4) * (1.0f / 16.0f);
            }
            else
            {
                edgePoint->pos = (a1 * 9.0f + a2 * 9.0f - b1 - b2) * (1.0f / 16.0f);
            }

            (*edge)->edgePoint = edgePoint;
            (*edge)->pair->edgePoint = (*edge)->edgePoint;
        }

        tempVertices.push_back((*edge)->edgePoint);
    }
    vertices.clear();
    vertices = tempVertices;
}

void Mesh::butterflyConnectNewMesh()
{
    std::vector<HalfEdge*> tempEdges;
    std::vector<Face*> tempFaces;

    for (std::vector<Face*>::iterator i = faces.begin();
        i != faces.end(); i++) {
        HalfEdge* edge = (*i)->edge;


        std::vector<Vertex*> f;
        Vertex** s1 = &(edge->next->edgePoint);
        Vertex** s2 = &(edge->next->next->edgePoint);
        Vertex** s3 = &(edge->edgePoint);
        f.push_back(*s1);
        f.push_back(*s2);
        f.push_back(*s3);

        makeFace(f, tempEdges, tempFaces);

        do
        {

            std::vector<Vertex*> fvertices;
            Vertex** v1 = &(edge->next->edgePoint);
            Vertex** v2 = &(edge->edgePoint);
            Vertex** v3 = &(edge->next->start);

            fvertices.push_back(*v1);
            fvertices.push_back(*v2);
            fvertices.push_back(*v3);
            makeFace(fvertices, tempEdges, tempFaces);

            edge = edge->next;
        } while (edge != (*i)->edge);
    }
    halfedges.clear();
    faces.clear();
    halfedges = tempEdges;
    faces = tempFaces;
}
