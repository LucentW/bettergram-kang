#include "rsschannellist.h"
#include "rsschannel.h"
#include <lang/lang_keys.h>
#include <logs.h>

namespace Bettergram {

const int RssChannelList::_defaultFreq = 60;
const QString RssChannelList::_defaultLastUpdateString = "...";

QString RssChannelList::countLastUpdateString(const QDateTime &dateTime)
{
	if (dateTime.isNull()) {
		return _defaultLastUpdateString;
	}

	qint64 daysBefore = QDateTime::currentDateTime().daysTo(dateTime);
	const QString timeString = dateTime.toString("hh:mm:ss");

	if (daysBefore == 0) {
		return lng_player_message_today(lt_time, timeString);
	} else if (daysBefore == -1) {
		return lng_player_message_yesterday(lt_time, timeString);
	} else {
		return lng_player_message_date(lt_date,
									   langDayOfMonthFull(dateTime.date()),
									   lt_time,
									   timeString);
	}
}

RssChannelList::RssChannelList(QObject *parent) :
	QObject(parent),
	_freq(_defaultFreq),
	_lastUpdateString(_defaultLastUpdateString)
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

QDateTime RssChannelList::lastUpdate() const
{
	return _lastUpdate;
}

QString RssChannelList::lastUpdateString() const
{
	return _lastUpdateString;
}

void RssChannelList::setLastUpdate(const QDateTime &lastUpdate)
{
	if (_lastUpdate != lastUpdate) {
		_lastUpdate = lastUpdate;

		_lastUpdateString = countLastUpdateString(_lastUpdate);
		emit lastUpdateChanged();
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

bool RssChannelList::isEmpty() const
{
	return _list.isEmpty();
}

int RssChannelList::count() const
{
	return _list.count();
}

int RssChannelList::countAllItems() const
{
	int result = 0;

	for (const QSharedPointer<RssChannel> &channel : _list) {
		result += channel->count();
	}

	return result;
}

int RssChannelList::countAllUnreadItems() const
{
	int result = 0;

	for (const QSharedPointer<RssChannel> &channel : _list) {
		result += channel->countUnread();
	}

	return result;
}

void RssChannelList::add(const QUrl &channelLink)
{
	if (channelLink.isEmpty()) {
		LOG(("Unable to add empty RSS channel"));
		return;
	}

	QSharedPointer<RssChannel> channel(new RssChannel(channelLink, nullptr));
	add(channel);
}

void RssChannelList::add(QSharedPointer<RssChannel> &channel)
{
	connect(channel.data(), &RssChannel::iconChanged, this, &RssChannelList::iconChanged);
	connect(channel.data(), &RssChannel::isReadChanged, this, &RssChannelList::onIsReadChanged);

	_list.push_back(channel);
}

void RssChannelList::markAsRead()
{
	for (QSharedPointer<RssChannel> &channel : _list) {
		disconnect(channel.data(), &RssChannel::isReadChanged, this, &RssChannelList::onIsReadChanged);

		channel->markAsRead();

		connect(channel.data(), &RssChannel::isReadChanged, this, &RssChannelList::onIsReadChanged);
	}

	onIsReadChanged();
}

QList<QSharedPointer<RssItem>> RssChannelList::getAllItems() const
{
	QList<QSharedPointer<RssItem>> result;
	result.reserve(countAllItems());

	for (const QSharedPointer<RssChannel> &channel : _list) {
		result.append(channel->getAllItems());
	}

	return result;
}

QList<QSharedPointer<RssItem>> RssChannelList::getAllUnreadItems() const
{
	QList<QSharedPointer<RssItem>> result;
	result.reserve(countAllUnreadItems());

	for (const QSharedPointer<RssChannel> &channel : _list) {
		result.append(channel->getAllUnreadItems());
	}

	return result;
}

void RssChannelList::parse()
{
	for (const QSharedPointer<RssChannel> &channel : _list) {
		if (channel->isFetching()) {
			return;
		}
	}

	bool isChanged = false;
	bool isAtLeastOneUpdated = false;

	for (const QSharedPointer<RssChannel> &channel : _list) {
		if (!channel->isFailed()) {
			isAtLeastOneUpdated = true;

			if (channel->parse()) {
				isChanged = true;
			}
		}
	}

	if (isChanged) {
		save();
		emit updated();
	}

	if (isAtLeastOneUpdated) {
		setLastUpdate(QDateTime::currentDateTime());
	}
}

void RssChannelList::save()
{
	//TODO: bettergram: save rss channel list to local file, not in the Windows Registry

	QSettings settings;

	settings.beginGroup("news");

	settings.setValue("lastUpdate", _lastUpdate);
	settings.setValue("frequency", _freq);
	settings.beginWriteArray("channels", _list.size());

	for (int i = 0; i < _list.size(); i++) {
		const QSharedPointer<RssChannel> &channel = _list.at(i);

		settings.setArrayIndex(i);
		channel->save(settings);
	}

	settings.endArray();
	settings.endGroup();
}

void RssChannelList::load()
{
	QSettings settings;

	settings.beginGroup("news");

	setLastUpdate(settings.value("lastUpdate").toDateTime());
	setFreq(settings.value("frequency", _defaultFreq).toInt());

	int size = settings.beginReadArray("channels");

	for (int i = 0; i < size; i++) {
		QSharedPointer<RssChannel> channel(new RssChannel(this));

		settings.setArrayIndex(i);
		channel->load(settings);

		add(channel);
	}

	settings.endArray();
	settings.endGroup();
}

void RssChannelList::onIsReadChanged()
{
	save();
}

} // namespace Bettergrams
