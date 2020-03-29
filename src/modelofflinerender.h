#ifndef DUST3D_MODEL_OFFLINE_RENDER_H
#define DUST3D_MODEL_OFFLINE_RENDER_H
#include <QOffscreenSurface>
#include <QScreen>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QImage>
#include <QThread>
#include "modelshaderprogram.h"
#include "modelmeshbinder.h"
#include "meshloader.h"

class ModelOfflineRender : QOffscreenSurface
{
public:
    ModelOfflineRender(QScreen *targetScreen = Q_NULLPTR);
    ~ModelOfflineRender();
    void setRenderThread(QThread *thread);
    void updateMesh(MeshLoader *mesh);
    QImage toImage(const QSize &size);
private:
    QOpenGLContext *m_context;
    MeshLoader *m_mesh;
};

#endif
