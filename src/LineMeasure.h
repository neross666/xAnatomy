#pragma once
#include <QObject>
#include <vtkSmartPointer.h>

class vtkObject;
class vtkDistanceWidget;
class vtkEventQtSlotConnect;
class vtkRenderWindowInteractor;
class LineMeasure : public QObject
{
	Q_OBJECT
public:
	LineMeasure(QObject* obj = nullptr);

	void init(vtkRenderWindowInteractor* inter);

private slots:
	void doMeasure(vtkObject*, unsigned long eventid, void*);

private:
	vtkSmartPointer<vtkDistanceWidget> m_distanceWidget;
	vtkSmartPointer<vtkEventQtSlotConnect> m_connector;
};