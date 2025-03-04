#include <vtkSmartPointer.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleImage.h>


class vtkActor;
class PlaneWidget;
class SlicePickInteractorStyle : public vtkInteractorStyleImage
{
public:
	static SlicePickInteractorStyle* New();
	vtkTypeMacro(SlicePickInteractorStyle, vtkInteractorStyleImage);

	SlicePickInteractorStyle();

	void SetWidget(PlaneWidget* w);

	virtual void OnLeftButtonDown() override;

	virtual void OnLeftButtonUp() override;

	virtual void OnKeyDown() override;

	virtual void OnKeyUp() override;
	
	virtual void OnMouseMove() override;

protected:
	void MoveAxes(int x, int y);
	void RotateAxes(int pre_x, int pre_y, int x, int y);

protected:
	PlaneWidget* m_widget = nullptr;
	vtkActor* m_pickedActor = nullptr;
	bool m_ctrlPressed = false;
	int m_lastXPos = 0;
	int m_lastYPos = 0;
};
