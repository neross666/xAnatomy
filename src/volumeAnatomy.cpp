#include "volumeAnatomy.h"
#include "DataModel.h"
#include "SliceExtractor.h"
#include "widget.h"


volumeAnatomy::volumeAnatomy()
{
	m_dataModel = std::make_unique<DataModel>();
	m_extracor = std::make_unique<SliceExtractor>();
	m_widget = std::make_unique<Widget>();
}

volumeAnatomy::~volumeAnatomy() = default;

void volumeAnatomy::setup()
{
	QString dataDir = QString("%1dcm").arg(DATA_DIR);
	m_dataModel->readDicomFiles(dataDir);
	m_dataModel->readImplantFiles(dataDir);
	m_extracor->setDataModel(m_dataModel.get());
	m_widget->setSliceExtractor(m_extracor.get());
	m_widget->show();
}
