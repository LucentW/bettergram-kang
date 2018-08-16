#pragma once

namespace ChatHelpers {

/**
 * @brief ListRow represents simple list row data: top, bottom, height and user data.
 * In normal project we would use QListView, but here we do not use such classes
 */
template<typename TUserData>
class ListRow
{
public:
	explicit ListRow(const TUserData &userData, int top, int height) :
	_userData(userData), _top(top), _height(height)
	{
	}

	const TUserData &userData() const
	{
		return _userData;
	}

	int top() const
	{
		return _top;
	}

	void setTop(int top)
	{
		_top = top;
	}

	int bottom() const
	{
		return _top + _height;
	}

	void setBottom(int bottom)
	{
		setHeight(bottom - _top);
	}

	int height() const
	{
		return _height;
	}

	void setHeight(int height)
	{
		if (_height < 0) {
			LOG(("Unable to set negative height (%1) to list row").arg(height));
			_height = 0;
		} else {
			_height = height;
		}
	}

	bool contains(int y)
	{
		return y >= _top && y <= (_top + _height);
	}

private:
	TUserData _userData;

	int _top = 0;
	int _height = 0;
};

} // namespace ChatHelpers
