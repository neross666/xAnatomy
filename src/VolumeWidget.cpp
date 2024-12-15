#include "VolumeWidget.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageData.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkAxesActor.h>
#include <vtkTransform.h>
#include "DataModel.h"
#include "SliceExtractor.h"


#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation.h>
#include <vtkDistanceRepresentation2D.h>
class distanceCallback : public vtkCommand
{
public:
	static distanceCallback* New()
	{
		return new distanceCallback;
	}

	virtual void Execute(vtkObject* caller, unsigned long, void*)
	{
		vtkDistanceWidget* biDimensionalWidget =
			reinterpret_cast<vtkDistanceWidget*> (caller);
		auto representation =
			static_cast<vtkDistanceRepresentation2D*> (biDimensionalWidget->GetRepresentation());
		double p1[3];
		representation->GetPoint1DisplayPosition(p1);
		double p2[3];
		representation->GetPoint1DisplayPosition(p2);
		double p3[3];
		representation->GetPoint1DisplayPosition(p3);
		double p4[3];
		representation->GetPoint1DisplayPosition(p4);
		representation->GetDistance();
		//显示其中一个点的屏幕坐标(px)
		std::cout << "P1: " << p1[0] << " " << p1[1] << " " << p1[2] << std::endl;
	}
	distanceCallback() { }
};

vtkSmartPointer<vtkDistanceWidget> distanceWidget = nullptr;
VolumeWidget::VolumeWidget(QWidget* parent /*= nullptr*/) : QVTKOpenGLNativeWidget(parent)
{
	m_renderer = vtkSmartPointer<vtkRenderer>::New();
	this->renderWindow()->AddRenderer(m_renderer);


	// 	distanceWidget = vtkSmartPointer<vtkDistanceWidget>::New();
	// 	distanceWidget->SetInteractor(interactor());
	// 	//必要时使用观察者/命令模式创建回调函数(此处没用)
	// 	//创建几何表达实体。用SetRepresentation()把事件与Widget关联起来
	// 	//或者使用Widget默认的几何表达实体
	// 	distanceWidget->CreateDefaultRepresentation();
	// 	static_cast<vtkDistanceRepresentation*>(distanceWidget->GetRepresentation())
	// 		->SetLabelFormat("%-#6.3g mm");
	// 	auto bidiCallback = vtkSmartPointer<distanceCallback>::New();
	// 	distanceWidget->AddObserver(vtkCommand::InteractionEvent, bidiCallback);
	// 	//激活Widget
	// 	distanceWidget->On();
}

void VolumeWidget::setSliceExtractor(SliceExtractor* dm)
{
	m_dataModel = dm->getDataModel();
	connect(dm, &SliceExtractor::sigAxesReset, this, &VolumeWidget::updateAxesPose);
	connect(dm, &SliceExtractor::sigAxesChanged, this, &VolumeWidget::updateAxesPose);

	auto outlineActor = makeOutline();
	auto implant = makeImplant();
	auto axes = makeAxes();
	auto volume = makeVolume();

	m_renderer->AddActor(implant);
	m_renderer->AddActor(axes);
	m_renderer->AddActor(outlineActor);
	m_renderer->AddVolume(volume);

	m_renderer->ResetCamera();

	this->renderWindow()->Render();
}

void VolumeWidget::updateAxesPose()
{
	auto mat = m_dataModel->getModelMat();
	m_axesPose->SetMatrix(mat);
	this->renderWindow()->Render();
}

vtkSmartPointer<vtkActor> VolumeWidget::makeOutline() const
{
	auto outlineFilter = vtkSmartPointer<vtkOutlineFilter>::New();
	outlineFilter->SetInputData(m_dataModel->getImageData());
	outlineFilter->Update();

	vtkNew<vtkPolyDataMapper> outlineMapper;
	outlineMapper->SetInputData(outlineFilter->GetOutput());

	auto outlineActor = vtkSmartPointer<vtkActor>::New();
	outlineActor->SetMapper(outlineMapper);
	outlineActor->GetProperty()->SetColor(0.5, 0.5, 0.5);
	outlineActor->GetProperty()->SetLineWidth(1);

	return outlineActor;
}

vtkSmartPointer<vtkAxesActor> VolumeWidget::makeAxes()
{
	auto axes = vtkSmartPointer<vtkAxesActor>::New();
	auto center = m_dataModel->getImageData()->GetCenter();
	axes->SetTotalLength(100, 100, 100);	//轴的长度
	axes->SetShaftType(0);					//旋转轴的类型：圆柱体、线或者自定义(用于显示)
	axes->SetAxisLabels(1);					//是否显示标签
	axes->SetCylinderRadius(0.02);			//设置轴的半径(用于显示)

	m_axesPose = vtkSmartPointer<vtkTransform>::New();
	m_axesPose->PostMultiply();
	m_axesPose->Translate(center);
	axes->SetUserTransform(m_axesPose);

	return axes;
}

vtkSmartPointer<vtkVolume> VolumeWidget::makeVolume() const
{
	auto mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	mapper->SetInputData(m_dataModel->getImageData());


	vtkNew<vtkColorTransferFunction> colorFun;
	vtkNew<vtkPiecewiseFunction> opacityFun;

	colorFun->AddRGBPoint(-3024, 0, 0, 0, 0.5, 0.0);
	colorFun->AddRGBPoint(-16, 0.73, 0.25, 0.30, 0.49, .61);
	colorFun->AddRGBPoint(641, .90, .82, .56, .5, 0.0);
	colorFun->AddRGBPoint(3071, 1, 1, 1, .5, 0.0);

	opacityFun->AddPoint(-3024, 0, 0.5, 0.0);
	opacityFun->AddPoint(-16, 0, .49, .61);
	opacityFun->AddPoint(641, .72, .5, 0.0);
	opacityFun->AddPoint(3071, .71, 0.5, 0.0);

	// 	// x-ray
	// 	colorFun->AddRGBPoint(-1990, 1, 1, 1);
	// 	colorFun->AddRGBPoint(2105, 1, 1, 1);
	// 
	// 	opacityFun->AddPoint(-1900, 0);
	// 	opacityFun->AddPoint(-92.66, 0);
	// 	opacityFun->AddPoint(1049.50, 0.05);
	// 	opacityFun->AddPoint(2105, 0.05);

	auto property = vtkSmartPointer<vtkVolumeProperty>::New();
	property->SetColor(colorFun);
	property->SetScalarOpacity(opacityFun);

	auto volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(mapper);
	volume->SetProperty(property);

	return volume;
}

vtkSmartPointer<vtkActor> VolumeWidget::makeImplant() const
{
	auto polyData = m_dataModel->getImplantData("Cone");
	auto polyMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	polyMapper->SetInputData(polyData);

	vtkSmartPointer<vtkActor> polyActor = vtkSmartPointer<vtkActor>::New();
	polyActor->SetMapper(polyMapper);
	polyActor->GetProperty()->SetColor(1.0, 1.0, 0.0);

	return polyActor;
}
