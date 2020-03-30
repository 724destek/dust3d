#ifndef DUST3D_NORMAL_AND_DEPTH_MAPS_GENERATOR_H
#define DUST3D_NORMAL_AND_DEPTH_MAPS_GENERATOR_H
#include <QObject>
#include <QOpenGLTexture>
#include "modelwidget.h"
#include "modelofflinerender.h"
#include "meshloader.h"

class NormalAndDepthMapsGenerator : public QObject
{
    Q_OBJECT
public:
    NormalAndDepthMapsGenerator(ModelWidget *modelWidget);
    void updateMesh(MeshLoader *mesh);
    void setRenderThread(QThread *thread);
    ~NormalAndDepthMapsGenerator();
    QOpenGLTexture *takeNormalMap();
    QOpenGLTexture *takeDepthMap();
signals:
    void finished();
public slots:
    void process();
private:
    ModelOfflineRender *m_normalMapRender = nullptr;
    ModelOfflineRender *m_depthMapRender = nullptr;
    QSize m_viewPortSize;
    QOpenGLTexture *m_normalMap = nullptr;
    QOpenGLTexture *m_depthMap = nullptr;
    
    ModelOfflineRender *createOfflineRender(ModelWidget *modelWidget, int purpose);
    void generate();
};

#endif
