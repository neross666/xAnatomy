#include <vtkImageReslice.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrix3x3.h>
#include <vtkImageData.h>
#include <vtkTransform.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkPolyData.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <qDebug>
#include "SliceExtractor.h"
#include "DataModel.h"


void SliceExtractor::setDataModel(DataModel* dm)
{
	m_dataModel = dm;
}

DataModel* SliceExtractor::getDataModel()
{
	return m_dataModel;
}

vtkSmartPointer<vtkMatrix4x4> SliceExtractor::getResliceAxes(BaseStruct::PlaneType t) const
{
	auto resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();

	// LPS:取矢状面(LS)-冠状面(PS)-横截面(LP)
	// RAS:取矢状面(AS)-冠状面(RS)-横截面(RA)

	auto origin = m_dataModel->getAxesOrigin();
	auto rotateMat = m_dataModel->getRotateMat();
	int s = static_cast<int>(t);
	for (int i = 0; i < 3; i++)
	{
		resliceAxes->SetElement(i, 0, rotateMat->GetElement(i, (s + 0) % 3));
		resliceAxes->SetElement(i, 1, rotateMat->GetElement(i, (s + 1) % 3));
		resliceAxes->SetElement(i, 2, rotateMat->GetElement(i, (s + 2) % 3));
		resliceAxes->SetElement(i, 3, origin[i]);
	}

	return resliceAxes;
}

vtkSmartPointer<vtkImageData> SliceExtractor::extractSlice(BaseStruct::PlaneType t) const
{
	auto imageData = m_dataModel->getImageData();

	auto reslice = vtkSmartPointer<vtkImageReslice>::New();
	reslice->SetInputData(imageData);

	auto minv = imageData->GetScalarTypeMin();

	// 设置切片的输出维度为2D
	reslice->SetAutoCropOutput(true);
	reslice->SetBackgroundLevel(minv);
	reslice->SetOutputDimensionality(2);
	reslice->SetInterpolationModeToLinear();
	reslice->SetResliceAxes(getResliceAxes(t));

	// 创建窗宽窗位映射过滤器
	auto windowWidth = m_dataModel->getWindowWidth();
	auto windowLevel = m_dataModel->getWindowCenter();
	auto windowLevelFilter = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
	windowLevelFilter->SetInputConnection(reslice->GetOutputPort());
	windowLevelFilter->SetWindow(windowWidth);
	windowLevelFilter->SetLevel(windowLevel);
	windowLevelFilter->Update();

	return windowLevelFilter->GetOutput();
}

vtkSmartPointer<vtkPolyData> SliceExtractor::extractImplantSlice(BaseStruct::PlaneType t) const
{
	auto resliceAxes = getResliceAxes(t);
	auto polyData = m_dataModel->getImplantData(QString("Cone"));

	// 创建切割平面
	double normalInSlice[4] = { 0.0, 0.0, 1.0 ,0.0 };		// Z 设置法向量
	double originInSlice[4] = { 0.0, 0.0, 0.0 ,1.0 };		// 设置平面原点
	double normalInVolume[4];
	double originInVolume[4];
	resliceAxes->MultiplyPoint(normalInSlice, normalInVolume);
	resliceAxes->MultiplyPoint(originInSlice, originInVolume);

	auto plane = vtkSmartPointer<vtkPlane>::New();
	plane->SetNormal(normalInVolume);
	plane->SetOrigin(originInVolume);

	// 对 vtkPolyData 进行切割
	auto polyCutter = vtkSmartPointer<vtkCutter>::New();
	polyCutter->SetInputData(polyData);
	polyCutter->SetCutFunction(plane);

	// 将切割结果变换到新的坐标系下
 	// Z方向移动一小段距离，确保图形不被图像遮挡
	auto transFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	auto trans = vtkSmartPointer<vtkTransform>::New();
	trans->PostMultiply();
	auto mat_inv = vtkSmartPointer<vtkMatrix4x4>::New();
	vtkMatrix4x4::Invert(resliceAxes, mat_inv);
	trans->SetMatrix(mat_inv);
	trans->Translate(0, 0, 0.01);
	transFilter->SetTransform(trans);
	transFilter->SetInputConnection(polyCutter->GetOutputPort());
	transFilter->Update();


	return transFilter->GetOutput();
}

