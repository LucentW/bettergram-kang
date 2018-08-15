#include "rsschannellist.h"
#include "rsschannel.h"
#include <logs.h>

namespace Bettergram {

const int RssChannelList::_defaultFreq = 60;

RssChannelList::RssChannelList(QObject *parent) :
	QObject(parent),
	_freq(_defaultFreq)
{
}

int RssChannelList::freq() const
{
	return _freq;
}

void RssChannelList::setFreq(int freq)
{
	if (freq == 0) {
		freq = _defaultFreq;
	}

	if (_freq != freq) {
		_freq = freq;
		emit freqChanged();
	}
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

void RssChannelList::add(const QUrl &channelLink)
{
	if (channelLink.isEmpty()) {
		LOG(("Unable to add empty RSS channel"));
		return;
	}

	QSharedPointer<RssChannel> channel(new RssChannel(channelLink, nullptr));
	_list.push_back(channel);
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
