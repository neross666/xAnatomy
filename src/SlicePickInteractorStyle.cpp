#include "SlicePickInteractorStyle.h"
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkObjectFactory.h>
#include <vtkWorldPointPicker.h>
#include <qDebug>
#include <vtkPropPicker.h>
#include "PlaneWidget.h"

vtkStandardNewMacro(SlicePickInteractorStyle);

SlicePickInteractorStyle::SlicePickInteractorStyle()
{
}

void SlicePickInteractorStyle::SetWidget(PlaneWidget* w)
{
	m_widget = w;
}


void SlicePickInteractorStyle::OnLeftButtonDown()
{
	int* pos = this->GetInteractor()->GetEventPosition();

	auto picker = vtkSmartPointer<vtkPropPicker>::New();
	picker->Pick(pos[0], pos[1], 0, this->GetDefaultRenderer());

	// Get the actor that was picked
	this->m_pickedActor = picker->GetActor();

	// Forward events
	vtkInteractorStyleImage::OnLeftButtonDown();
}

void SlicePickInteractorStyle::OnLeftButtonUp()
{
	this->m_pickedActor = nullptr;

	// Forward events
	vtkInteractorStyleImage::OnLeftButtonUp();
}

void SlicePickInteractorStyle::OnKeyDown()
{
	vtkRenderWindowInteractor* rwi = this->Interactor;
	std::string key = rwi->GetKeySym();

	// Check if the Control key was pressed
	if (key == "Control_L" || key == "Control_R")
	{
		m_ctrlPressed = true;

		this->Interactor->GetEventPosition(m_lastXPos, m_lastYPos);
 		MoveAxes(m_lastXPos, m_lastYPos);
 		this->Interactor->Render();
	}

	// Forward events
	vtkInteractorStyleImage::OnKeyDown();
}

void SlicePickInteractorStyle::OnKeyUp()
{
	vtkRenderWindowInteractor* rwi = this->Interactor;
	std::string key = rwi->GetKeySym();

	// Check if the Control key was released
	if (key == "Control_L" || key == "Control_R")
	{
		m_ctrlPressed = false;
	}

	// Forward events
	vtkInteractorStyleImage::OnKeyUp();
}

void SlicePickInteractorStyle::OnMouseMove()
{
	if (m_ctrlPressed)
	{
		int x, y;
		this->Interactor->GetEventPosition(x, y);
		if (x != m_lastXPos || y != m_lastYPos)
		{
			MoveAxes(x, y);
			this->Interactor->Render();
		}
	}

	if (m_pickedActor != nullptr)
	{
		// 
		m_widget;
		int x, y;
		this->Interactor->GetEventPosition(x, y);
		RotateAxes(x, y);
	}

	// Forward events
	vtkInteractorStyle::OnMouseMove();
}


void SlicePickInteractorStyle::MoveAxes(int x, int y)
{
	qDebug() << QString("screen   x:%1, y:%2").arg(x).arg(y);
	m_widget->setAxesOrigin(x, y);
}

void SlicePickInteractorStyle::RotateAxes(int x, int y)
{
	if (m_widget->rect().contains(x, y)) {
		qDebug() << QString("screen   x:%1, y:%2").arg(x).arg(y);
		m_widget->rotateAxes(x, y, m_pickedActor);
	}
}
