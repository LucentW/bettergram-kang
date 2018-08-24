#include "resourceitem.h"

#include <styles/style_chat_helpers.h>

#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace Bettergram {

ResourceItem::ResourceItem(QObject *parent) :
	QObject(parent)
{
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
	_iconLink(iconLink)
{
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
	return _iconLink;
}

const QPixmap &ResourceItem::icon() const
{
	return _icon;
}

void ResourceItem::setIcon(const QByteArray &byteArray)
{
	if (byteArray.isEmpty()) {
		LOG(("Can not get icon for resource item %1. Response is empty.")
			.arg(_link.toString()));
		return;
	}

	QPixmap icon;

	if (!icon.loadFromData(byteArray)) {
		LOG(("Can not get icon for resource item %1. Can not convert response to image.")
			.arg(_link.toString()));
		return;
	}

	setIcon(icon);
}

void ResourceItem::setIcon(const QPixmap &icon)
{
	_icon = icon;

	if (!_icon.isNull()
			&& (_icon.width() != st::resourcesPanImageSize || _icon.height() != st::resourcesPanImageSize)) {
		_icon = _icon.scaled(st::resourcesPanImageSize,
							 st::resourcesPanImageSize,
							 Qt::KeepAspectRatio,
							 Qt::SmoothTransformation);
	}

	emit iconChanged();
}

void ResourceItem::parse(const QJsonObject &json)
{
	if (json.isEmpty()) {
		return;
	}

	_title = json.value("title").toString();
	_description = json.value("description").toString();

	_link = json.value("link").toString();
	_iconLink = json.value("iconLink").toString();

	downloadIcon();
}

void ResourceItem::downloadIcon()
{
	if (!_iconLink.isValid()) {
		return;
	}

	QNetworkAccessManager *networkManager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(_iconLink);

	QNetworkReply *reply = networkManager->get(request);

	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		if(reply->error() == QNetworkReply::NoError) {
			setIcon(reply->readAll());
		} else {
			LOG(("Can not get icon for resource item %1 (%2). %3 (%4)")
				.arg(_link.toString())
				.arg(_iconLink.toString())
				.arg(reply->errorString())
				.arg(reply->error()));

			downloadIconLater();
		}
	});

	connect(reply, &QNetworkReply::finished, [networkManager, reply]() {
		reply->deleteLater();
		networkManager->deleteLater();
	});

	connect(reply, &QNetworkReply::sslErrors, this, [](QList<QSslError> errors) {
		for(const QSslError &error : errors) {
			LOG(("%1").arg(error.errorString()));
		}
	});
}

void ResourceItem::downloadIconLater()
{
	QTimer::singleShot(5000, this, [this](){ downloadIcon(); });
}

} // namespace Bettergrams
