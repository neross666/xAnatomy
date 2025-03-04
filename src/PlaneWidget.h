#pragma once
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
#include "BaseStruct.h"


// 矢状面（Sagital Plane）：沿着身体前后径所做的与地面垂直的切面；
// 冠状面（Coronal Plane）：沿着身体左右径所做的与地面垂直的切面；
// 横断面（Transverse / Axial Plane）：与地面平行的切面。


class vtkRenderer;
class vtkStringArray;
class SliceExtractor;
class vtkMatrix4x4;
class vtkImageActor;
class vtkActor;
class vtkCamera;
class vtkLineSource;
class vtkImageData;
class PlaneWidget : public QVTKOpenGLNativeWidget
{
	Q_OBJECT

public:
	PlaneWidget(QWidget* parent = nullptr);

	virtual ~PlaneWidget() = default;

	void setSliceExtractor(SliceExtractor* dm);

	void setAxesOrigin(int x, int y);

	void rotateAxes(int pre_x, int pre_y, int x, int y);

public slots:
	void setMeasureMode(bool enter);

signals:
	void changeAxisOrigin(double origin[4]);

protected:
	void resetSlice();

	void updateSlice(BaseStruct::PlaneType t, double theta);

	void updateAxes(double bounds[6]);

	void updateCamera(BaseStruct::PlaneType t, double position[3], double theta);

	void makeAxesActor(double bounds[6]);

	void makeImageActor(const SliceExtractor* dm);

	void makeImplantActor(const SliceExtractor* dm);

	void pickWorldPosition(int x, int y, double world[4]) const;

protected:
	BaseStruct::PlaneType m_type;
	SliceExtractor* m_sliceExtractor = nullptr;

	vtkSmartPointer<vtkLineSource> HAxes = nullptr;
	vtkSmartPointer<vtkLineSource> VAxes = nullptr;
	vtkSmartPointer<vtkActor> HAxesActor = nullptr;
	vtkSmartPointer<vtkActor> VAxesActor = nullptr;

	vtkSmartPointer<vtkImageActor> m_imgActor = nullptr;
	vtkSmartPointer<vtkActor> m_implantActor = nullptr;

	vtkSmartPointer<vtkRenderer> m_renderer = nullptr;
	vtkSmartPointer<vtkCamera> m_initCamera = nullptr;
};

