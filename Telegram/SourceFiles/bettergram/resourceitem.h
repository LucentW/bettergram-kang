#pragma once

#include "remoteimage.h"

#include <QObject>

namespace Bettergram {

/**
 * @brief The ResourceItem class contains information from a resource item for the ResourcesWidget.
 */
class ResourceItem : public QObject {
	Q_OBJECT

public:
	explicit ResourceItem(QObject *parent = nullptr);

	explicit ResourceItem(const QString &title,
						  const QString &description,
						  const QUrl &link,
						  const QUrl &iconLink,
						  QObject *parent = nullptr);

	const QString &title() const;
	const QString &description() const;

	const QUrl &link() const;
	const QUrl &iconLink() const;
	const QPixmap &icon() const;

	void parse(const QJsonObject &json);

public slots:

signals:
	void iconChanged();

protected:

private:
	QString _title;
	QString _description;

	QUrl _link;

	RemoteImage _icon;
};

} // namespace Bettergram
