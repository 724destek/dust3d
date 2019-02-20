#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QMenu>
#include <QWidgetAction>
#include <QColorDialog>
#include <QSizePolicy>
#include <QFileDialog>
#include "partwidget.h"
#include "theme.h"
#include "floatnumberwidget.h"
#include "materiallistwidget.h"
#include "infolabel.h"

PartWidget::PartWidget(const Document *document, QUuid partId) :
    m_document(document),
    m_partId(partId),
    m_unnormal(false)
{
    QSizePolicy retainSizePolicy = sizePolicy();
    retainSizePolicy.setRetainSizeWhenHidden(true);

    m_visibleButton = new QPushButton();
    m_visibleButton->setToolTip(tr("Show/hide nodes"));
    m_visibleButton->setSizePolicy(retainSizePolicy);
    initButton(m_visibleButton);
    
    m_lockButton = new QPushButton();
    m_lockButton->setToolTip(tr("Lock/unlock nodes"));
    m_lockButton->setSizePolicy(retainSizePolicy);
    initButton(m_lockButton);
    
    m_subdivButton = new QPushButton();
    m_subdivButton->setToolTip(tr("Square/Round"));
    m_subdivButton->setSizePolicy(retainSizePolicy);
    initButton(m_subdivButton);
    
    m_disableButton = new QPushButton();
    m_disableButton->setToolTip(tr("Join/Remove from final result"));
    m_disableButton->setSizePolicy(retainSizePolicy);
    initButton(m_disableButton);
    
    m_xMirrorButton = new QPushButton();
    m_xMirrorButton->setToolTip(tr("Toggle mirror"));
    m_xMirrorButton->setSizePolicy(retainSizePolicy);
    initButton(m_xMirrorButton);
    
    m_deformButton = new QPushButton();
    m_deformButton->setToolTip(tr("Deform"));
    m_deformButton->setSizePolicy(retainSizePolicy);
    initButton(m_deformButton);
    
    m_roundButton = new QPushButton;
    m_roundButton->setToolTip(tr("Toggle round end"));
    m_roundButton->setSizePolicy(retainSizePolicy);
    initButton(m_roundButton);
    
    m_colorButton = new QPushButton;
    m_colorButton->setToolTip(tr("Color and material picker"));
    m_colorButton->setSizePolicy(retainSizePolicy);
    initButton(m_colorButton);
    
    m_wrapButton = new QPushButton;
    m_wrapButton->setToolTip(tr("Toggle convex wrap"));
    m_wrapButton->setSizePolicy(retainSizePolicy);
    initButton(m_wrapButton);
    
    m_previewWidget = new ModelWidget;
    m_previewWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_previewWidget->enableMove(false);
    m_previewWidget->enableZoom(false);
    m_previewWidget->setFixedSize(Theme::partPreviewImageSize, Theme::partPreviewImageSize);
    
    QWidget *hrLightWidget = new QWidget;
    hrLightWidget->setFixedHeight(1);
    hrLightWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //hrLightWidget->setStyleSheet(QString("background-color: #565656;"));
    hrLightWidget->setContentsMargins(0, 0, 0, 0);
    
    QWidget *hrWidget = new QWidget;
    hrWidget->setFixedHeight(1);
    hrWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //hrWidget->setStyleSheet(QString("background-color: #1a1a1a;"));
    hrWidget->setContentsMargins(0, 0, 0, 0);
    
    QGridLayout *toolsLayout = new QGridLayout;
    toolsLayout->setSpacing(0);
    toolsLayout->setContentsMargins(0, 0, 5, 0);
    int row = 0;
    int col = 0;
    toolsLayout->addWidget(m_lockButton, row, col++, Qt::AlignBottom);
    toolsLayout->addWidget(m_disableButton, row, col++, Qt::AlignBottom);
    toolsLayout->addWidget(m_wrapButton, row, col++, Qt::AlignBottom);
    toolsLayout->addWidget(m_colorButton, row, col++, Qt::AlignBottom);
    row++;
    col = 0;
    toolsLayout->addWidget(m_subdivButton, row, col++, Qt::AlignTop);
    toolsLayout->addWidget(m_deformButton, row, col++, Qt::AlignTop);
    toolsLayout->addWidget(m_xMirrorButton, row, col++, Qt::AlignTop);
    toolsLayout->addWidget(m_roundButton, row, col++, Qt::AlignTop);
    
    m_visibleButton->setContentsMargins(0, 0, 0, 0);
    
    QHBoxLayout *previewAndToolsLayout = new QHBoxLayout;
    previewAndToolsLayout->setSpacing(0);
    previewAndToolsLayout->setContentsMargins(0, 0, 0, 0);
    previewAndToolsLayout->addWidget(m_visibleButton);
    previewAndToolsLayout->addWidget(m_previewWidget);
    previewAndToolsLayout->addLayout(toolsLayout);
    previewAndToolsLayout->setStretch(0, 0);
    previewAndToolsLayout->setStretch(1, 0);
    previewAndToolsLayout->setStretch(2, 1);
    
    QWidget *backgroundWidget = new QWidget;
    backgroundWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    backgroundWidget->setFixedSize(preferredSize().width(), Theme::partPreviewImageSize);
    backgroundWidget->setObjectName("background");
    m_backgroundWidget = backgroundWidget;
    backgroundWidget->setLayout(previewAndToolsLayout);
    
    QHBoxLayout *backgroundLayout = new QHBoxLayout;
    backgroundLayout->setSpacing(0);
    backgroundLayout->setContentsMargins(0, 0, 0, 0);
    backgroundLayout->addWidget(backgroundWidget);
    backgroundLayout->addSpacing(10);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(hrLightWidget);
    mainLayout->addSpacing(3);
    mainLayout->addLayout(backgroundLayout);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(hrWidget);
    
    setLayout(mainLayout);
    
    connect(this, &PartWidget::setPartLockState, m_document, &Document::setPartLockState);
    connect(this, &PartWidget::setPartVisibleState, m_document, &Document::setPartVisibleState);
    connect(this, &PartWidget::setPartSubdivState, m_document, &Document::setPartSubdivState);
    connect(this, &PartWidget::setPartDisableState, m_document, &Document::setPartDisableState);
    connect(this, &PartWidget::setPartXmirrorState, m_document, &Document::setPartXmirrorState);
    connect(this, &PartWidget::setPartDeformThickness, m_document, &Document::setPartDeformThickness);
    connect(this, &PartWidget::setPartDeformWidth, m_document, &Document::setPartDeformWidth);
    connect(this, &PartWidget::setPartRoundState, m_document, &Document::setPartRoundState);
    connect(this, &PartWidget::setPartWrapState, m_document, &Document::setPartWrapState);
    connect(this, &PartWidget::setPartColorState, m_document, &Document::setPartColorState);
    connect(this, &PartWidget::setPartMaterialId, m_document, &Document::setPartMaterialId);
    connect(this, &PartWidget::checkPart, m_document, &Document::checkPart);
    connect(this, &PartWidget::enableBackgroundBlur, m_document, &Document::enableBackgroundBlur);
    connect(this, &PartWidget::disableBackgroundBlur, m_document, &Document::disableBackgroundBlur);
    
    connect(this, &PartWidget::groupOperationAdded, m_document, &Document::saveSnapshot);
    
    connect(m_lockButton, &QPushButton::clicked, [=]() {
        const SkeletonPart *part = m_document->findPart(m_partId);
        if (!part) {
            qDebug() << "Part not found:" << m_partId;
            return;
        }
        emit setPartLockState(m_partId, !part->locked);
        emit groupOperationAdded();
    });
    
    connect(m_visibleButton, &QPushButton::clicked, [=]() {
        const SkeletonPart *part = m_document->findPart(m_partId);
        if (!part) {
            qDebug() << "Part not found:" << m_partId;
            return;
        }
        emit setPartVisibleState(m_partId, !part->visible);
        emit groupOperationAdded();
    });
    
    connect(m_subdivButton, &QPushButton::clicked, [=]() {
        const SkeletonPart *part = m_document->findPart(m_partId);
        if (!part) {
            qDebug() << "Part not found:" << m_partId;
            return;
        }
        emit setPartSubdivState(m_partId, !part->subdived);
        emit groupOperationAdded();
    });
    
    connect(m_disableButton, &QPushButton::clicked, [=]() {
        const SkeletonPart *part = m_document->findPart(m_partId);
        if (!part) {
            qDebug() << "Part not found:" << m_partId;
            return;
        }
        emit setPartDisableState(m_partId, !part->disabled);
        emit groupOperationAdded();
    });
    
    connect(m_xMirrorButton, &QPushButton::clicked, [=]() {
        const SkeletonPart *part = m_document->findPart(m_partId);
        if (!part) {
            qDebug() << "Part not found:" << m_partId;
            return;
        }
        emit setPartXmirrorState(m_partId, !part->xMirrored);
        emit groupOperationAdded();
    });
    
    connect(m_deformButton, &QPushButton::clicked, [=]() {
        const SkeletonPart *part = m_document->findPart(m_partId);
        if (!part) {
            qDebug() << "Part not found:" << m_partId;
            return;
        }
        showDeformSettingPopup(mapFromGlobal(QCursor::pos()));
    });
    
    connect(m_roundButton, &QPushButton::clicked, [=]() {
        const SkeletonPart *part = m_document->findPart(m_partId);
        if (!part) {
            qDebug() << "Part not found:" << m_partId;
            return;
        }
        emit setPartRoundState(m_partId, !part->rounded);
        emit groupOperationAdded();
    });
    
    connect(m_colorButton, &QPushButton::clicked, [=]() {
        const SkeletonPart *part = m_document->findPart(m_partId);
        if (!part) {
            qDebug() << "Part not found:" << m_partId;
            return;
        }
        showColorSettingPopup(mapFromGlobal(QCursor::pos()));
    });
    
    connect(m_wrapButton, &QPushButton::clicked, [=]() {
        const SkeletonPart *part = m_document->findPart(m_partId);
        if (!part) {
            qDebug() << "Part not found:" << m_partId;
            return;
        }
        emit setPartWrapState(m_partId, !part->wrapped);
        emit groupOperationAdded();
    });
    
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setFixedSize(preferredSize());
    
    updateAllButtons();
}

