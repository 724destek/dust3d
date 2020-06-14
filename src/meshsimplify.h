#ifndef DUST3D_MESH_SIMPLIFY_H
#define DUST3D_MESH_SIMPLIFY_H
#include <QVector3D>
#include <vector>

#if 0
bool meshSimplifyFromQuadsUsingFQMS(const std::vector<QVector3D> &vertices, const std::vector<std::vector<size_t>> &quads,
    std::vector<QVector3D> *simplifiedVertices, std::vector<std::vector<size_t>> *simplifiedFaces);
#endif
    
bool meshSimplifyFromQuads(const std::vector<QVector3D> &vertices, const std::vector<std::vector<size_t>> &quads,
    std::vector<QVector3D> *simplifiedVertices, std::vector<std::vector<size_t>> *simplifiedFaces);

#endif