#include "exportoptions.h"
#include "ui_exportoptions.h"
#include "src/backgroundimage.h"
#include "src/metaroom.h"
#include <fstream>

ExportOptions::ExportOptions(QWidget *parent) :
QDialog(parent),
ui(new Ui::ExportOptions)
{
	ui->setupUi(this);
}

ExportOptions::~ExportOptions()
{
	delete ui;
}

void  ExportOptions::save(Metaroom* mta, std::string const& path)
{
	const auto offset = room_offset();
	const auto mmsc   = music();
	const auto blk    = filename();
	const auto coord  = metaroom_coordinates();

	FILE * file = fopen(path.c_str(), "w");

	if(!file)
		throw std::system_error(errno, std::system_category(), path);

	fprintf(file, "setv va01 addm %i %i %i %i \"%s\"\n", coord.x, coord.y, coord.z, coord.w, blk.c_str());
	fprintf(file, "mmsc %i %i \"\"\n", coord.x + coord.z / 2, coord.y + coord.w / 2);


	std::unique_ptr<uint8_t[]> saved(new uint8_t[(mta->size() + 7)/8]);
	memset(&saved[0], 0, (mta->size() + 7)/8);

	for(uint32_t i = 0; i < mta->size(); ++i)
	{
		int l{-1}, r, tl, tr, bl, br;

		for(int j = 0; j < 4; ++j)
		{
			if(mta->m_verts[i*4+j]      .x == mta->m_verts[i*4+(j+1)%4].x
			&& mta->m_verts[i*4+(j+2)%4].x == mta->m_verts[i*4+(j+3)%4].x
			&& mta->m_verts[i*4+j      ].x != mta->m_verts[i*4+(j+2)%4].x)
			{
				l = mta->m_verts[i*4+j      ].x + offset.x;
				r = mta->m_verts[i*4+(j+2)%4].x + offset.x;

				tl = -std::max(mta->m_verts[i*4+j      ].x,  mta->m_verts[i*4+(j+1)%4].x) + offset.y;
				bl = -std::min(mta->m_verts[i*4+j      ].x,  mta->m_verts[i*4+(j+1)%4].x) + offset.y;

				tr = -std::max(mta->m_verts[i*4+j + (j+2)%4].x,  mta->m_verts[i*4+(j+3)%4].x) + offset.y;
				br = -std::min(mta->m_verts[i*4+j + (j+2)%4].x,  mta->m_verts[i*4+(j+3)%4].x) + offset.y;

				if(l > r)
				{
					std::swap(l, r);
					std::swap(tl, tr);
					std::swap(bl, br);
				}

				break;
			}
		}

		if(l > -1)
		{
			saved[i/8] |= 1 << (i % 8);
			fprintf(file, "  setv va00 addr va01 %i %i %i %i %i %i\n", l, r, tl, tr, bl, br);
			fprintf(file, "    rtyp va00 %i\n", mta->m_roomType[i]);
			fprintf(file, "    rmsc %i %i \"\"\n", (l + r) / 2, (tl+tr+bl+br)/4);
			fprintf(file, "    setv game \"map_tmp_%u\" va00\n", i);
		}
	}

//clear game variables
	for(uint32_t i = 0; i < mta->size(); ++i)
	{
		fprintf(file, "delg \"map_tmp_%u\"\n", i);

		if(i > 0 && i % 10 == 0)
			fprintf(file, "\n");
	}

	fclose(file);
}

void ExportOptions::configure(BackgroundImage* img)
{
	ui->background->setText(img->GetFilename().c_str());

	ui->width->setMinimum(img->width() - 128);
	ui->width->setMaximum(img->width());

	ui->height->setMinimum(img->height() - 128);
	ui->height->setMaximum(img->height());

	center.x = img->width()/2;
	center.y = img->height()/2;
}

std::string ExportOptions::filename() const
{
	return ui->background->text().toStdString();
}

std::string ExportOptions::music() const
{
	return ui->background->text().toStdString();
}

glm::ivec2  ExportOptions::room_offset() const
{
	return glm::ivec2(ui->posx->value(), ui->posy->value()) + center;
}

glm::ivec4  ExportOptions::metaroom_coordinates() const
{
	return glm::ivec4(ui->posx->value(), ui->posy->value(),
					  ui->width->value(), ui->height->value());
}
