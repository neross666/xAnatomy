#include "widget.h"
#include "resources/ui/ui_widget.h"		// 必须与widget.ui文件的路径匹配
#include <qDebug>
#include <QTimer>
#include "SliceExtractor.h"


Widget::Widget(QWidget* parent /*= nullptr*/) : QWidget(parent), ui(new Ui::Widget)
{
	ui->setupUi(this);

	connect(ui->pushButton_reset, &QPushButton::clicked, this, [this]() {
		m_extractor->resetAxes();
		});
	connect(ui->pushButton_rotate, &QPushButton::clicked, this, [this]() {
		m_extractor->rotateAxes(m_activedPlane, 5.0);
		});
	connect(ui->pushButton_rotate_inv, &QPushButton::clicked, this, [this]() {
		m_extractor->rotateAxes(m_activedPlane, -5.0);
		});
	connect(ui->pushButton_up, &QPushButton::clicked, this, [this]() {
		m_extractor->moveAxesOrigin(m_activedPlane, false, 1.0);
		});
	connect(ui->pushButton_down, &QPushButton::clicked, this, [this]() {
		m_extractor->moveAxesOrigin(m_activedPlane, false, -1.0);
		});
	connect(ui->pushButton_left, &QPushButton::clicked, this, [this]() {
		m_extractor->moveAxesOrigin(m_activedPlane, true, -1.0);
		});
	connect(ui->pushButton_right, &QPushButton::clicked, this, [this]() {
		m_extractor->moveAxesOrigin(m_activedPlane, true, 1.0);
		});
	connect(ui->pushButton_playSlice, &QPushButton::clicked, this, [this]() {
		if (m_timer->isActive())
			m_timer->stop();
		else
			m_timer->start(50);
		});

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, [this]() {
		m_extractor->playAxes(m_activedPlane);
		});
}

void Widget::setSliceExtractor(SliceExtractor* dm)
{
	m_extractor = dm;
	ui->widget_sagittal->setSliceExtractor(dm);
	ui->widget_coronal->setSliceExtractor(dm);
	ui->widget_axial->setSliceExtractor(dm);
	ui->widget_volume->setSliceExtractor(dm);
}

