#include "rssitem.h"
#include "rsschannel.h"
#include <QXmlStreamReader>

namespace Bettergram {

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
	return _publishDate.isNull() ? QString() : _publishDate.toString("hh:mm");
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
	//TODO: bettergram: realize RssItem::isValid() method
	return true;
}

bool RssItem::isRead() const
{
	return _isRead;
}

void RssItem::setIsRead(bool isRead)
{
	if (_isRead != isRead) {
		_isRead = isRead;
	}
}

void RssItem::markAsRead()
{
	setIsRead(true);
}

void RssItem::markAsUnRead()
{
	setIsRead(false);
}

void RssItem::parseItem(QXmlStreamReader &xml)
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
		} else {
			xml.skipCurrentElement();
		}
	}
}

QString RssItem::removeHtmlTags(QString text)
{
	QTextDocument textDocument;
	textDocument.setHtml(text);
	return textDocument.toPlainText();
}

} // namespace Bettergrams
