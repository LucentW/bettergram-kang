#pragma once

#include <QObject>

namespace Bettergram {

class ResourceItem;

/**
 * @brief The ResourceGroup class contains list of ResourceItems.
 */
class ResourceGroup : public QObject {
	Q_OBJECT

public:
	typedef QList<QSharedPointer<ResourceItem>>::const_iterator const_iterator;
	typedef QList<QSharedPointer<ResourceItem>>::iterator iterator;

	explicit ResourceGroup(QObject *parent = nullptr);

	const QString &title() const;

	const_iterator begin() const;
	const_iterator end() const;

	const QSharedPointer<ResourceItem> &at(int index) const;

	int count() const;
	bool isEmpty() const;

	const QList<QSharedPointer<ResourceItem>> &items() const;

	void parse(const QJsonObject &json);

public slots:

signals:
	void iconChanged();

protected:

private:
	QString _title;
	QList<QSharedPointer<ResourceItem>> _list;
};

} // namespace Bettergram
