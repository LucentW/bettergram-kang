#pragma once

#include "remoteimage.h"

#include <QObject>

class QXmlStreamReader;

namespace Bettergram {

class RssChannel;

/**
 * @brief The RssItem class contains information from a RSS item.
 */
class RssItem : public QObject {
	Q_OBJECT

public:
	explicit RssItem(RssChannel *channel);

	explicit RssItem(const QString &guid,
					 const QString &title,
					 const QString &description,
					 const QString &author,
					 const QStringList &categoryList,
					 const QUrl &link,
					 const QUrl &commentsLink,
					 const QDateTime &publishDate,
					 RssChannel *channel);

	const QString &guid() const;
	const QString &title() const;
	const QString &description() const;
	const QString &author() const;
	const QStringList &categoryList() const;
	const QUrl &link() const;
	const QUrl &commentsLink() const;
	const QDateTime &publishDate() const;
	const QString publishDateString() const;
	const QPixmap &image() const;

	bool isValid() const;
	bool isOld(const QDateTime &now = QDateTime::currentDateTime()) const;

	/// Return true if user marks this news as read
	bool isRead() const;
	void markAsRead();
	void markAllNewsAtSiteAsRead();
	void markAsUnRead();

	bool equalsTo(const QSharedPointer<RssItem> &item);
	void update(const QSharedPointer<RssItem> &item);

	void parse(QXmlStreamReader &xml);

	void load(QSettings &settings);
	void save(QSettings &settings);

public slots:

signals:
	void isReadChanged();
	void imageChanged();

protected:

private:
	/// Keep news only for the last hours
	static const qint64 _maxLastHoursInMs;

	RssChannel *const _channel;

	QString _guid;
	QString _title;
	QString _description;
	QString _author;
	QStringList _categoryList;

	QUrl _link;

	QUrl _commentsLink;
	QDateTime _publishDate;
	QString _publishDateString;

	/// We try to get _imageLink from <enclosure url="link-to-image" type="image/..."/>,
	/// or from <description>Text <img src="link-to-image"></description>,
	/// or from <title>Text <img src="link-to-image"></title> tags
	RemoteImage _image;

	bool _isRead = false;

	static QString removeHtmlTags(const QString &text);

	void setIsRead(bool isRead);
	void tryToGetImageLink(const QString &text);
};

} // namespace Bettergram
