#include <instant-meshes-api.h>
#include <cmath>
#include "remesher.h"

Remesher::Remesher()
{
}

Remesher::~Remesher()
{
}

void Remesher::setMesh(const std::vector<QVector3D> &vertices,
        const std::vector<std::vector<size_t>> &triangles)
{
    m_vertices = vertices;
    m_triangles = triangles;
}

const std::vector<QVector3D> &Remesher::getRemeshedVertices() const
{
    return m_remeshedVertices;
}

const std::vector<std::vector<size_t>> &Remesher::getRemeshedFaces() const
{
    return m_remeshedFaces;
}

const std::vector<std::pair<QUuid, QUuid>> &Remesher::getRemeshedVertexSources() const
{
    return m_remeshedVertexSources;
}

void Remesher::remesh()
{
    std::vector<Dust3D_InstantMeshesVertex> inputVertices(m_vertices.size());
    for (size_t i = 0; i < m_vertices.size(); ++i) {
        const auto &vertex = m_vertices[i];
        inputVertices[i] = Dust3D_InstantMeshesVertex {
            vertex.x(), vertex.y(), vertex.z()
        };
    }
    std::vector<Dust3D_InstantMeshesTriangle> inputTriangles;
    inputTriangles.reserve(m_triangles.size());
    for (size_t i = 0; i < m_triangles.size(); ++i) {
        const auto &triangle = m_triangles[i];
        if (triangle.size() != 3)
            continue;
        inputTriangles.push_back(Dust3D_InstantMeshesTriangle {{
            triangle[0],
            triangle[1],
            triangle[2]
        }});
    }
    const Dust3D_InstantMeshesVertex *resultVertices = nullptr;
    size_t nResultVertices = 0;
    const Dust3D_InstantMeshesTriangle *resultTriangles = nullptr;
    size_t nResultTriangles = 0;
    const Dust3D_InstantMeshesQuad *resultQuads = nullptr;
    size_t nResultQuads = 0;
    Dust3D_instantMeshesRemesh(inputVertices.data(), inputVertices.size(),
        inputTriangles.data(), inputTriangles.size(),
        (size_t)(inputVertices.size() * 1.0f),
        &resultVertices,
        &nResultVertices,
        &resultTriangles,
        &nResultTriangles,
        &resultQuads,
        &nResultQuads);
    m_remeshedVertices.resize(nResultVertices);
    memcpy(m_remeshedVertices.data(), resultVertices, sizeof(Dust3D_InstantMeshesVertex) * nResultVertices);
    m_remeshedFaces.reserve(nResultTriangles + nResultQuads);
    for (size_t i = 0; i < nResultTriangles; ++i) {
        const auto &source = resultTriangles[i];
        m_remeshedFaces.push_back(std::vector<size_t> {
            source.indices[0],
            source.indices[1],
            source.indices[2]
        });
    }
    for (size_t i = 0; i < nResultQuads; ++i) {
        const auto &source = resultQuads[i];
        m_remeshedFaces.push_back(std::vector<size_t> {
            source.indices[0],
            source.indices[1],
            source.indices[2],
            source.indices[3]
        });
    }
    resolveSources();
}

void Remesher::setNodes(const std::vector<std::pair<QVector3D, float>> &nodes,
        const std::vector<std::pair<QUuid, QUuid>> &sourceIds)
{
    m_nodes = nodes;
    m_sourceIds = sourceIds;
}

void Remesher::resolveSources()
{
    m_remeshedVertexSources.resize(m_remeshedVertices.size());
    std::vector<float> nodeRadius2(m_nodes.size());
    for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex) {
        nodeRadius2[nodeIndex] = std::pow(m_nodes[nodeIndex].second, 2);
    }
    for (size_t vertexIndex = 0; vertexIndex < m_remeshedVertices.size(); ++vertexIndex) {
        std::vector<std::pair<float, size_t>> matches;
        const auto &vertexPosition = m_remeshedVertices[vertexIndex];
        for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex) {
            const auto &nodePosition = m_nodes[nodeIndex].first;
            auto length2 = (vertexPosition - nodePosition).lengthSquared();
            if (length2 > nodeRadius2[nodeIndex])
                continue;
            matches.push_back(std::make_pair(length2, nodeIndex));
        }
        std::sort(matches.begin(), matches.end(), [](const std::pair<float, size_t> &first,
                const std::pair<float, size_t> &second) {
            return first.first < second.first;
        });
        if (matches.empty())
            continue;
        m_remeshedVertexSources[vertexIndex] = m_sourceIds[matches[0].second];
    }
}
