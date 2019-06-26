#include "preferences.h"
#include "util.h"

Preferences &Preferences::instance()
{
    static Preferences *s_preferences = nullptr;
    if (nullptr == s_preferences) {
        s_preferences = new Preferences;
    }
    return *s_preferences;
}

void Preferences::loadDefault()
{
    m_componentCombineMode = CombineMode::Normal;
    m_partColor = Qt::white;
    m_flatShading = false;
    m_threeNodesBranchEnabled = false;
}

Preferences::Preferences()
{
    loadDefault();
    {
        QString value = m_settings.value("componentCombineMode").toString();
        if (!value.isEmpty())
            m_componentCombineMode = CombineModeFromString(value.toUtf8().constData());
    }
    {
        QString value = m_settings.value("partColor").toString();
        if (!value.isEmpty())
            m_partColor = QColor(value);
    }
    {
        QString value = m_settings.value("flatShading").toString();
        if (value.isEmpty())
            m_flatShading = true;
        else
            m_flatShading = isTrueValueString(value);
    }
    {
        QString value = m_settings.value("threeNodesBranchEnabled").toString();
        if (!value.isEmpty())
            m_threeNodesBranchEnabled = isTrueValueString(value);
    }
}

CombineMode Preferences::componentCombineMode() const
{
    return m_componentCombineMode;
}

const QColor &Preferences::partColor() const
{
    return m_partColor;
}

bool Preferences::flatShading() const
{
    return m_flatShading;
}

bool Preferences::threeNodesBranchEnabled() const
{
    return m_threeNodesBranchEnabled;
}

void Preferences::setComponentCombineMode(CombineMode mode)
{
    if (m_componentCombineMode == mode)
        return;
    m_componentCombineMode = mode;
    m_settings.setValue("componentCombineMode", CombineModeToString(m_componentCombineMode));
    emit componentCombineModeChanged();
}

void Preferences::setPartColor(const QColor &color)
{
    if (m_partColor == color)
        return;
    m_partColor = color;
    m_settings.setValue("partColor", color.name());
    emit partColorChanged();
}

void Preferences::setFlatShading(bool flatShading)
{
    if (m_flatShading == flatShading)
        return;
    m_flatShading = flatShading;
    m_settings.setValue("flatShading", flatShading ? "true" : "false");
    emit flatShadingChanged();
}

void Preferences::setThreeNodesBranchEnableState(bool enabled)
{
    if (m_threeNodesBranchEnabled == enabled)
        return;
    m_threeNodesBranchEnabled = enabled;
    m_settings.setValue("threeNodesBranchEnabled", enabled ? "true" : "false");
    emit threeNodesBranchEnableStateChanged();
}

void Preferences::reset()
{
    m_settings.clear();
    loadDefault();
    emit componentCombineModeChanged();
    emit partColorChanged();
    emit flatShadingChanged();
    emit threeNodesBranchEnableStateChanged();
}
