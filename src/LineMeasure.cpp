#include "LineMeasure.h"
#include <qDebug>
#include <vtkRenderWindowInteractor.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkEventQtSlotConnect.h>


LineMeasure::LineMeasure(QObject* obj /*= nullptr*/) : QObject(obj)
{
	m_distanceWidget = vtkSmartPointer<vtkDistanceWidget>::New();
	m_distanceWidget->CreateDefaultRepresentation();
	m_connector = vtkSmartPointer<vtkEventQtSlotConnect>::New();
	m_connector->Connect(m_distanceWidget, vtkCommand::EndInteractionEvent,
		this, SLOT(doMeasure(vtkObject*, unsigned long, void*)));
}

void LineMeasure::init(vtkRenderWindowInteractor* inter)
{
	m_distanceWidget->SetInteractor(inter);
	m_distanceWidget->On();
}

void LineMeasure::doMeasure(vtkObject*, unsigned long eventid, void*)
{
	switch (eventid)
	{
	case vtkCommand::EndInteractionEvent:
	{
		auto representation =
			static_cast<vtkDistanceRepresentation2D*> (m_distanceWidget->GetRepresentation());
		double p1[3];
		representation->GetPoint1DisplayPosition(p1);
		double p2[3];
		representation->GetPoint1WorldPosition(p2);
		double p3[3];
		representation->GetPoint2DisplayPosition(p3);
		double p4[3];
		representation->GetPoint2WorldPosition(p4);
		qDebug() << "dis: " << representation->GetDistance();
		//显示其中一个点的屏幕坐标(px)
		std::cout << "P1: " << p1[0] << " " << p1[1] << " " << p1[2] << std::endl;
	}
	break;
	default:
		break;
	}
}
