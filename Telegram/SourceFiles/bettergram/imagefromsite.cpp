#include "imagefromsite.h"

#include <QUrl>
#include <QRegExp>

namespace Bettergram {

const int ImageFromSite::DEFAULT_WIDTH = 600;
const int ImageFromSite::DEFAULT_HEIGHT = 380;

ImageFromSite::ImageFromSite(QObject *parent) :
	QObject(parent)
{
	connect(&_siteContent, &RemoteTempData::downloaded,
			this, &ImageFromSite::onSiteContentDownloaded);

	connect(&_image, &RemoteImage::imageChanged,
			this, &ImageFromSite::imageChanged);
}

ImageFromSite::ImageFromSite(const QUrl &link, QObject *parent) :
	QObject(parent),
	_siteContent(link)
{
	connect(&_siteContent, &RemoteTempData::downloaded,
			this, &ImageFromSite::onSiteContentDownloaded);

	connect(&_image, &RemoteImage::imageChanged,
			this, &ImageFromSite::imageChanged);
}

ImageFromSite::ImageFromSite(int scaledWidth, int scaledHeight, QObject *parent) :
	QObject(parent),
	_siteContent(),
	_image(scaledWidth, scaledHeight)
{
	connect(&_siteContent, &RemoteTempData::downloaded,
			this, &ImageFromSite::onSiteContentDownloaded);

	connect(&_image, &RemoteImage::imageChanged,
			this, &ImageFromSite::imageChanged);
}

const QUrl &ImageFromSite::link() const
{
	return _siteContent.link();
}

void Bettergram::ImageFromSite::setLink(const QUrl &link)
{
	_siteContent.setLink(link);
}

int Bettergram::ImageFromSite::scaledWidth() const
{
	return _image.scaledWidth();
}

void Bettergram::ImageFromSite::setScaledWidth(int scaledWidth)
{
	_image.setScaledWidth(scaledWidth);
}

int Bettergram::ImageFromSite::scaledHeight() const
{
	return _image.scaledHeight();
}

void Bettergram::ImageFromSite::setScaledHeight(int scaledHeight)
{
	_image.setScaledHeight(scaledHeight);
}

void Bettergram::ImageFromSite::setScaledSize(int scaledWidth, int scaledHeight)
{
	_image.setScaledSize(scaledWidth, scaledHeight);
}

const QUrl &ImageFromSite::imageLink() const
{
	return _image.link();
}

void ImageFromSite::setImageLink(const QUrl &imageLink)
{
	_image.setLink(imageLink);
}

const QPixmap &ImageFromSite::image() const
{
	return _image.image();
}

bool ImageFromSite::isNull() const
{
	return _image.isNull();
}

double ImageFromSite::parseDoubleAttribute(const QStringRef &source,
										   const QString &startAttribute,
										   const QString &endAttribute)
{
	int startAttributeIndex = source.indexOf(startAttribute, 0, Qt::CaseInsensitive);

	if (startAttributeIndex != -1) {
		startAttributeIndex += startAttribute.size();

		int endAttributeIndex = source.indexOf(endAttribute,
											   startAttributeIndex,
											   Qt::CaseInsensitive);

		if (endAttributeIndex != -1) {
			return source.mid(startAttributeIndex, endAttributeIndex - startAttributeIndex)
					.toDouble();
		}
	}

	return 0.0;
}

QStringRef ImageFromSite::parseStringAttribute(const QStringRef &source,
											   const QString &startAttribute,
											   const QString &endAttribute)
{
	int startAttributeIndex = source.indexOf(startAttribute, 0, Qt::CaseInsensitive);

	if (startAttributeIndex != -1) {
		startAttributeIndex += startAttribute.size();

		int endAttributeIndex = source.indexOf(endAttribute,
											   startAttributeIndex,
											   Qt::CaseInsensitive);

		if (endAttributeIndex != -1) {
			return source.mid(startAttributeIndex, endAttributeIndex - startAttributeIndex);
		}
	}

	return QStringRef();
}

QStringRef ImageFromSite::getLargestImageInImageTags(const QString &source,
													 double &maxWidth,
													 double &maxHeight)
{
	QStringRef imageLink;

	const QString startImageTag = QStringLiteral("<img");
	const QString endImageTag = QStringLiteral(">");

	const QString startSourceAttribute = QStringLiteral("src=\"");
	const QString endSourceAttribute = QStringLiteral("\"");

	const QString startWidthAttribute = QStringLiteral("width=\"");
	const QString endWidthAttribute = QStringLiteral("\"");

	const QString startHeightAttribute = QStringLiteral("height=\"");
	const QString endHeightAttribute = QStringLiteral("\"");

	int startIndex = 0;

	while (true) {
		int startImageTagIndex = source.indexOf(startImageTag, startIndex, Qt::CaseInsensitive);

		if (startImageTagIndex == -1) {
			break;
		}

		startImageTagIndex += startImageTag.size();
		startIndex = startImageTagIndex;

		int endImageTagIndex = source.indexOf(endImageTag, startImageTagIndex, Qt::CaseInsensitive);

		if (endImageTagIndex == -1) {
			endImageTagIndex = source.size();
		}

		startIndex = endImageTagIndex;

		QStringRef imageTag = source.midRef(startImageTagIndex,
											endImageTagIndex - startImageTagIndex);

		QStringRef currentImage = parseStringAttribute(imageTag,
													   startSourceAttribute,
													   endSourceAttribute);

		if (currentImage.isEmpty()) {
			continue;
		}

		double currentWidth = parseDoubleAttribute(imageTag,
												   startWidthAttribute,
												   endWidthAttribute);

		double currentHeight = parseDoubleAttribute(imageTag,
													startHeightAttribute,
													endHeightAttribute);

		if (currentHeight > maxHeight) {
			maxHeight = currentHeight;
			maxWidth = currentWidth;

			imageLink = currentImage;
		} else if (currentHeight == maxHeight && currentWidth > maxWidth) {
			maxHeight = currentHeight;
			maxWidth = currentWidth;

			imageLink = currentImage;
		} else if (maxWidth == 0.0 && maxHeight == 0.0) {
			imageLink = currentImage;
		}

		// We do not need to continue parsing the site content
		// if we have already got a big enough image
		if (currentWidth >= DEFAULT_WIDTH && currentHeight >= DEFAULT_HEIGHT) {
			break;
		}
	}

	return imageLink;
}

QString ImageFromSite::getLargestImageInFileNames(const QString &source,
												  double &maxWidth,
												  double &maxHeight)
{
	QRegExp regExp("(http://|https://)([\\w]|[0-9]|_|-|/|\\.)+(-|_)(\\d+)(x)(\\d+)(.jpg|.png|.jpeg)");
	int position = 0;

	int tempMaxWidth = 0;
	int tempMaxHeight = 0;

	QString imageLink;

	while((position = regExp.indexIn(source, position)) != -1) {
		int currentWidth = regExp.cap(4).toInt();
		int currentHeight = regExp.cap(6).toInt();

		if (currentHeight > tempMaxHeight) {
			tempMaxHeight = currentHeight;
			tempMaxWidth = currentWidth;

			imageLink = regExp.cap(0);
		} else if (currentHeight == tempMaxHeight && currentWidth > tempMaxWidth) {
			tempMaxHeight = currentHeight;
			tempMaxWidth = currentWidth;

			imageLink = regExp.cap(0);
		} else if (tempMaxWidth == 0.0 && tempMaxHeight == 0.0) {
			imageLink = regExp.cap(0);
		}

		position += regExp.matchedLength();

		// We do not need to continue parsing the site content
		// if we have already got a big enough image
		if (currentWidth >= DEFAULT_WIDTH && currentHeight >= DEFAULT_HEIGHT) {
			break;
		}
	}

	maxWidth = tempMaxWidth;
	maxHeight = tempMaxHeight;

	return imageLink;
}

void Bettergram::ImageFromSite::onSiteContentDownloaded(QByteArray data)
{
	// Here we should find all images with sizes and find the largest one

	const QString source = QString::fromUtf8(data);

	double maxWidthInImageTags = 0.0;
	double maxHeightInImageTags = 0.0;

	QStringRef imageLinkInImageTags = getLargestImageInImageTags(source,
																 maxWidthInImageTags,
																 maxHeightInImageTags);

	double maxWidthInFileNames = 0.0;
	double maxHeightInFileNames = 0.0;

	QString imageLinkInFileName = getLargestImageInFileNames(source,
															 maxWidthInFileNames,
															 maxHeightInFileNames);

	QUrl imageUrl;

	if (maxHeightInFileNames > maxHeightInImageTags) {
		imageUrl = imageLinkInFileName;
	} else if (maxHeightInFileNames == maxHeightInImageTags
			   && maxWidthInFileNames > maxWidthInImageTags) {
		imageUrl = imageLinkInFileName;
	} else {
		imageUrl = imageLinkInImageTags.toString();
	}

	if (imageUrl.isValid()) {
		_image.setLink(imageUrl);
	}
}

} // namespace Bettergrams
