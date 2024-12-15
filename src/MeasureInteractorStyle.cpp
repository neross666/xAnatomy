#include "MeasureInteractorStyle.h"
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkObjectFactory.h>
#include <vtkWorldPointPicker.h>
#include <qDebug>
#include <vtkPropPicker.h>
#include "PlaneWidget.h"

vtkStandardNewMacro(MeasureInteractorStyle);

MeasureInteractorStyle::MeasureInteractorStyle()
{
}

void MeasureInteractorStyle::SetWidget(PlaneWidget* w)
{
	m_widget = w;
}


void MeasureInteractorStyle::OnLeftButtonDoubleClick()
{
	int* pos = this->GetInteractor()->GetEventPosition();

	auto picker = vtkSmartPointer<vtkPropPicker>::New();
	picker->Pick(pos[0], pos[1], 0, this->GetDefaultRenderer());
	auto actor = picker->GetActor();
	if (actor)
	{
		// 删除这个测量
	}
}