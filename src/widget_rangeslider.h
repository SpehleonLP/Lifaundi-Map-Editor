#ifndef Q_RANGE_SLIDER_HPP
#define Q_RANGE_SLIDER_HPP
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class RangeSlider : public QWidget
{
	Q_OBJECT
	Q_ENUMS(RangeSliderTypes)

public:
	enum Option {
		NoHandle = 0x0,
		LeftHandle = 0x1,
		RightHandle = 0x2,
		DoubleHandles = LeftHandle | RightHandle
	};
	Q_DECLARE_FLAGS(Options, Option)

	RangeSlider( QWidget* aParent = Q_NULLPTR);
	RangeSlider( Qt::Orientation ori, Options t = DoubleHandles, QWidget* aParent = Q_NULLPTR);

	QSize minimumSizeHint() const override;

	int GetMinimun() const;
	void SetMinimum(int aMinimum);

	int GetMaximun() const;
	void SetMaximum(int aMaximum);

	int GetLowerValue() const;
	void SetLowerValue(int aLowerValue);

	int GetUpperValue() const;
	void SetUpperValue(int aUpperValue);

	void setValue(int aLowerValue, int aUpperValue);
	void SetRange(int aMinimum, int aMaximum);

	void setTickInterval(int) {};
	void setSingleStep(int) {};
	void setPageStep(int) {};
	void setTickPosition(int) {};
	void setOrientation(Qt::Orientation o) { orientation = o; };

protected:
	void paintEvent(QPaintEvent* aEvent) override;
	void mousePressEvent(QMouseEvent* aEvent) override;
	void mouseMoveEvent(QMouseEvent* aEvent) override;
	void mouseReleaseEvent(QMouseEvent* aEvent) override;
	void changeEvent(QEvent* aEvent) override;

	QRectF firstHandleRect() const;
	QRectF secondHandleRect() const;
	QRectF handleRect(int aValue) const;

signals:
	void lowerValueChanged(int aLowerValue);
	void upperValueChanged(int aUpperValue);
	void rangeChanged(int aMin, int aMax);
	void valueChanged();

public slots:
	void setLowerValue(int aLowerValue);
	void setUpperValue(int aUpperValue);
	void setMinimum(int aMinimum);
	void setMaximum(int aMaximum);

private:
	Q_DISABLE_COPY(RangeSlider)
	float currentPercentage();
	int validLength() const;

	int mMinimum{};
	int mMaximum{100};
	int mLowerValue{0};
	int mUpperValue{100};
	bool mFirstHandlePressed{false};
	bool mSecondHandlePressed{false};
	int mInterval{mMaximum - mMinimum};
	int mDelta{0};
	QColor mBackgroudColorEnabled{QColor(0x1E, 0x90, 0xFF)};
	QColor mBackgroudColorDisabled{Qt::darkGray};
	QColor mBackgroudColor{mBackgroudColorEnabled};
	Qt::Orientation orientation{Qt::Horizontal};
	Options type{DoubleHandles};
};

Q_DECLARE_OPERATORS_FOR_FLAGS(RangeSlider::Options)

#endif
