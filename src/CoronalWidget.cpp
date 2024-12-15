#include "CoronalWidget.h"
#include <vtkImageActor.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include "DataModel.h"
#include "SlicePickInteractorStyle.h"


CoronalWidget::CoronalWidget(QWidget* parent) : PlaneWidget(parent)
{
	m_type = BaseStruct::PlaneType::Coronal;


	auto style = dynamic_cast<SlicePickInteractorStyle*>(this->interactor()->GetInteractorStyle());
	if (style != nullptr)
		style->SetWidget(this);
}


