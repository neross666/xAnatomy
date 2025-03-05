#include "DataModel.h"
#include <qDebug>
#include <QDir>
#include <vtkDICOMReader.h>
#include <vtkDICOMMetaData.h>
#include <vtkStringArray.h>
#include <vtkDICOMMetaData.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrix3x3.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkConeSource.h>
#include <vtkPolyData.h>
#include <vtkDICOMToRAS.h>


DataModel::DataModel()
{
	m_rot = vtkSmartPointer<vtkMatrix3x3>::New();
	m_rot->Identity();
}

vtkImageData* DataModel::getImageData() const
{
	return m_imageData.Get();
}

double* DataModel::getAxesOrigin()
{
	return m_position;
}

vtkMatrix3x3* DataModel::getRotateMat() const
{
	return m_rot.Get();
}

void DataModel::setRotateMat(vtkMatrix3x3* mat)
{
	m_rot->DeepCopy(mat);
}

vtkSmartPointer<vtkMatrix4x4> DataModel::getModelMat()
{
	auto modelMat = vtkSmartPointer<vtkMatrix4x4>::New();
	for (int i = 0; i < 3; i++)
	{
		modelMat->SetElement(i, 0, m_rot->GetElement(i, 0));
		modelMat->SetElement(i, 1, m_rot->GetElement(i, 1));
		modelMat->SetElement(i, 2, m_rot->GetElement(i, 2));
		modelMat->SetElement(i, 3, m_position[i]);
	}
	return modelMat;
}

void DataModel::setModelMat(const vtkMatrix4x4* modelMat)
{
	for (int i = 0; i < 3; i++)
	{
		m_rot->SetElement(i, 0, modelMat->GetElement(i, 0));
		m_rot->SetElement(i, 1, modelMat->GetElement(i, 1));
		m_rot->SetElement(i, 2, modelMat->GetElement(i, 2));
		m_position[i] = modelMat->GetElement(i, 3);
	}
}

void DataModel::resetAxes()
{
	const double* center = m_imageData->GetCenter();// 体数据的中心作为输出切片的(0,0,0)点
	m_position[0] = center[0];
	m_position[1] = center[1];
	m_position[2] = center[2];
	m_rot->Identity();
}

vtkDICOMMetaData* DataModel::getMetaData() const
{
	return m_metaData.Get();
}

vtkPolyData* DataModel::getImplantData(const QString& name) const
{
	const auto it = m_implantSet.find(name.toStdString());
	if (it == m_implantSet.end())
		return nullptr;

	return it->second.Get();
}

double DataModel::getWindowWidth() const
{
	return m_metaData->Get(DC::WindowWidth).AsDouble();
}

double DataModel::getWindowCenter() const
{
	return m_metaData->Get(DC::WindowCenter).AsDouble();
}

/*
* DICOM数据是LPS坐标系，reader->GetPatientMatrix()标识XYZ-LPS之间的变换矩阵
* vtkDICOMToRAS可以将LPS坐标系转换为RAS坐标系，允许行列重排，即确保XYZ与RAS一一对应
* 1. 变换到LPS坐标系下，然后再取矢状面(LP)-冠状面(PS)-横截面(LS)
* 2. 变换到RAS坐标系下，然后再取矢状面(AS)-冠状面(RS)-横截面(RA)
*/
void DataModel::readDicomFiles(const QString& dicomFilePath)
{
	auto files = getDcmFileList(dicomFilePath);
	auto reader = vtkSmartPointer<vtkDICOMReader>::New();
	reader->SetFileNames(files);
	reader->Update();

	auto pmat = reader->GetPatientMatrix();
	pmat->Print(std::cout);

	//vtkNew<vtkDICOMToRAS> converter;
	//converter->SetInputConnection(reader->GetOutputPort());
	//converter->SetPatientMatrix(reader->GetPatientMatrix());
	//converter->SetAllowRowReordering(true);
	//converter->SetAllowColumnReordering(true);
	//converter->UpdateMatrix();
	//converter->Update();

	m_metaData = reader->GetMetaData();
	m_imageData = reader->GetOutput();	
	//m_imageData = converter->GetOutput();

	m_imageData->GetCenter(m_position);// 体数据的中心作为输出切片的(0,0,0)点
}


void DataModel::readImplantFiles(const QString& implantFilePath)
{
	auto center = m_imageData->GetCenter();
	// 模拟implant数据
	auto coneSource = vtkSmartPointer<vtkConeSource>::New();
	coneSource->SetRadius(10.0);
	coneSource->SetHeight(30.0);
	coneSource->SetDirection(0, 1, 0);
	coneSource->SetResolution(20);
	coneSource->SetCenter(center[0], center[1], center[2]);
	coneSource->Update();

	m_implantSet["Cone"] = vtkSmartPointer<vtkPolyData>::New();
	m_implantSet["Cone"]->DeepCopy(coneSource->GetOutput());
}

void DataModel::setAxesOrigin(double origin[3])
{
	m_position[0] = origin[0];
	m_position[1] = origin[1];
	m_position[2] = origin[2];
	m_position[3] = 1.0;
}

vtkSmartPointer<vtkStringArray> DataModel::getDcmFileList(const QString& dicomFilePath)
{
	QDir dir(dicomFilePath);
	QStringList dcmFiles = dir.entryList(QStringList{ "*.dcm" }, QDir::Files);
	auto files = vtkSmartPointer<vtkStringArray>::New();
	for (const auto& name : dcmFiles)
	{
		auto str = (dicomFilePath + "/" + name).toStdString();
		files->InsertNextValue(str);
	}
	return files;
}