void SliceExtractor::setAxesOrigin(BaseStruct::PlaneType t, double origin[4])
{
	auto resliceAxes = getResliceAxes(t);

	double position[4]{ 0.0 };
	resliceAxes->MultiplyPoint(origin, position);
	qDebug() << QString("volume position   x:%1, y:%2, z:%3").arg(position[0]).arg(position[1]).arg(position[2]);
	m_dataModel->setAxesOrigin(position);

	emit sigAxesChanged(t);
}

void SliceExtractor::moveAxesOrigin(BaseStruct::PlaneType t, bool isHAxis, double offset)
{
	double nposition[4]{ 0.0 };
	auto position = m_dataModel->getAxesOrigin();

	auto rotateMat = m_dataModel->getRotateMat();
	double xAxes[3]{
	rotateMat->GetElement(0, 0),
	rotateMat->GetElement(1, 0),
	rotateMat->GetElement(2, 0) };
	double yAxes[3]{
		rotateMat->GetElement(0, 1),
		rotateMat->GetElement(1, 1),
		rotateMat->GetElement(2, 1) };
	double zAxes[3]{
		rotateMat->GetElement(0, 2),
		rotateMat->GetElement(1, 2),
		rotateMat->GetElement(2, 2) };

	switch (t)
	{
	case BaseStruct::PlaneType::Sagittal:
	{
		double* axes = isHAxis ? xAxes : yAxes;
		nposition[0] = position[0] + offset * axes[0];
		nposition[1] = position[1] + offset * axes[1];
		nposition[2] = position[2] + offset * axes[2];
		nposition[3] = 1.0;
		m_dataModel->setAxesOrigin(nposition);
	}
	break;
	case BaseStruct::PlaneType::Coronal:
	{
		double* axes = isHAxis ? yAxes : zAxes;
		nposition[0] = position[0] + offset * axes[0];
		nposition[1] = position[1] + offset * axes[1];
		nposition[2] = position[2] + offset * axes[2];
		nposition[3] = 1.0;
		m_dataModel->setAxesOrigin(nposition);
	}
	break;
	case BaseStruct::PlaneType::Axial:
	{
		double* axes = isHAxis ? zAxes : xAxes;
		nposition[0] = position[0] + offset * axes[0];
		nposition[1] = position[1] + offset * axes[1];
		nposition[2] = position[2] + offset * axes[2];
		nposition[3] = 1.0;
		m_dataModel->setAxesOrigin(nposition);
	}
	break;
	default:
		break;
	}

	emit sigAxesChanged(t);
}

void SliceExtractor::rotateAxes(BaseStruct::PlaneType t, double pre_pt[4], double cur_pt[4])
{
	auto rotateMat = m_dataModel->getRotateMat();
	auto resliceAxes = getResliceAxes(t);

	auto getDirection = [&resliceAxes](const double pt[4], double dir[3]) {
		// 获取鼠标点在体数据中的坐标
		double ptInVolume[4]{ 0.0 };
		resliceAxes->MultiplyPoint(pt, ptInVolume);

		double ori[4] = {0,0,0,1};
		double origin[4]{ 0.0 };
		resliceAxes->MultiplyPoint(ori, origin);

		// 计算方向
		for (int i = 0; i < 3; i++)
			dir[i] = ptInVolume[i] - origin[i];
		vtkMath::Normalize(dir);
	};

	double pre_dir[3]{ 0.0 };
	double cur_dir[3]{ 0.0 };
	getDirection(pre_pt, pre_dir);
	getDirection(cur_pt, cur_dir);
	
	std::unordered_map<BaseStruct::PlaneType, int> idx_map = {
		{BaseStruct::PlaneType::Sagittal, 2},
		{BaseStruct::PlaneType::Coronal, 0},
		{BaseStruct::PlaneType::Axial, 1}
	};
	double baseAxes[3]{
		rotateMat->GetElement(0, idx_map[t]),
		rotateMat->GetElement(1, idx_map[t]),
		rotateMat->GetElement(2, idx_map[t]) };
	auto angleInDegrees = calculateRotateDegrees(pre_dir, cur_dir, baseAxes);
		
	rotateAxes(t, angleInDegrees);
}

