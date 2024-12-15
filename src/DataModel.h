#pragma once
#include <QObject>
#include <QString>
#include <unordered_map>
#include <vtkSmartPointer.h>

class vtkStringArray;
class vtkImageData;
class vtkDICOMMetaData;
class vtkMatrix4x4;
class vtkMatrix3x3;
class vtkPolyData;

class DataModel : public QObject
{
	Q_OBJECT

public:
	DataModel();
	~DataModel() = default;

	double getWindowWidth() const;
	double getWindowCenter() const;
	vtkImageData* getImageData() const;
	vtkDICOMMetaData* getMetaData() const;

	vtkPolyData* getImplantData(const QString& name) const;

	double* getAxesOrigin();
	void setAxesOrigin(double origin[3]);
	vtkMatrix3x3* getRotateMat() const;
	void setRotateMat(vtkMatrix3x3* mat);
	vtkSmartPointer<vtkMatrix4x4> getModelMat();
	void setModelMat(const vtkMatrix4x4* mat);
	void resetAxes();

	void readDicomFiles(const QString& dicomFilePath);
	void readImplantFiles(const QString& implantFilePath);

private:
	vtkSmartPointer<vtkStringArray> getDcmFileList(const QString& dicomFilePath);


private:
	vtkSmartPointer<vtkDICOMMetaData> m_metaData = nullptr;
	vtkSmartPointer<vtkImageData> m_imageData = nullptr;
	std::unordered_map<std::string, vtkSmartPointer<vtkPolyData>> m_implantSet;

	double m_position[4] = { 0.0, 0.0, 0.0, 1.0 };
	vtkSmartPointer<vtkMatrix3x3> m_rot = nullptr;
};