#include "rsschannel.h"
#include "rssitem.h"
#include <logs.h>

namespace Bettergram {

RssChannel::RssChannel(QObject *parent) :
	QObject(parent)
{
}

RssChannel::RssChannel(const QUrl &link, QObject *parent) :
	QObject(parent),
	_link(link)
{
}

const QString &RssChannel::title() const
{
	return _title;
}

void RssChannel::setTitle(const QString &title)
{
	_title = title;
}

const QString &RssChannel::description() const
{
	return _description;
}

void RssChannel::setDescription(const QString &description)
{
	_description = description;
}

const QString &RssChannel::language() const
{
	return _language;
}

void RssChannel::setLanguage(const QString &language)
{
	_language = language;
}

const QString &RssChannel::copyright() const
{
	return _copyright;
}

void RssChannel::setCopyright(const QString &copyright)
{
	_copyright = copyright;
}

const QString &RssChannel::editorEmail() const
{
	return _editorEmail;
}

void RssChannel::setEditorEmail(const QString &editorEmail)
{
	_editorEmail = editorEmail;
}

const QString &RssChannel::webMasterEmail() const
{
	return _webMasterEmail;
}

void RssChannel::setWebMasterEmail(const QString &webMasterEmail)
{
	_webMasterEmail = webMasterEmail;
}

const QStringList &RssChannel::categoryList() const
{
	return _categoryList;
}

void RssChannel::setCategoryList(const QStringList &categoryList)
{
	_categoryList = categoryList;
}

const QDateTime &RssChannel::publishDate() const
{
	return _publishDate;
}

void RssChannel::setPublishDate(const QDateTime &publishDate)
{
	_publishDate = publishDate;
}

const QDateTime &RssChannel::lastBuildDate() const
{
	return _lastBuildDate;
}

void RssChannel::setLastBuildDate(const QDateTime &lastBuildDate)
{
	_lastBuildDate = lastBuildDate;
}

const QString &RssChannel::skipHours() const
{
	return _skipHours;
}

void RssChannel::setSkipHours(const QString &skipHours)
{
	_skipHours = skipHours;
}

const QString &RssChannel::skipDays() const
{
	return _skipDays;
}

void RssChannel::setSkipDays(const QString &skipDays)
{
	_skipDays = skipDays;
}

const QUrl &RssChannel::link() const
{
	return _link;
}

void RssChannel::setLink(const QUrl &link)
{
	_link = link;
}

const QUrl &RssChannel::image() const
{
	return _image;
}

void RssChannel::setImage(const QUrl &image)
{
	_image = image;
}

bool RssChannel::isFetching() const
{
	return _isFetching;
}

void RssChannel::setIsFetching(bool isFetching)
{
	_isFetching = isFetching;
}

RssChannel::const_iterator RssChannel::begin() const
{
	return _list.begin();
}

RssChannel::const_iterator RssChannel::end() const
{
	return _list.end();
}

RssItem *RssChannel::at(int index) const
{
	if (index < 0 || index >= _list.size()) {
		LOG(("Index is out of bounds"));
		return nullptr;
	}

	return _list.at(index);
}

int RssChannel::count() const
{
	return _list.count();
}

bool RssChannel::isMayFetchNewData() const
{
	//TODO: bettergram: realize RssChannel::isMayFetchNewData() method
	return true;
}

void RssChannel::startFetching()
{
	setIsFetching(true);
}

void RssChannel::fetchingSucceed(const QString &source)
{
	_source = source;
	setIsFetching(false);
}

void RssChannel::fetchingFailed()
{
	_source.clear();
	setIsFetching(false);
}

void RssChannel::parse()
{
	//TODO: bettergram: realize RssChannel::parse() method
}

void RssChannel::updateData(const QList<RssItem*> &rssItems)
{
	//TODO: bettergram: realize RssChannel::updateData() method
}

} // namespace Bettergrams
