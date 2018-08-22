#pragma once

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
	const QPixmap &icon() const;

	bool isValid() const;

	/// Return true if user marks this news as read
	bool isRead() const;
	void markAsRead();
	void markAllNewsAtSiteAsRead();
	void markAsUnRead();

	void parse(QXmlStreamReader &xml);

	void load(QSettings &settings);
	void save(QSettings &settings);

public slots:

signals:
	void isReadChanged();

protected:

private:
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

	bool _isRead = false;

	static QString removeHtmlTags(QString text);
	static QString countPublishDateString(const QDateTime &dateTime);

	void setIsRead(bool isRead);
};

} // namespace Bettergram
