#ifndef EXPORTOPTIONS_H
#define EXPORTOPTIONS_H
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <QDialog>

class MainWindow;
class BackgroundImage;
class Metaroom;

namespace Ui {
class ExportOptions;
}

class ExportOptions : public QDialog
{
	Q_OBJECT

public:
	explicit ExportOptions(QWidget *parent = nullptr);
	~ExportOptions();

	void save(Metaroom*, std::string const& path);
	void configure(BackgroundImage*);

	std::string filename() const;
	std::string music() const;
	glm::ivec2  room_offset() const;
	glm::ivec4  metaroom_coordinates() const;


private:
	glm::ivec2         center{0, 0};
	Ui::ExportOptions *ui;
};

#endif // EXPORTOPTIONS_H
