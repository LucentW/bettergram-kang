#pragma once

#include <QObject>

namespace Bettergram {

class RssChannel;

/**
 * @brief The RssChannelList class contains list of RssChannel instances.
 */
class RssChannelList : public QObject {
	Q_OBJECT

public:
	typedef QList<QSharedPointer<RssChannel>>::const_iterator const_iterator;

	explicit RssChannelList(QObject *parent = nullptr);

	int freq() const;
	void setFreq(int freq);

	QTime lastUpdate() const;
	QString lastUpdateString() const;

	const_iterator begin() const;
	const_iterator end() const;

	/// Can throw std::out_of_range() exception
	const QSharedPointer<RssChannel> &at(int index) const;

	int count() const;
	int countAllItems() const;
	int countAllUnreadItems() const;

	void add(const QUrl &channelLink);

	void parse();

public slots:

signals:
	void freqChanged();
	void lastUpdateChanged();
	
	void updated();

protected:

private:
	/// Default frequency of updates in seconds
	static const int _defaultFreq;

	QList<QSharedPointer<RssChannel>> _list;

	/// Frequency of updates in seconds
	int _freq;

	QTime _lastUpdate;
};

} // namespace Bettergram
