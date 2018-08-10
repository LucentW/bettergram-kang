#pragma once

#include <QObject>

namespace Bettergram {

/**
 * @brief The RssItem class contains information from a RSS item.
 */
class RssItem : public QObject {
	Q_OBJECT

public:
	explicit RssItem(const QString &title,
					 const QString &description,
					 const QString &author,
					 const QStringList &categoryList,
					 const QUrl &url,
					 const QUrl &commentsUrl,
					 const QDateTime &publishDate,
					 QObject *parent = nullptr);

	const QString &title() const;
	const QString &description() const;
	const QString &author() const;
	const QStringList &categoryList() const;
	const QUrl &url() const;
	const QUrl &commentsUrl() const;
	const QDateTime &publishDate() const;

	bool isValid() const;

public slots:

signals:

protected:

private:
	const QString _title;
	const QString _description;
	const QString _author;
	const QStringList _categoryList;

	const QUrl _url;
	const QUrl _commentsUrl;
	const QDateTime _publishDate;
};

} // namespace Bettergram
