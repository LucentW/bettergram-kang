#include "rssitem.h"
#include "rsschannel.h"
#include <lang/lang_keys.h>

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
			_publishDateString = countPublishDateString(_publishDate.toLocalTime());
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

QString RssItem::countPublishDateString(const QDateTime &dateTime)
{
	if (dateTime.isNull()) {
		return QString();
	}

	qint64 daysBefore = QDateTime::currentDateTime().daysTo(dateTime);
	const QString timeString = dateTime.toString("hh:mm");

	if (daysBefore == 0) {
		return lng_player_message_today(lt_time, timeString);
	} else if (daysBefore == -1) {
		return lng_player_message_yesterday(lt_time, timeString);
	} else {
		return lng_player_message_date(lt_date,
									   langDayOfMonthFull(dateTime.date()),
									   lt_time,
									   timeString);
	}
}

} // namespace Bettergrams