ModelWidget *PartWidget::previewWidget()
{
    return m_previewWidget;
}

QSize PartWidget::preferredSize()
{
    return QSize(Theme::miniIconSize + Theme::partPreviewImageSize + Theme::miniIconSize * 4 + 5 + 2, Theme::partPreviewImageSize + 6);
}

void PartWidget::updateAllButtons()
{
    updateVisibleButton();
    updateLockButton();
    updateSubdivButton();
    updateDisableButton();
    updateXmirrorButton();
    updateDeformButton();
    updateRoundButton();
    updateColorButton();
    updateWrapButton();
}

void PartWidget::updateCheckedState(bool checked)
{
    if (checked)
        m_backgroundWidget->setStyleSheet("QWidget#background {border: 1px solid " + (m_unnormal ? Theme::blue.name() : Theme::red.name()) + ";}");
    else
        m_backgroundWidget->setStyleSheet("QWidget#background {border: 1px solid transparent;}");
}

void PartWidget::updateUnnormalState(bool unnormal)
{
    if (m_unnormal == unnormal)
        return;
    
    m_unnormal = unnormal;
    updateAllButtons();
}

void PartWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
    emit checkPart(m_partId);
}

void PartWidget::initToolButtonWithoutFont(QPushButton *button)
{
    Theme::initAwesomeToolButtonWithoutFont(button);
}

