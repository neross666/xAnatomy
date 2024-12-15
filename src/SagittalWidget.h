#pragma once
#include "PlaneWidget.h"

class SagittalWidget : public PlaneWidget
{
public:
	SagittalWidget(QWidget* parent = nullptr);
	~SagittalWidget() = default;
	
	void updateSlice();

private:
};
