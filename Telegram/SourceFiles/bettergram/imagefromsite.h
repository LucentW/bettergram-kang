#pragma once

#include "remotetempdata.h"
#include "remoteimage.h"

namespace Bettergram {

/**
 * @brief The ImageFromSite class is used to download the biggest image from a site.
 * We use this class to fetch RSS thumbnail if all other ways are broken.
 */
class ImageFromSite : public QObject {
	Q_OBJECT

public:
	explicit ImageFromSite(QObject *parent = nullptr);
	explicit ImageFromSite(const QUrl &link, QObject *parent = nullptr);
	explicit ImageFromSite(int scaledWidth, int scaledHeight, QObject *parent = nullptr);

	const QUrl &link() const;
	void setLink(const QUrl &link);

	int scaledWidth() const;
	void setScaledWidth(int scaledWidth);

	int scaledHeight() const;
	void setScaledHeight(int scaledHeight);

	void setScaledSize(int scaledWidth, int scaledHeight);

	const QUrl &imageLink() const;
	void setImageLink(const QUrl &imageLink);

	const QPixmap &image() const;

	bool isNull() const;

public slots:

signals:
	void imageChanged();

protected:

private:
	RemoteTempData _siteContent;
	RemoteImage _image;

	double parseDoubleAttribute(const QStringRef &source,
								const QString &startAttribute,
								const QString &endAttribute);

	QStringRef parseStringAttribute(const QStringRef &source,
									const QString &startAttribute,
									const QString &endAttribute);

private slots:
	void onSiteContentDownloaded(QByteArray data);
};

} // namespace Bettergram
