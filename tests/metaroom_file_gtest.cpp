#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/document.h"
#include "src/Shaders/shaders.h"
#include "qt-gl/initialize_gl.h"
#include <QFileInfo>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_5_Core>
#include <QTemporaryDir>
#include <gtest/gtest.h>
#include <fstream>
#include <iterator>
#include <map>
#include <tuple>

// Round-trips real map files through the same Document::LoadFile/SaveFile
// path the File menu uses. A MainWindow is required because the track list
// lives in its music combo box, but it is never shown. Background uploads get
// their GL context from a hidden offscreen surface instead of the view widget.
namespace
{

const QString kData = QStringLiteral(MAPEDITOR_TEST_DATA);

// Everything the file format persists, keyed by the room's position in the
// live list so face ids that differ only by reindexing still compare equal.
struct RoomRecord
{
	std::array<glm::ivec2, 4> verts;
	uint32_t     gravity;
	uint8_t      type;
	std::string  music;
	uint32_t     directionalShade;
	uint8_t      ambientShade;
	glm::u8vec4  audio;
	glm::u16vec2 depth;

	auto tie() const
	{
		return std::tie(verts, gravity, type, music, directionalShade,
						ambientShade, audio, depth);
	}
	bool operator==(RoomRecord const& o) const { return tie() == o.tie(); }
};

struct Snapshot
{
	std::vector<RoomRecord> rooms;
	std::map<std::pair<int,int>, float> permeabilities;
};

Snapshot Capture(MainWindow & w, Metaroom const& mta)
{
	Snapshot s;
	std::map<int, int> ordinal;

	for(auto face : mta.range())
	{
		ordinal[face] = s.rooms.size();

		int track = mta._music[face];
		s.rooms.push_back({
			mta._verts[face],
			mta._gravity[face],
			mta._roomType[face],
			track < 0? std::string() : w.ui->room_music->itemText(track).toStdString(),
			mta._directionalShade[face],
			mta._ambientShade[face],
			mta._audio[face],
			mta._depth[face],
		});
	}

	for(auto const& [key, perm] : mta._permeabilities)
	{
		int a = key & 0xFFFFFFFF;
		int b = key >> 32;
		if(ordinal.count(a) && ordinal.count(b))
			s.permeabilities[std::minmax(ordinal[a], ordinal[b])] = perm;
	}

	return s;
}

std::string ReadBytes(QString const& path)
{
	std::ifstream f(path.toStdString(), std::ios::binary);
	return {std::istreambuf_iterator<char>(f), {}};
}

// What GLViewWidget::initializeGL sets up, minus the widget.
struct OffscreenGL : QOpenGLFunctions_4_5_Core
{
	QOffscreenSurface surface;
	QOpenGLContext    context;
	std::unique_ptr<Shaders> shaders;

	bool Create()
	{
		surface.create();
		if(!context.create() || !context.makeCurrent(&surface)
		|| !initializeOpenGLFunctions())
			return false;

		OpenGL.Initialize(this);
		shaders = std::make_unique<Shaders>(this);
		return true;
	}

	~OffscreenGL()
	{
		if(shaders && context.makeCurrent(&surface))
			shaders.reset();
	}
};

// One window for the whole run, deliberately never destroyed: its GL child
// widgets' destructors assume they were shown and initialized.
MainWindow & Window()
{
	static MainWindow * w = new MainWindow;
	return *w;
}

class MetaroomFileTest : public ::testing::TestWithParam<const char*>
{
protected:
	OffscreenGL gl;
	MainWindow & w = Window();
	QTemporaryDir tmp;

	void TearDown() override
	{
		ReleaseDocument();
		w.document = std::make_unique<Document>(&w);
	}

	QString Map() const { return kData + "/" + GetParam(); }

	// Without a context no GL objects were ever created, so there is nothing
	// to release (and Release would dereference the null Shaders).
	void ReleaseDocument()
	{
		if(w.document && gl.shaders)
			w.document->Release(gl.shaders.get());
	}

	Document & NewDocument()
	{
		ReleaseDocument();
		w.document = std::make_unique<Document>(&w);
		return *w.document;
	}

	Snapshot Load(Shaders * shaders, QString const& path)
	{
		auto & doc = *w.document;
		doc.LoadFile(shaders, QFileInfo(path), true, false, w.GetBackgroundLayer());
		EXPECT_EQ(doc.m_metaroom.TestTreeSymmetry(), "") << path.toStdString();
		return Capture(w, doc.m_metaroom);
	}

	// Load -> save -> reload must preserve every room, and a second save must
	// reproduce the first byte for byte.
	void RoundTrip(Shaders * shaders, Snapshot const& original)
	{
		auto first  = tmp.filePath("first.lf_mta");
		auto second = tmp.filePath("second.lf_mta");

		ASSERT_TRUE(w.document->SaveFile(QFileInfo(first)));

		NewDocument();
		auto reloaded = Load(shaders, first);

		ASSERT_EQ(reloaded.rooms.size(), original.rooms.size());
		for(size_t i = 0; i < original.rooms.size(); ++i)
			EXPECT_TRUE(reloaded.rooms[i] == original.rooms[i]) << "room " << i;
		EXPECT_EQ(reloaded.permeabilities, original.permeabilities);

		ASSERT_TRUE(w.document->SaveFile(QFileInfo(second)));
		EXPECT_TRUE(ReadBytes(first) == ReadBytes(second))
			<< "a reloaded map does not save byte-identically";
	}
};

}

TEST_P(MetaroomFileTest, RoundTripsWithoutBackground)
{
	NewDocument();
	auto original = Load(nullptr, Map());
	ASSERT_FALSE(original.rooms.empty());

	RoundTrip(nullptr, original);
}

TEST_P(MetaroomFileTest, RoundTripsWithBackground)
{
	const QString background = kData + "/desert.lf_bck";
	if(!QFileInfo::exists(background))
		GTEST_SKIP() << "SKIPPED: " << background.toStdString()
					 << " is missing (git-ignored; copy it from"
						" Lifaundi/Development/DesertOasis/Metaroom)";

	NewDocument();
	auto without = Load(nullptr, Map());

	if(!gl.Create())
		GTEST_SKIP() << "SKIPPED: no GL 4.5 context on platform "
					 << QGuiApplication::platformName().toStdString();
	Shaders * shaders = gl.shaders.get();

	auto & doc = NewDocument();
	doc.LoadFile(shaders, QFileInfo(background), false, true, w.GetBackgroundLayer());
	ASSERT_GT(doc.width(), 0) << "background did not load";

	// Loading rooms under a background must not move them.
	auto with = Load(shaders, Map());
	ASSERT_EQ(with.rooms.size(), without.rooms.size());
	for(size_t i = 0; i < without.rooms.size(); ++i)
		EXPECT_TRUE(with.rooms[i] == without.rooms[i]) << "room " << i;

	RoundTrip(shaders, with);
}

INSTANTIATE_TEST_SUITE_P(Maps, MetaroomFileTest,
	::testing::Values("desert.lf_mta", "mansion-blockout.lf_mta"),
	[](auto const& info) {
		std::string name = info.param;
		name = name.substr(0, name.find('.'));
		for(auto & c : name) if(!isalnum((unsigned char)c)) c = '_';
		return name;
	});
