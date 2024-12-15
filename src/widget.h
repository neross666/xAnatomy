#pragma once
#include <QWidget>
#include "BaseStruct.h"

namespace Ui {
	class Widget;
}


class QTimer;
class SliceExtractor;
class Widget : public QWidget
{
	Q_OBJECT
public:
	explicit Widget(QWidget* parent = nullptr);
	~Widget() = default;

	void setSliceExtractor(SliceExtractor* dm);

private:
	Ui::Widget* ui;
	QTimer* m_timer;
	SliceExtractor* m_extractor = nullptr;

	BaseStruct::PlaneType m_activedPlane = BaseStruct::PlaneType::Axial;
};