void PartWidget::initToolButton(QPushButton *button)
{
    Theme::initAwesomeToolButton(button);
}

void PartWidget::showColorSettingPopup(const QPoint &pos)
{
    QMenu popupMenu;
    
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Find part failed:" << m_partId;
        return;
    }
    
    QWidget *popup = new QWidget;
    
    QPushButton *colorEraser = new QPushButton(QChar(fa::eraser));
    initToolButton(colorEraser);
    
    QPushButton *pickButton = new QPushButton();
    initToolButtonWithoutFont(pickButton);
    QPalette palette = pickButton->palette();
    QColor choosenColor = part->hasColor ? part->color : Qt::white;
    palette.setColor(QPalette::Window, choosenColor);
    palette.setColor(QPalette::Button, choosenColor);
    pickButton->setPalette(palette);
    
    QHBoxLayout *colorLayout = new QHBoxLayout;
    colorLayout->addWidget(colorEraser);
    colorLayout->addWidget(pickButton);
    colorLayout->addStretch();
    
    connect(colorEraser, &QPushButton::clicked, [=]() {
        emit setPartColorState(m_partId, false, Qt::white);
        emit groupOperationAdded();
    });
    
    connect(pickButton, &QPushButton::clicked, [=]() {
        emit disableBackgroundBlur();
        QColor color = QColorDialog::getColor(part->color, this);
        emit enableBackgroundBlur();
        if(color.isValid()) {
            emit setPartColorState(m_partId, true, color);
            emit groupOperationAdded();
        }
    });
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(colorLayout);
    
    if (m_document->materialIdList.empty()) {
        InfoLabel *infoLabel = new InfoLabel;
        infoLabel->setText(tr("Missing Materials"));
        mainLayout->addWidget(infoLabel);
    } else {
        MaterialListWidget *materialListWidget = new MaterialListWidget(m_document);
        materialListWidget->enableMultipleSelection(false);
        materialListWidget->selectMaterial(part->materialId);
        connect(materialListWidget, &MaterialListWidget::currentSelectedMaterialChanged, this, [=](QUuid materialId) {
            emit setPartMaterialId(m_partId, materialId);
            emit groupOperationAdded();
        });
        mainLayout->addWidget(materialListWidget);
    }
    
    popup->setLayout(mainLayout);
    
    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(popup);
    
    popupMenu.addAction(action);
    
    popupMenu.exec(mapToGlobal(pos));
}

