#pragma once

#include <QObject>

namespace Bettergram {

class ResourceGroup;
class RssItem;

/**
 * @brief The ResourceGroupList class contains list of ResourceGroup instances.
 */
class ResourceGroupList : public QObject {
	Q_OBJECT

public:
	typedef QList<QSharedPointer<ResourceGroup>>::const_iterator const_iterator;

	explicit ResourceGroupList(QObject *parent = nullptr);

	int freq() const;
	void setFreq(int freq);

	QDateTime lastUpdate() const;
	QString lastUpdateString() const;

	const_iterator begin() const;
	const_iterator end() const;

	/// Can throw std::out_of_range() exception
	const QSharedPointer<ResourceGroup> &at(int index) const;

	bool isEmpty() const;
	int count() const;

	void parseFile(const QString &filePath);
	void parse(const QByteArray &byteArray);
	void parse(const QJsonObject &json);

public slots:

signals:
	void freqChanged();
	void lastUpdateChanged();
	
	void updated();

protected:

private:
	/// Default frequency of updates in seconds
	static const int _defaultFreq;

	QList<QSharedPointer<ResourceGroup>> _list;

	/// Frequency of updates in seconds
	int _freq;

	QDateTime _lastUpdate;
	QString _lastUpdateString;

	void setLastUpdate(const QDateTime &lastUpdate);
};

} // namespace Bettergram