void SliceExtractor::rotateAxes(BaseStruct::PlaneType t, double theta)
{
	auto rotateMat = m_dataModel->getRotateMat();
	double xAxes[3]{
		rotateMat->GetElement(0, 0),
		rotateMat->GetElement(1, 0),
		rotateMat->GetElement(2, 0) };
	double yAxes[3]{
		rotateMat->GetElement(0, 1),
		rotateMat->GetElement(1, 1),
		rotateMat->GetElement(2, 1) };
	double zAxes[3]{
		rotateMat->GetElement(0, 2),
		rotateMat->GetElement(1, 2),
		rotateMat->GetElement(2, 2) };

	switch (t)
	{
	case BaseStruct::PlaneType::Sagittal:
	{
		double direction[3]{ 0.0 };
		auto transform = vtkSmartPointer<vtkTransform>::New();
		transform->RotateWXYZ(-theta, zAxes);		// 区分顺时针(-)和逆时针(+)		
		transform->TransformVector(xAxes, direction);

		vtkMath::Cross(zAxes, direction, yAxes);
		for (int i = 0; i < 3; i++)
		{
			rotateMat->SetElement(i, 0, direction[i]);
			rotateMat->SetElement(i, 1, yAxes[i]);
			rotateMat->SetElement(i, 2, zAxes[i]);
		}
	}
	break;
	case BaseStruct::PlaneType::Coronal:
	{
		double direction[3]{ 0.0 };
		auto transform = vtkSmartPointer<vtkTransform>::New();
		transform->RotateWXYZ(-theta, xAxes);		// 区分顺时针(-)和逆时针(+)		
		transform->TransformVector(yAxes, direction);

		vtkMath::Cross(xAxes, direction, zAxes);
		for (int i = 0; i < 3; i++)
		{
			rotateMat->SetElement(i, 0, xAxes[i]);
			rotateMat->SetElement(i, 1, direction[i]);
			rotateMat->SetElement(i, 2, zAxes[i]);
		}
	}
	break;
	case BaseStruct::PlaneType::Axial:
	{
		double direction[3]{ 0.0 };
		auto transform = vtkSmartPointer<vtkTransform>::New();
		transform->RotateWXYZ(-theta, yAxes);		// 区分顺时针(-)和逆时针(+)		
		transform->TransformVector(zAxes, direction);

		vtkMath::Cross(yAxes, direction, xAxes);
		for (int i = 0; i < 3; i++)
		{
			rotateMat->SetElement(i, 0, xAxes[i]);
			rotateMat->SetElement(i, 1, yAxes[i]);
			rotateMat->SetElement(i, 2, direction[i]);
		}
	}
	break;
	default:
		break;
	}

	qDebug() << "rotate: " << theta;
	emit sigAxesChanged(t, theta);
}

double SliceExtractor::calculateRotateDegrees(const double* oldAxes, double* newAxes, const double* baseAxis)
{
	// 求出旋转的角度,通过角度确定旋转轴的方向
	double angleInRadians = vtkMath::AngleBetweenVectors(oldAxes, newAxes);
	double angleInDegrees = vtkMath::DegreesFromRadians(angleInRadians);

	double t[3]{ 0.0 };
	vtkMath::Cross(oldAxes, newAxes, t);
	double dotProduct = vtkMath::Dot(t, baseAxis);
	if (dotProduct > 0)	// 检测顺时针旋转还是逆时针旋转,这里和baseAxis观测向量有关
		angleInDegrees = -angleInDegrees;
	return angleInDegrees;
}

