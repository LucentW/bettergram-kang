#include "rssitem.h"
#include "rsschannel.h"
#include "imagefromsite.h"
#include "bettergramservice.h"

#include <QXmlStreamReader>

namespace Bettergram {

const qint64 RssItem::_maxLastHoursInMs = 24 * 60 * 60 * 1000;

RssItem::RssItem(RssChannel *channel) :
	QObject(channel),
	_channel(channel),
	_image(_channel->iconWidth(), _channel->iconHeight())
{
	if (!_channel) {
		throw std::invalid_argument("RSS Channel is null");
	}

	connect(&_image, &RemoteImage::imageChanged, this, &RssItem::imageChanged);
}

RssItem::RssItem(const QString &guid,
				 const QString &title,
				 const QString &description,
				 const QString &author,
				 const QStringList &categoryList,
				 const QUrl &link,
				 const QUrl &commentsLink,
				 const QDateTime &publishDate,
				 RssChannel *channel) :
	QObject(channel),
	_channel(channel),
	_guid(guid),
	_title(title),
	_description(description),
	_author(author),
	_categoryList(categoryList),
	_link(link),
	_commentsLink(commentsLink),
	_publishDate(publishDate),
	_image(_channel->iconWidth(), _channel->iconHeight())
{
	if (!_channel) {
		throw std::invalid_argument("RSS Channel is null");
	}

	connect(&_image, &RemoteImage::imageChanged, this, &RssItem::imageChanged);
}

const QString &RssItem::guid() const
{
	return _guid;
}

const QString &RssItem::title() const
{
	return _title;
}

const QString &RssItem::description() const
{
	return _description;
}

const QString &RssItem::author() const
{
	return _author;
}

const QStringList &RssItem::categoryList() const
{
	return _categoryList;
}

const QUrl &RssItem::link() const
{
	return _link;
}

const QUrl &RssItem::commentsLink() const
{
	return _commentsLink;
}

const QDateTime &RssItem::publishDate() const
{
	return _publishDate;
}

const QString RssItem::publishDateString() const
{
	return _publishDateString;
}

const QPixmap &RssItem::image() const
{
	if (!_image.isNull()) {
		return _image.image();
	}

	if (_imageFromSite && !_imageFromSite->isNull()) {
		return _imageFromSite->image();
	}

	if (!_channel) {
		throw std::invalid_argument("RSS Channel is null");
	}

	return _channel->icon();
}

bool RssItem::isValid() const
{
	return !_link.isEmpty() && !_title.isEmpty() && !_publishDate.isNull();
}

bool RssItem::isOld(const QDateTime &now) const
{
	return now.msecsTo(_publishDate) < -_maxLastHoursInMs;
}

bool RssItem::isRead() const
{
	return _isRead;
}

void RssItem::setIsRead(bool isRead)
{
	if (_isRead != isRead) {
		_isRead = isRead;
		emit isReadChanged();
	}
}

void RssItem::tryToGetImageLink(const QString &text)
{
	if (_image.link().isValid()) {
		return;
	}

	int imgTagIndex = text.indexOf("<img");

	if (imgTagIndex == -1) {
		return;
	}

	int srcAttributeStartIndex = text.indexOf("src=\"", imgTagIndex + 5);

	if (srcAttributeStartIndex == -1) {
		return;
	}

	int srcAttributeEndIndex = text.indexOf("\"", srcAttributeStartIndex + 6);

	if (srcAttributeEndIndex == -1) {
		return;
	}

	srcAttributeStartIndex += 5;

	QString urlString = text.mid(srcAttributeStartIndex,
								 srcAttributeEndIndex - srcAttributeStartIndex);

	if (urlString.isEmpty()) {
		return;
	}

	QUrl url(urlString);

	if (url.isValid()) {
		_image.setLink(url);
	}
}

void RssItem::markAsRead()
{
	setIsRead(true);
}

void RssItem::markAllNewsAtSiteAsRead()
{
	if (!_channel) {
		throw std::invalid_argument("RSS Channel is null");
	}

	_channel->markAsRead();
}

void RssItem::markAsUnRead()
{
	setIsRead(false);
}

bool RssItem::equalsTo(const QSharedPointer<RssItem> &item)
{
	return _link == item->link();
}

void RssItem::update(const QSharedPointer<RssItem> &item)
{
	_guid = item->_guid;
	_title = item->_title;
	_description = item->_description;
	_author = item->_author;
	_categoryList = item->_categoryList;
	_link = item->_link;
	_commentsLink = item->_commentsLink;
	_publishDate = item->_publishDate;
	_publishDateString = item->_publishDateString;
	_image.setLink(item->_image.link());

	if (_imageFromSite && item->_imageFromSite) {
		_imageFromSite->setLink(item->_imageFromSite->link());
		_imageFromSite->setImageLink(item->_imageFromSite->imageLink());
	} else if (!_imageFromSite && item->_imageFromSite) {
		createImageFromSite();

		_imageFromSite->setLink(item->_imageFromSite->link());
		_imageFromSite->setImageLink(item->_imageFromSite->imageLink());
	}

	// We do not change _isRead field in this method
}

void RssItem::parse(QXmlStreamReader &xml)
{
	_categoryList.clear();

	while (xml.readNextStartElement()) {
		if (!xml.prefix().isEmpty()) {
			if (xml.name() == QLatin1String("encoded")
					&& xml.namespaceUri() == "http://purl.org/rss/1.0/modules/content/") {
				tryToGetImageLink(xml.readElementText());
				continue;
			}

			xml.skipCurrentElement();
			continue;
		}

		QStringRef xmlName = xml.name();

		if (xmlName == QLatin1String("guid")) {
			_guid = xml.readElementText();
		} else if (xmlName == QLatin1String("title")) {
			const QString elementText = xml.readElementText();

			tryToGetImageLink(elementText);

			_title = removeHtmlTags(elementText);
		} else if (xmlName == QLatin1String("description")) {
			const QString elementText = xml.readElementText();

			tryToGetImageLink(elementText);

			_description = removeHtmlTags(elementText);
		} else if (xmlName == QLatin1String("author")) {
			_author = xml.readElementText();
		} else if (xmlName == QLatin1String("category")) {
			_categoryList.push_back(xml.readElementText());
		} else if (xmlName == QLatin1String("link")) {
			_link = xml.readElementText();
		} else if (xmlName == QLatin1String("comments")) {
			_commentsLink = xml.readElementText();
		} else if (xmlName == QLatin1String("pubDate")) {
			_publishDate = QDateTime::fromString(xml.readElementText(), Qt::RFC2822Date);
			_publishDateString =
					BettergramService::generateLastUpdateString(_publishDate.toLocalTime(), false);
		} else if (xmlName == QLatin1String("enclosure")) {
			QUrl url = QUrl(xml.attributes().value("url").toString());

			if (url.isValid()) {
				if (xml.attributes().value("type").contains("image")) {
					_image.setLink(url);
				}
			}
			xml.skipCurrentElement();
		} else {
			xml.skipCurrentElement();
		}
	}

	if (!_image.link().isValid()) {
		createImageFromSite();
		_imageFromSite->setLink(_link);
	}
}

void RssItem::parseAtom(QXmlStreamReader &xml)
{
	_categoryList.clear();

	while (xml.readNextStartElement()) {
		QStringRef xmlName = xml.name();
		QStringRef xmlNamespace = xml.namespaceUri();

		if (xmlNamespace.isEmpty() || xmlNamespace == "http://www.w3.org/2005/Atom") {
			if (xmlName == QLatin1String("id")) {
				_guid = xml.readElementText();
			} else if (xmlName == QLatin1String("title")) {
				_title = removeHtmlTags(xml.readElementText());
			} else if (xmlName == QLatin1String("category")) {
				_categoryList.push_back(xml.attributes().value("term").toString());
				xml.skipCurrentElement();
			} else if (xmlName == QLatin1String("link")) {
				_link = QUrl(xml.attributes().value("href").toString());
				xml.skipCurrentElement();
			} else if (xmlName == QLatin1String("category")) {
				_categoryList.push_back(xml.attributes().value("term").toString());
			} else if (xmlName == QLatin1String("published")) {
				if (_publishDate.isValid()) {
					xml.skipCurrentElement();
				} else {
					_publishDate = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
					_publishDateString =
							BettergramService::generateLastUpdateString(_publishDate.toLocalTime(), false);
				}
			} else if (xmlName == QLatin1String("updated")) {
				_publishDate = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
				_publishDateString =
						BettergramService::generateLastUpdateString(_publishDate.toLocalTime(), false);
			} else {
				xml.skipCurrentElement();
			}
		} else if (xmlNamespace  == "http://search.yahoo.com/mrss/") {
			if (xmlName == QLatin1String("group")) {
				parseAtomMediaGroup(xml);
			}
		} else {
			xml.skipCurrentElement();
		}
	}

	if (!_image.link().isValid()) {
		createImageFromSite();
		_imageFromSite->setLink(_link);
	}
}

void RssItem::parseAtomMediaGroup(QXmlStreamReader &xml)
{
	while (xml.readNextStartElement()) {
		QStringRef xmlName = xml.name();
		QStringRef xmlNamespace = xml.namespaceUri();

		if (xmlNamespace != "http://search.yahoo.com/mrss/") {
			xml.skipCurrentElement();
			continue;
		}

		if (xmlName == QLatin1String("description")) {
			_description = xml.readElementText();
		} else if (xmlName == QLatin1String("thumbnail")) {
			_image.setLink(QUrl(xml.attributes().value("url").toString()));
			xml.skipCurrentElement();
		} else {
			xml.skipCurrentElement();
		}
	}
}

void RssItem::load(QSettings &settings)
{
	_guid =  settings.value("guid").toString();
	_title = settings.value("title").toString();
	_description = settings.value("description").toString();
	_author =  settings.value("author").toString();
	_categoryList = settings.value("categoryList").toStringList();

	_link = settings.value("link").toUrl();
	_commentsLink = settings.value("commentsLink").toUrl();
	_publishDate = settings.value("publishDate").toDateTime();
	_image.setLink(settings.value("imageLink").toString());

	if (!_image.link().isValid() && _link.isValid()) {
		createImageFromSite();

		_imageFromSite->setLink(_link);
	}

	setIsRead(settings.value("isRead").toBool());
}

void RssItem::save(QSettings &settings)
{
	settings.setValue("guid", guid());
	settings.setValue("title", title());
	settings.setValue("description", description());
	settings.setValue("author", author());
	settings.setValue("categoryList", categoryList());

	settings.setValue("link", link());
	settings.setValue("commentsLink", commentsLink());
	settings.setValue("publishDate", publishDate());
	settings.setValue("imageLink", _image.link());

	settings.setValue("isRead", isRead());
}

QString RssItem::removeHtmlTags(const QString &text)
{
	QTextDocument textDocument;
	textDocument.setHtml(text);
	return textDocument.toPlainText();
}

void RssItem::createImageFromSite()
{
	if (_imageFromSite) {
		return;
	}

	if (!_channel) {
		throw std::invalid_argument("RSS Channel is null");
	}

	_imageFromSite = new ImageFromSite(_channel->iconWidth(), _channel->iconHeight(), this);

	connect(_imageFromSite, &ImageFromSite::imageChanged, this, &RssItem::imageChanged);
}

} // namespace Bettergrams
