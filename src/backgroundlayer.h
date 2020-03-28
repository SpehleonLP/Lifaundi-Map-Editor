#ifndef BACKGROUNDLAYER_H
#define BACKGROUNDLAYER_H


class BackgroundLayer
{
public:
	BackgroundLayer();

	bool IsProcessingTask() { return false; }
	//unsafe.
	BackgroundLayer * createdFrom;
};

#endif // BACKGROUNDLAYER_H