void PartWidget::showDeformSettingPopup(const QPoint &pos)
{
    QMenu popupMenu;
    
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Find part failed:" << m_partId;
        return;
    }
    
    QWidget *popup = new QWidget;
    
    FloatNumberWidget *thicknessWidget = new FloatNumberWidget;
    thicknessWidget->setItemName(tr("Thickness"));
    thicknessWidget->setRange(0, 2);
    thicknessWidget->setValue(part->deformThickness);
    
    connect(thicknessWidget, &FloatNumberWidget::valueChanged, [=](float value) {
        emit setPartDeformThickness(m_partId, value);
        emit groupOperationAdded();
    });
    
    FloatNumberWidget *widthWidget = new FloatNumberWidget;
    widthWidget->setItemName(tr("Width"));
    widthWidget->setRange(0, 2);
    widthWidget->setValue(part->deformWidth);
    
    connect(widthWidget, &FloatNumberWidget::valueChanged, [=](float value) {
        emit setPartDeformWidth(m_partId, value);
        emit groupOperationAdded();
    });
    
    QPushButton *thicknessEraser = new QPushButton(QChar(fa::eraser));
    initToolButton(thicknessEraser);
    
    connect(thicknessEraser, &QPushButton::clicked, [=]() {
        thicknessWidget->setValue(1.0);
        emit groupOperationAdded();
    });
    
    QPushButton *widthEraser = new QPushButton(QChar(fa::eraser));
    initToolButton(widthEraser);
    
    connect(widthEraser, &QPushButton::clicked, [=]() {
        widthWidget->setValue(1.0);
        emit groupOperationAdded();
    });
    
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *thicknessLayout = new QHBoxLayout;
    QHBoxLayout *widthLayout = new QHBoxLayout;
    thicknessLayout->addWidget(thicknessEraser);
    thicknessLayout->addWidget(thicknessWidget);
    widthLayout->addWidget(widthEraser);
    widthLayout->addWidget(widthWidget);
    layout->addLayout(thicknessLayout);
    layout->addLayout(widthLayout);
    
    popup->setLayout(layout);
    
    QWidgetAction action(this);
    action.setDefaultWidget(popup);
    
    popupMenu.addAction(&action);
    
    popupMenu.exec(mapToGlobal(pos));
}

void PartWidget::initButton(QPushButton *button)
{
    Theme::initAwesomeMiniButton(button);
}

void PartWidget::updateButton(QPushButton *button, QChar icon, bool highlighted)
{
    Theme::updateAwesomeMiniButton(button, icon, highlighted, m_unnormal);
}

void PartWidget::updatePreview()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    //m_previewLabel->setPixmap(QPixmap::fromImage(part->preview));
    MeshLoader *previewMesh = part->takePreviewMesh();
    m_previewWidget->updateMesh(previewMesh);
}

void PartWidget::updateLockButton()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    if (part->locked)
        updateButton(m_lockButton, QChar(fa::lock), true);
    else
        updateButton(m_lockButton, QChar(fa::unlock), false);
}

void PartWidget::updateVisibleButton()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    if (part->visible)
        updateButton(m_visibleButton, QChar(fa::eye), false);
    else
        updateButton(m_visibleButton, QChar(fa::eyeslash), true);
}

void PartWidget::updateSubdivButton()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    if (part->subdived)
        updateButton(m_subdivButton, QChar(fa::circleo), true);
    else
        updateButton(m_subdivButton, QChar(fa::squareo), false);
}

void PartWidget::updateDisableButton()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    if (part->disabled)
        updateButton(m_disableButton, QChar(fa::unlink), true);
    else
        updateButton(m_disableButton, QChar(fa::link), false);
}

void PartWidget::updateXmirrorButton()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    if (part->xMirrored)
        updateButton(m_xMirrorButton, QChar(fa::balancescale), true);
    else
        updateButton(m_xMirrorButton, QChar(fa::balancescale), false);
}

void PartWidget::updateDeformButton()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    if (part->deformAdjusted())
        updateButton(m_deformButton, QChar(fa::handlizardo), true);
    else
        updateButton(m_deformButton, QChar(fa::handlizardo), false);
}

void PartWidget::updateRoundButton()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    if (part->rounded)
        updateButton(m_roundButton, QChar(fa::magnet), true);
    else
        updateButton(m_roundButton, QChar(fa::magnet), false);
}

void PartWidget::updateColorButton()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    if (part->hasColor || part->materialAdjusted())
        updateButton(m_colorButton, QChar(fa::eyedropper), true);
    else
        updateButton(m_colorButton, QChar(fa::eyedropper), false);
}

void PartWidget::updateWrapButton()
{
    const SkeletonPart *part = m_document->findPart(m_partId);
    if (!part) {
        qDebug() << "Part not found:" << m_partId;
        return;
    }
    if (part->wrapped)
        updateButton(m_wrapButton, QChar(fa::cube), true);
    else
        updateButton(m_wrapButton, QChar(fa::cube), false);
}

void PartWidget::reload()
{
    updatePreview();
    updateAllButtons();
}
