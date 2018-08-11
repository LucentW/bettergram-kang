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

	const_iterator begin() const;
	const_iterator end() const;

	/// Can throw std::out_of_range() exception
	const QSharedPointer<RssChannel> &at(int index) const;
	int count() const;

	void parse();

public slots:

signals:

protected:

private:
	QList<QSharedPointer<RssChannel>> _list;
};

} // namespace Bettergram
