#ifndef ENUMS_HPP
#define ENUMS_HPP

enum
{
	TILE_BYTES = 550000,
};

enum class Tool : int
{
	None = -1,
	Create,
	Duplicate,
	Select,
	Translate,
	Rotate,
	Scale,
	Slice,
	Extrude,
	Order,
	Face,

	SliceGravity,
	PropagateGravity,

	Finish
};

enum class State : int
{
	None = -1,
	MouseDownOnSelected,
	MouseDownOnUnselected,
	MouseDownOnNothing,

	CreateSet,
	BoxSelectSet,
	TranslateSet,
	RotateSet,
	ScaleSet,
	SliceSet,
	ExtrudeSet,

	CreateBegin,
	BoxSelectBegin,
	TranslateBegin,
	RotateBegin,
	ScaleBegin,
	SliceBegin,
	ExtrudeBegin,
	WeldBegin,

	SliceSetGravity,

	RotateGravity,
	DirectGravity,
	ScaleGravity,
	SliceGravity,
	PropagateGravity,

	Reorder,
};


enum class SelectionType : unsigned char
{
	VERTEX,
	EDGE,
	FACE,
};

enum class Tools
{
	CLICK_SELECT,
	BOX_SELECT,

	DELETE_FACES,

	TRANSLATE,
	ROTATE,
	SCALE,

	EXTRUDE,
	SLICE,
	KNIFE,
};

enum class Bitwise
{
	SET = 1,
	AND = 2,
	OR  = 4,
	XOR = 8,
	NOT = 16,
};

enum class BackgroundLayer : char
{
	None = -1,
	BaseColor,
	MetallicRoughness,
	Normals,
	AmbientOcclusion,
	Depth,
	Total
};

extern "C"
{

struct BC4_Block
{
	unsigned char c[2];
	unsigned char i[6];
};

struct DXT1_Block
{
	unsigned short c[2];
	unsigned char  i[4];
};

struct DXT5_Block
{
	BC4_Block  alpha;
	DXT1_Block color;
};

struct BC5_Block
{
	BC4_Block  R;
	BC4_Block  G;
};

struct DepthBlock
{
	unsigned short depth[16];
};

}


enum {
	BaseColorBlockSize = sizeof(DXT1_Block),
	RoughBlockSize		= sizeof(BC5_Block),
	NormalBlockSize		= sizeof(BC5_Block),
	OcclusionBlockSize = sizeof(BC4_Block),
	DepthBlockSize     = 2*16,
};

enum
{
	BaseColorOffset   = 0,
	RoughOffset       = BaseColorOffset + BaseColorBlockSize,
	NormalOffset      = RoughOffset     + RoughBlockSize,
	OcclusionOffset   = NormalOffset    + NormalBlockSize,
	DepthOffset       = OcclusionOffset + OcclusionBlockSize,
	DeinterlacedBytes = DepthOffset     + DepthBlockSize,
};

namespace Background
{
	enum
	{
		FileMipLayers = (int)BackgroundLayer::Total,

		Width = 256,
		Height = 256,

		Area = Width * Height,
		Blocks = Width * Height / 16,

		BYTES_PER_BLOCK = DeinterlacedBytes,
	};
}

#endif // ENUMS_HPP
