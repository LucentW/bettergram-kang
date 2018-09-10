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
	/// We use these values to break searching images if we have already found big enough image
	static const int DEFAULT_WIDTH;
	static const int DEFAULT_HEIGHT;

	RemoteTempData _siteContent;
	RemoteImage _image;

	int parseIntAttribute(const QStringRef &source,
						  const QString &startAttribute,
						  const QString &endAttribute);

	QStringRef parseStringAttribute(const QStringRef &source,
									const QString &startAttribute,
									const QString &endAttribute);

	QStringRef getLargestImageInImageTags(const QString &source,
										  int &maxWidth,
										  int &maxHeight,
										  int &position);

	QString getLargestImageInFileNames(const QString &source,
									   int &maxWidth,
									   int &maxHeight,
									   int &position);

private slots:
	void onSiteContentDownloaded(QByteArray data);
};

} // namespace Bettergram
