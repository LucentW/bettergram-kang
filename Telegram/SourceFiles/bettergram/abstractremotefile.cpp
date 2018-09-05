#include "abstractremotefile.h"

#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace Bettergram {

AbstractRemoteFile::AbstractRemoteFile(QObject *parent) :
	QObject(parent)
{
}

AbstractRemoteFile::AbstractRemoteFile(const QUrl &link, QObject *parent) :
	QObject(parent),
	_link(link)
{
	download();
}

const QUrl &AbstractRemoteFile::link() const
{
	return _link;
}

void AbstractRemoteFile::setLink(const QUrl &link)
{
	if (_link != link) {
		_link = link;

		download();
		emit linkChanged();
	}
}

void AbstractRemoteFile::download()
{
	if (!_link.isValid()) {
		resetData();
		return;
	}

	QNetworkAccessManager *networkManager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(_link);

	QNetworkReply *reply = networkManager->get(request);

	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		if(reply->error() == QNetworkReply::NoError) {
			dataDownloaded(reply->readAll());
		} else {
			LOG(("Can not download file at %1. %2 (%3)")
				.arg(_link.toString())
				.arg(reply->errorString())
				.arg(reply->error()));

			downloadLater();
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

void AbstractRemoteFile::downloadLater()
{
	qsrand(static_cast<uint>(QDateTime::currentMSecsSinceEpoch() / (5 * 1000 * 1000)));

	//TODO: bettergram: increase the timeout after each call of this method
	QTimer::singleShot(2000 + (qrand() % 3000), this, [this](){ download(); });
}

} // namespace Bettergrams
