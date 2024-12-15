#include <QApplication>
#include <QFile>
#include "volumeAnatomy.h"

void loadQssFile()
{
	QFile file(":/qdarkgraystyle/style.qss");
	if (file.open(QFile::ReadOnly))
	{
		QString styleSheet = QLatin1String(file.readAll());
		qApp->setStyleSheet(styleSheet);
	}
	file.close();
}

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	loadQssFile();

	volumeAnatomy anatomy;
	anatomy.setup();

	app.exec();

	return 0;
}

