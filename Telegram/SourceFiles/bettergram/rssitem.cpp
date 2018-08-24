#include "rssitem.h"
#include "rsschannel.h"
#include "bettergramservice.h"

#include <QXmlStreamReader>

namespace Bettergram {

const qint64 RssItem::_maxLastHoursInMs = 24 * 60 * 60 * 1000;

RssItem::RssItem(RssChannel *channel) :
	QObject(channel),
	_channel(channel)
{
	if (!_channel) {
		throw std::invalid_argument("RSS Channel is null");
	}
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
	_publishDate(publishDate)
{
	if (!_channel) {
		throw std::invalid_argument("RSS Channel is null");
	}
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

const QPixmap &RssItem::icon() const
{
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

	// We do not change _isRead field in this method
}

void RssItem::parse(QXmlStreamReader &xml)
{
	_categoryList.clear();

	while (xml.readNextStartElement()) {
		if (xml.name() == QLatin1String("guid")) {
			_guid = xml.readElementText();
		} else if (xml.name() == QLatin1String("title")) {
			_title = removeHtmlTags(xml.readElementText());
		} else if (xml.name() == QLatin1String("description")) {
			_description = removeHtmlTags(xml.readElementText());
		} else if (xml.name() == QLatin1String("author")) {
			_author = xml.readElementText();
		} else if (xml.name() == QLatin1String("category")) {
			_categoryList.push_back(xml.readElementText());
		} else if (xml.name() == QLatin1String("link")) {
			_link = xml.readElementText();
		} else if (xml.name() == QLatin1String("comments")) {
			_commentsLink = xml.readElementText();
		} else if (xml.name() == QLatin1String("pubDate")) {
			_publishDate = QDateTime::fromString(xml.readElementText(), Qt::RFC2822Date);
			_publishDateString =
					BettergramService::generateLastUpdateString(_publishDate.toLocalTime(), false);
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
	settings.setValue("isRead", isRead());
}

QString RssItem::removeHtmlTags(QString text)
{
	QTextDocument textDocument;
	textDocument.setHtml(text);
	return textDocument.toPlainText();
}

} // namespace Bettergrams
