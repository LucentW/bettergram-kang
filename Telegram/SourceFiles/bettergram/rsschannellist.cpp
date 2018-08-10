#include "rsschannellist.h"
#include "rsschannel.h"
#include <logs.h>

namespace Bettergram {

RssChannelList::RssChannelList(QObject *parent) :
	QObject(parent)
{
}

RssChannelList::const_iterator RssChannelList::begin() const
{
	return _list.begin();
}

RssChannelList::const_iterator RssChannelList::end() const
{
	return _list.end();
}

RssChannel *RssChannelList::at(int index) const
{
	if (index < 0 || index >= _list.size()) {
		LOG(("Index is out of bounds"));
		return nullptr;
	}

	return _list.at(index);
}

int RssChannelList::count() const
{
	return _list.count();
}

} // namespace Bettergrams
