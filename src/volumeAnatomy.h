#pragma once
#include <memory>

class Widget;
class DataModel;
class SliceExtractor;
class volumeAnatomy
{
public:
	volumeAnatomy();
	~volumeAnatomy();

	void setup();

private:
	std::unique_ptr<Widget> m_widget = nullptr;
	std::unique_ptr<DataModel> m_dataModel = nullptr;
	std::unique_ptr<SliceExtractor> m_extracor = nullptr;
};

