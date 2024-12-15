#include <vtkSmartPointer.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleImage.h>


class vtkActor;
class PlaneWidget;
class MeasureInteractorStyle : public vtkInteractorStyleImage
{
public:
	static MeasureInteractorStyle* New();
	vtkTypeMacro(MeasureInteractorStyle, vtkInteractorStyleImage);

	MeasureInteractorStyle();

	void SetWidget(PlaneWidget* w);

	virtual void OnLeftButtonDoubleClick() override;


protected:

protected:
	PlaneWidget* m_widget = nullptr;
};
