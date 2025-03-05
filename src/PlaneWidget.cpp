#include "PlaneWidget.h"
#include <qDebug>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkImageReslice.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>
#include <QVTKInteractor.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkWorldPointPicker.h>
#include <vtkCamera.h>
#include <vtkTransform.h>
#include "SlicePickInteractorStyle.h"
#include "MeasureInteractorStyle.h"
#include "DataModel.h"
#include "SliceExtractor.h"

#include "LineMeasure.h"


PlaneWidget::PlaneWidget(QWidget* parent/* = nullptr*/) : QVTKOpenGLNativeWidget(parent)
{
	m_initCamera = vtkSmartPointer<vtkCamera>::New();

	m_renderer = vtkSmartPointer<vtkRenderer>::New();
	m_renderer->SetBackground(0.32, 0.32, 0.32);
	this->renderWindow()->AddRenderer(m_renderer);

	auto style = vtkSmartPointer<SlicePickInteractorStyle>::New();
	style->SetDefaultRenderer(m_renderer);
	interactor()->SetInteractorStyle(style);

	// 	LineMeasure* linemeasrure = new LineMeasure(this);
	// 	linemeasrure->init(interactor());
}

void PlaneWidget::setSliceExtractor(SliceExtractor* dm)
{
	m_sliceExtractor = dm;
	connect(dm, &SliceExtractor::sigAxesReset, this, &PlaneWidget::resetSlice);
	connect(dm, &SliceExtractor::sigAxesChanged, this, &PlaneWidget::updateSlice);

	makeImageActor(dm);
	makeImplantActor(dm);

	m_renderer->ResetCamera();
	m_initCamera->DeepCopy(m_renderer->GetActiveCamera());

	this->renderWindow()->Render();
}

void PlaneWidget::updateSlice(BaseStruct::PlaneType t, double theta)
{
	auto slice = m_sliceExtractor->extractSlice(m_type);
	m_imgActor->GetMapper()->SetInputData(slice);

	// 坐标轴的原点始终在(0,0,0)处!!!通过调整相机焦点使得图像在窗口中居中显示
	double* bounds = slice->GetBounds();
	updateAxes(bounds);

	// 
	auto implant_slice = m_sliceExtractor->extractImplantSlice(m_type);
	auto mapper = dynamic_cast<vtkPolyDataMapper*>(m_implantActor->GetMapper());
	mapper->SetInputData(implant_slice);


	// 还需区分调整窗口和被影响窗口的显示
	updateCamera(t, slice->GetCenter(), theta);

	this->renderWindow()->Render();
}

void PlaneWidget::resetSlice()
{
	auto slice = m_sliceExtractor->extractSlice(m_type);
	m_imgActor->GetMapper()->SetInputData(slice);

	// 坐标轴的原点始终在(0,0,0)处!!!通过调整相机焦点使得图像在窗口中居中显示
	double* bounds = slice->GetBounds();
	updateAxes(bounds);


	// 
	auto implant_slice = m_sliceExtractor->extractImplantSlice(m_type);
	auto mapper = dynamic_cast<vtkPolyDataMapper*>(m_implantActor->GetMapper());
	mapper->SetInputData(implant_slice);


	// 还原相机
	auto camera = m_renderer->GetActiveCamera();
	camera->DeepCopy(m_initCamera);


	this->renderWindow()->Render();
}

void PlaneWidget::updateAxes(double bounds[6])
{
	// Z方向移动一小段距离，避免被图像遮挡
	this->HAxes->SetPoint1(bounds[0], 0, 0.01);
	this->HAxes->SetPoint2(bounds[1], 0, 0.01);
	this->VAxes->SetPoint1(0, bounds[2], 0.01);
	this->VAxes->SetPoint2(0, bounds[3], 0.01);

	// 	qDebug() << "Slice Bounds: "
	// 		<< bounds[0] << "," << bounds[1] << "," << bounds[2] << ","
	// 		<< bounds[3] << "," << bounds[4] << "," << bounds[5];
}

