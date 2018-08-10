#include "rssitem.h"

namespace Bettergram {

RssItem::RssItem(const QString &title,
				 const QString &description,
				 const QString &author,
				 const QStringList &categoryList,
				 const QUrl &url,
				 const QUrl &commentsUrl,
				 const QDateTime &publishDate,
				 QObject *parent) :
	QObject(parent),
	_title(title),
	_description(description),
	_author(author),
	_categoryList(categoryList),
	_url(url),
	_commentsUrl(commentsUrl),
	_publishDate(publishDate)
{
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

const QUrl &RssItem::url() const
{
	return _url;
}

const QUrl &RssItem::commentsUrl() const
{
	return _commentsUrl;
}

const QDateTime &RssItem::publishDate() const
{
	return _publishDate;
}

bool RssItem::isValid() const
{
	//TODO: bettergram: realize RssItem::isValid() method
	return true;
}

} // namespace Bettergrams
