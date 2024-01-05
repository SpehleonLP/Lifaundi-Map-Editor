#ifndef COLORPROGRESSINDICATOR_H
#define COLORPROGRESSINDICATOR_H
#include "src/color_rooms.h"
#include <QMainWindow>

namespace Ui {
class ColorProgressIndicator;
}

class ColorProgressIndicator : public QMainWindow
{
	Q_OBJECT

public:
	explicit ColorProgressIndicator(QWidget *parent, std::shared_ptr<ColorProgress> progress);
	~ColorProgressIndicator();



	void Update();

private:
	Ui::ColorProgressIndicator *ui;
	std::shared_ptr<ColorProgress> _progress;
};

#endif // COLORPROGRESSINDICATOR_H
