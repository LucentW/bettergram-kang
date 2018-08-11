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

const QSharedPointer<RssChannel> &RssChannelList::at(int index) const
{
	if (index < 0 || index >= _list.size()) {
		LOG(("Index is out of bounds"));
		throw std::out_of_range("rss channel index is out of range");
	}

	return _list.at(index);
}

int RssChannelList::count() const
{
	return _list.count();
}

void RssChannelList::parse()
{
	for (const QSharedPointer<RssChannel> &channel : _list) {
		if (channel->isFetching()) {
			return;
		}
	}

	for (const QSharedPointer<RssChannel> &channel : _list) {
		channel->parse();
	}
}

} // namespace Bettergrams