void PlaneWidget::updateCamera(BaseStruct::PlaneType t, double center[3], double theta)
{
	// 使得图像更新后，直角坐标系的原点处于鼠标处
	// 需要依据slice的结果来设置一个合理的相机
	// 1. 焦点始终对准图像中心---done
	// 2. 相机位置与焦点的距离始终保持设定的值---done
	// 3. 对于当前调整的窗口,相机viewUp向量始终保持设定的值---done
	// 4. 相机始终对视方向始终与图像垂直---done(0,0,1)


	// 应用姿态矩阵到相机
// 	auto transform = vtkSmartPointer<vtkTransform>::New();
// 	transform->Translate(center);
// 	//transform->RotateX();
// 
// 	auto camera = vtkSmartPointer<vtkCamera>::New();	
// 	camera->DeepCopy(m_initCamera);
// 	camera->ApplyTransform(transform);
// 	m_renderer->SetActiveCamera(camera);


	// 直接设置相机参数
// 	{
		// 设置相机焦点和位置
	auto camera = m_renderer->GetActiveCamera();
	auto distance = camera->GetDistance();

	double vn[3]{ 0.0 };
	camera->GetViewPlaneNormal(vn);
	//auto vn = camera->GetViewPlaneNormal();	// 这种方式取出来的值不对!!!
	auto fp = m_initCamera->GetFocalPoint();
	double nfp[3]{ fp[0] + center[0], fp[1] + center[1], fp[2] + center[2] };
	camera->SetFocalPoint(nfp);
	camera->SetPosition(
		nfp[0] + distance * vn[0], nfp[1] + distance * vn[1], nfp[2] + distance * vn[2]);


	// 
	if (t == m_type)
	{
		// 设置ViewUp
		double nvup[3];
		auto transform = vtkSmartPointer<vtkTransform>::New();
		transform->RotateWXYZ(theta, vn);		// 区分顺时针(+)和逆时针(-)以及相机视角
		double vup[3]{ 0.0 };
		camera->GetViewUp(vup);
		transform->TransformVector(vup, nvup);
		camera->SetViewUp(nvup);
	}


	// 	}


	auto printCam = [&camera]() {
		auto position = camera->GetPosition();
		auto fp = camera->GetFocalPoint();
		auto viewUp = camera->GetViewUp();
		double vn[3]{ 0.0 };
		camera->GetViewPlaneNormal(vn);
		qDebug() << "pos: " << position[0] << ", " << position[1] << ", " << position[2];
		qDebug() << "FP: " << fp[0] << ", " << fp[1] << ", " << fp[2];
		qDebug() << "ViewNormal: " << vn[0] << ", " << vn[1] << ", " << vn[2];
		qDebug() << "viewUp: " << viewUp[0] << ", " << viewUp[1] << ", " << viewUp[2];
	};
	//printCam();
}

void PlaneWidget::makeAxesActor(double bounds[6])
{
	this->HAxes = vtkSmartPointer<vtkLineSource>::New();
	this->VAxes = vtkSmartPointer<vtkLineSource>::New();

	// 坐标轴原点始终在切出结果图像的(0,0,0)点，通过调整相机焦点位置来使得图像处于窗口中间
	this->HAxes->SetPoint1(bounds[0], 0, 0.1);
	this->HAxes->SetPoint2(bounds[1], 0, 0.1);
	this->VAxes->SetPoint1(0, bounds[2], 0.1);
	this->VAxes->SetPoint2(0, bounds[3], 0.1);


	auto mapperX = vtkSmartPointer<vtkPolyDataMapper>::New();
	auto mapperY = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapperX->SetInputConnection(this->HAxes->GetOutputPort());
	mapperY->SetInputConnection(this->VAxes->GetOutputPort());

	// 创建 actor
	this->HAxesActor = vtkSmartPointer<vtkActor>::New();
	this->VAxesActor = vtkSmartPointer<vtkActor>::New();
	this->HAxesActor->SetMapper(mapperX);
	this->VAxesActor->SetMapper(mapperY);

	static double colors[3][3] = {
		{0.5, 0.3, 0.3},
		{0.3, 0.5, 0.3},
		{0.3, 0.3, 0.5}
	};

	// 颜色依赖m_type区分
	this->HAxesActor->GetProperty()->SetLineWidth(5);
	this->VAxesActor->GetProperty()->SetLineWidth(5);
	this->HAxesActor->GetProperty()->SetOpacity(0.8);
	this->VAxesActor->GetProperty()->SetOpacity(0.8);

	int i = static_cast<int>(m_type);
	this->HAxesActor->GetProperty()->SetColor(colors[i % 3]);
	this->VAxesActor->GetProperty()->SetColor(colors[(i + 1) % 3]);

	m_renderer->AddActor(HAxesActor);
	m_renderer->AddActor(VAxesActor);
}

