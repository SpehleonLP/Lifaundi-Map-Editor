#include "exportoptions.h"
#include "ui_exportoptions.h"
#include "src/backgroundimage.h"
#include "src/metaroom.h"
#include <fstream>
#include <QFileInfo>

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

void  ExportOptions::save(Metaroom* mta, std::string const& path, const std::vector<std::string> & track_list)
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

	std::unique_ptr<uint8_t[]> saved(new uint8_t[(mta->noFaces() + 7)/8]);
	memset(&saved[0], 0, (mta->noFaces() + 7)/8);

	for(uint32_t i : mta->range())
	{
		int l{-1}, r, tl, tr, bl, br;

		for(int j = 0; j < 4; ++j)
		{
			if(mta->_verts[i][j]      .x == mta->_verts[i][(j+1)%4].x
			&& mta->_verts[i][(j+2)%4].x == mta->_verts[i][(j+3)%4].x
			&& mta->_verts[i][j      ].x != mta->_verts[i][(j+2)%4].x)
			{
				l = mta->_verts[i][j      ].x + offset.x;
				r = mta->_verts[i][(j+2)%4].x + offset.x;

				tl = -std::max(mta->_verts[i][j      ].x,  mta->_verts[i][(j+1)%4].x) + offset.y;
				bl = -std::min(mta->_verts[i][j      ].x,  mta->_verts[i][(j+1)%4].x) + offset.y;

				tr = -std::max(mta->_verts[i][j + (j+2)%4].x,  mta->_verts[i][(j+3)%4].x) + offset.y;
				br = -std::min(mta->_verts[i][j + (j+2)%4].x,  mta->_verts[i][(j+3)%4].x) + offset.y;

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
			fprintf(file, "    rtyp va00 %i\n", mta->_roomType[i]);

			if(mta->_music[i] >= 0)
			{
				fprintf(file, "    rmsc %i %i \"%s\\\\%s\"\n", (l + r) / 2, (tl+tr+bl+br)/4, mmsc.c_str(), track_list[mta->_music[i]].c_str() );
			}
			else
			{
				fprintf(file, "    rmsc %i %i \"\"\n", (l + r) / 2, (tl+tr+bl+br)/4);
			}

			fprintf(file, "    setv game \"map_tmp_%u\" va00\n", i);
		}
	}

	fprintf(file, "\n");

	std::vector<QuadTree::Door>     doors;
	std::vector<QuadTree::DoorList> indices;

	mta->_tree.GetWriteDoors(doors, indices);

	int write_count{0};

	for(uint32_t i = 0; i < indices.size(); ++i)
	{
		if(!(saved[i/8] & (1 << i)))
			continue;

		auto j   = &doors[indices[i].index];
		auto end = j + indices[i].length;

		for(; j < end; ++j)
		{
			if(j->face > (int32_t)i
			&& (saved[j->face/8] & (1 << j->face)))
			{
				if(++write_count % 10 == 0)
					fprintf(file, "\n");

				fprintf(file, "perm game \"map_tmp_%u\" game \"map_tmp_%u\" %i\n", i, j->face, j->perm);
			}
		}
	}


	fprintf(file, "\n");

	write_count = 0;
//clear game variables
	for(uint32_t i : mta->range())
	{
		if(!(saved[i/8] & (1 << (i%8))))
			continue;

		if(++write_count % 10 == 0)
			fprintf(file, "\n");

		fprintf(file, "delg \"map_tmp_%u\"\n", i);
	}

	fclose(file);
}

void ExportOptions::configure(BackgroundImage* img)
{
	if(img != nullptr)
	{
		QFileInfo path(img->GetFilename().c_str());

		ui->background->setText(path.fileName());

		ui->width->setMinimum(img->width() - 128);
		ui->width->setMaximum(img->width());

		ui->height->setMinimum(img->height() - 128);
		ui->height->setMaximum(img->height());

		center.x = img->width()/2;
		center.y = img->height()/2;
	}
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
