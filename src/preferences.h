#ifndef PREFERENCES_H
#define PREFERENCES_H
#include <QSettings>
#include <QColor>
#include "combinemode.h"

class Preferences : public QObject
{
    Q_OBJECT
public:
    static Preferences &instance();
    Preferences();
    CombineMode componentCombineMode() const;
    const QColor &partColor() const;
    bool flatShading() const;
    bool threeNodesBranchEnabled() const;
signals:
    void componentCombineModeChanged();
    void partColorChanged();
    void flatShadingChanged();
    void threeNodesBranchEnableStateChanged();
public slots:
    void setComponentCombineMode(CombineMode mode);
    void setPartColor(const QColor &color);
    void setFlatShading(bool flatShading);
    void setThreeNodesBranchEnableState(bool enabled);
    void reset();
private:
    CombineMode m_componentCombineMode;
    QColor m_partColor;
    bool m_flatShading;
    QSettings m_settings;
    bool m_threeNodesBranchEnabled;
private:
    void loadDefault();
};

#endif
