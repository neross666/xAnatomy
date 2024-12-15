#include "SagittalWidget.h"
#include <vtkImageActor.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include "DataModel.h"
#include "SlicePickInteractorStyle.h"



SagittalWidget::SagittalWidget(QWidget* parent/* = nullptr*/) : 
	PlaneWidget(parent)
{
	m_type = BaseStruct::PlaneType::Sagittal;

	auto style = dynamic_cast<SlicePickInteractorStyle*>(this->interactor()->GetInteractorStyle());
	if (style != nullptr)
		style->SetWidget(this);
}


void SagittalWidget::updateSlice()
{
}