void SliceExtractor::resetAxes()
{
	m_dataModel->resetAxes();

	emit sigAxesReset();
}

void SliceExtractor::playAxes(BaseStruct::PlaneType t)
{
	double nposition[4]{ 0.0 };
	auto position = m_dataModel->getAxesOrigin();

	auto rotateMat = m_dataModel->getRotateMat();
	double xAxes[3]{
	rotateMat->GetElement(0, 0),
	rotateMat->GetElement(1, 0),
	rotateMat->GetElement(2, 0) };
	double yAxes[3]{
		rotateMat->GetElement(0, 1),
		rotateMat->GetElement(1, 1),
		rotateMat->GetElement(2, 1) };
	double zAxes[3]{
		rotateMat->GetElement(0, 2),
		rotateMat->GetElement(1, 2),
		rotateMat->GetElement(2, 2) };

	double offset = 1.0;
	switch (t)
	{
	case BaseStruct::PlaneType::Sagittal:
	{
		// 沿着Z轴方向移动坐标轴原点
		nposition[0] = position[0] + offset * zAxes[0];
		nposition[1] = position[1] + offset * zAxes[1];
		nposition[2] = position[2] + offset * zAxes[2];
		nposition[3] = 1.0;

		clampPosition(nposition, BaseStruct::PlaneType::Coronal);

		m_dataModel->setAxesOrigin(nposition);

		emit sigAxesChanged(t);
	}
	break;
	case BaseStruct::PlaneType::Coronal:
	{
		// 沿着X轴方向移动坐标轴原点
		nposition[0] = position[0] + offset * xAxes[0];
		nposition[1] = position[1] + offset * xAxes[1];
		nposition[2] = position[2] + offset * xAxes[2];
		nposition[3] = 1.0;

		clampPosition(nposition, BaseStruct::PlaneType::Axial);

		m_dataModel->setAxesOrigin(nposition);

		emit sigAxesChanged(t);
	}
	break;
	case BaseStruct::PlaneType::Axial:
	{
		// 沿着Y轴方向移动坐标轴原点
		nposition[0] = position[0] + offset * yAxes[0];
		nposition[1] = position[1] + offset * yAxes[1];
		nposition[2] = position[2] + offset * yAxes[2];
		nposition[3] = 1.0;

		clampPosition(nposition, BaseStruct::PlaneType::Sagittal);

		m_dataModel->setAxesOrigin(nposition);

		emit sigAxesChanged(t);
	}
	break;
	default:
		break;
	}
}

void SliceExtractor::clampPosition(double nposition[4], BaseStruct::PlaneType t)
{
	auto resliceAxes = getResliceAxes(t);

	auto slice = extractSlice(t);
	double* bounds = slice->GetBounds();

	double lb[4]{ 0.0, bounds[2], 0.0, 1.0 };		// 使用垂直方向的范围
	double lbound[4]{ 0.0, 0.0, 0.0, 1.0 };
	resliceAxes->MultiplyPoint(lb, lbound);

	double ub[4]{ 0.0, bounds[3], 0.0, 1.0 };
	double ubound[4]{ 0.0, 0.0, 0.0, 1.0 };
	resliceAxes->MultiplyPoint(ub, ubound);

	double v0[3] = { ubound[0] - lbound[0],ubound[1] - lbound[1],ubound[2] - lbound[2] };
	double v1[3] = { ubound[0] - nposition[0],ubound[1] - nposition[1],ubound[2] - nposition[2] };
	if (vtkMath::Dot(v0, v1) < 0)
	{
		nposition[0] = lbound[0];
		nposition[1] = lbound[1];
		nposition[2] = lbound[2];
	}
}

