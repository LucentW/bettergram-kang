#include "resourceitem.h"

#include <styles/style_chat_helpers.h>

#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace Bettergram {

ResourceItem::ResourceItem(QObject *parent) :
	QObject(parent),
	_icon(st::resourcesPanImageSize, st::resourcesPanImageSize)
{
	connect(&_icon, &RemoteImage::imageChanged, this, &ResourceItem::iconChanged);
}

ResourceItem::ResourceItem(const QString &title,
						   const QString &description,
						   const QUrl &link,
						   const QUrl &iconLink,
						   QObject *parent) :
	QObject(parent),
	_title(title),
	_description(description),
	_link(link),
	_icon(iconLink, st::resourcesPanImageSize, st::resourcesPanImageSize)
{
	connect(&_icon, &RemoteImage::imageChanged, this, &ResourceItem::iconChanged);
}

const QString &ResourceItem::title() const
{
	return _title;
}

const QString &ResourceItem::description() const
{
	return _description;
}

const QUrl &ResourceItem::link() const
{
	return _link;
}

const QUrl &ResourceItem::iconLink() const
{
	return _icon.link();
}

const QPixmap &ResourceItem::icon() const
{
	return _icon.image();
}

void ResourceItem::parse(const QJsonObject &json)
{
	if (json.isEmpty()) {
		return;
	}

	_title = json.value("title").toString();
	_description = json.value("description").toString();

	_link = json.value("url").toString();
	_icon.setLink(json.value("iconUrl").toString());
}

} // namespace Bettergrams
