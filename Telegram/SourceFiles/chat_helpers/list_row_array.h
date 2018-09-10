#pragma once

#include "list_row.h"

namespace ChatHelpers {

/**
 * @brief ListRowArray represents simple array of ListRow instances.
 * In normal project we would use QListView, but here we do not use such classes
 */
template<typename TUserData>
class ListRowArray
{
public:
	typedef ListRow<TUserData> Row;
	typedef typename QList<Row>::const_iterator const_iterator;

	explicit ListRowArray()
	{
	}

	int top() const
	{
		return _list.empty() ? _top : _list.first().top();
	}

	void setTop(int top)
	{
		if (_top != top) {
			_top = top;
			updateGeometry();
		}
	}

	int spacing() const
	{
		return _spacing;
	}

	void setSpacing(int spacing)
	{
		if (_spacing != spacing) {
			_spacing = spacing;
			updateGeometry();
		}
	}

	int bottom() const
	{
		return _list.empty() ? _top : _list.last().bottom();
	}

	int height() const
	{
		return bottom() - top();
	}

	bool contains(int y) const
	{
		if (y >= top() && y <= bottom()) {
			//TODO: bettergram: we can optimize this by using dichotomy for example
			for (const Row &row : _list) {
				if (row.contains(y)) {
					return true;
				}
			}
		}

		return false;
	}

	int findRowIndex(int y) const
	{
		if (y >= top() && y <= bottom()) {
			//TODO: bettergram: we can optimize this by using dichotomy for example
			for (int i = 0; i < _list.count(); i++) {
				if (_list.at(i).contains(y)) {
					return i;
				}
			}
		}

		return -1;
	}

	const_iterator begin() const
	{
		return _list.cbegin();
	}

	const_iterator end() const
	{
		return _list.cend();
	}

	int count() const
	{
		return _list.count();
	}

	const Row &at(int index) const
	{
		if (index < 0 || index >= _list.count()) {
			throw std::out_of_range("Unable to get ListRow at wrong index");
		}

		return _list.at(index);
	}

	void clear()
	{
		_list.clear();
	}

	void add(const TUserData &userData, int height)
	{
		_list.push_back(Row(userData, bottom() + _spacing, height));
	}

private:
	int _top = 0;
	int _spacing = 0;
	
	QList<Row> _list;

	void updateGeometry()
	{
		int rowTop = _top;

		for (Row &row : _list) {
			row.setTop(rowTop);
			rowTop += row.height() + _spacing;
		}
	}
};

} // namespace ChatHelpers
