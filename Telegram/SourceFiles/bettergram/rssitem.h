#pragma once

#include <QObject>

class QXmlStreamReader;

namespace Bettergram {

/**
 * @brief The RssItem class contains information from a RSS item.
 */
class RssItem : public QObject {
	Q_OBJECT

public:
	explicit RssItem(QObject *parent = nullptr);

	explicit RssItem(const QString &guid,
					 const QString &title,
					 const QString &description,
					 const QString &author,
					 const QStringList &categoryList,
					 const QUrl &link,
					 const QUrl &commentsLink,
					 const QDateTime &publishDate,
					 QObject *parent = nullptr);

	const QString &guid() const;
	const QString &title() const;
	const QString &description() const;
	const QString &author() const;
	const QStringList &categoryList() const;
	const QUrl &link() const;
	const QUrl &commentsLink() const;
	const QDateTime &publishDate() const;
	const QString publishDateString() const;

	bool isValid() const;

	/// Return true if user marks this news as read
	bool isRead() const;
	void markAsRead();
	void markAsUnRead();

	void parseItem(QXmlStreamReader &xml);

public slots:

signals:

protected:

private:
	QString _guid;
	QString _title;
	QString _description;
	QString _author;
	QStringList _categoryList;

	QUrl _link;
	QUrl _commentsLink;
	QDateTime _publishDate;

	bool _isRead = false;

	void setIsRead(bool isRead);
};

} // namespace Bettergram
