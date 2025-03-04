#pragma once
#include <QObject>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include "BaseStruct.h"

class DataModel;
class SliceExtractor : public QObject
{
	Q_OBJECT

public:
	void setDataModel(DataModel* dm);
	DataModel* getDataModel();

	vtkSmartPointer<vtkMatrix4x4> getResliceAxes(BaseStruct::PlaneType t) const;
	vtkSmartPointer<vtkImageData> extractSlice(BaseStruct::PlaneType t) const;
	vtkSmartPointer<vtkPolyData> extractImplantSlice(BaseStruct::PlaneType t) const;

	// origin:在切片中的坐标值
	void setAxesOrigin(BaseStruct::PlaneType t, double origin[4]);
	void moveAxesOrigin(BaseStruct::PlaneType t, bool isHAxis, double offset);

	void rotateAxes(BaseStruct::PlaneType t, double theta);
	void rotateAxes(BaseStruct::PlaneType t, double pre_origin[4], double origin[4]);

	void resetAxes();

	void playAxes(BaseStruct::PlaneType t);


private:
	void clampPosition(double nposition[4], BaseStruct::PlaneType t);
	double calculateRotateDegrees(const double* oldAxes, double* newAxes, const double* baseAxis);

signals:
	void sigAxesReset();
	void sigAxesChanged(BaseStruct::PlaneType t, double theta = 0.0);

private:
	DataModel* m_dataModel = nullptr;
};