void PlaneWidget::makeImageActor(const SliceExtractor* dm)
{
	if (m_imgActor == nullptr)
	{
		m_imgActor = vtkSmartPointer<vtkImageActor>::New();
		m_renderer->AddActor(m_imgActor);
	}
	auto slice = dm->extractSlice(m_type);
	m_imgActor->GetMapper()->SetInputData(slice);

	makeAxesActor(slice->GetBounds());
}

void PlaneWidget::makeImplantActor(const SliceExtractor* dm)
{
	if (m_implantActor == nullptr)
	{
		m_implantActor = vtkSmartPointer<vtkActor>::New();
		auto polyMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		m_implantActor->SetMapper(polyMapper);
		m_implantActor->GetProperty()->SetColor(1.0, 1.0, 0.0);
		m_implantActor->GetProperty()->SetLineWidth(3);
		m_renderer->AddActor(m_implantActor);
	}
	auto slice = dm->extractImplantSlice(m_type);
	auto mapper = dynamic_cast<vtkPolyDataMapper*>(m_implantActor->GetMapper());
	mapper->SetInputData(slice);
}

void PlaneWidget::setAxesOrigin(int x, int y)
{
	double world[4]{ 0.0 };
	pickWorldPosition(x, y, world);
	m_sliceExtractor->setAxesOrigin(m_type, world);
}

void PlaneWidget::rotateAxes(int pre_x, int pre_y, int x, int y)
{
	double pre_world[4]{ 0.0 };
	double world[4]{ 0.0 };
	pickWorldPosition(pre_x, pre_y, pre_world);
	pickWorldPosition(x, y, world);

	m_sliceExtractor->rotateAxes(m_type, pre_world, world);
}

void PlaneWidget::setMeasureMode(bool enter)
{
	if (enter)
	{
		auto style = vtkSmartPointer<MeasureInteractorStyle>::New();
		style->SetDefaultRenderer(m_renderer);
		style->SetWidget(this);
		interactor()->SetInteractorStyle(style);
	}
	else
	{
		auto style = vtkSmartPointer<SlicePickInteractorStyle>::New();
		style->SetDefaultRenderer(m_renderer);
		style->SetWidget(this);
		interactor()->SetInteractorStyle(style);
	}
}

void PlaneWidget::pickWorldPosition(int x, int y, double world[4]) const
{
	auto worldPicker = vtkSmartPointer<vtkWorldPointPicker>::New();
	worldPicker->Pick(x, y, 0, this->m_renderer);
	worldPicker->GetPickPosition(world);
	world[3] = 1.0;
	//qDebug() << QString("slice position   x:%1, y:%2, z:%3").arg(world[0]).arg(world[1]).arg(world[2]);
}

void PlaneWidget::activeAxesActor(bool active)
{
	if (active)
	{
		this->HAxesActor->GetProperty()->SetOpacity(1.0);
		this->VAxesActor->GetProperty()->SetOpacity(1.0);
	}
	else
	{
		this->HAxesActor->GetProperty()->SetOpacity(0.8);
		this->VAxesActor->GetProperty()->SetOpacity(0.8);
	}	
	this->renderWindow()->Render();
}
