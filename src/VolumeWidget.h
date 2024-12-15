#pragma once
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkVolume.h>
#include <vtkAxesActor.h>

class DataModel;
class SliceExtractor;
class vtkTransform;
class VolumeWidget : public QVTKOpenGLNativeWidget
{
	Q_OBJECT

public:
	VolumeWidget(QWidget* parent = nullptr);
	~VolumeWidget() = default;

	void setSliceExtractor(SliceExtractor* dm);


public slots:
	void updateAxesPose();

	// protected:
	// 	void paintGL() override;
	// 	void resizeEvent(QResizeEvent *event) override;
	// 	void mousePressEvent(QMouseEvent *event) override;
	// 	void mouseReleaseEvent(QMouseEvent *event) override;
	// 	void mouseMoveEvent(QMouseEvent *event) override;	

private:
	vtkSmartPointer<vtkActor> makeOutline() const;
	vtkSmartPointer<vtkAxesActor> makeAxes();
	vtkSmartPointer<vtkVolume> makeVolume() const;
	vtkSmartPointer<vtkActor> makeImplant() const;

private:
	DataModel* m_dataModel = nullptr;
	vtkSmartPointer<vtkTransform> m_axesPose = nullptr;
	vtkSmartPointer<vtkRenderer> m_renderer = nullptr;
};

