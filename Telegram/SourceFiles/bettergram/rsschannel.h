#pragma once

#include <QObject>

class QXmlStreamReader;

namespace Bettergram {

class RssItem;

/**
 * @brief The RssChannel class contains information from a RSS channel.
 */
class RssChannel : public QObject {
	Q_OBJECT

public:
	typedef QList<QSharedPointer<RssItem>>::const_iterator const_iterator;

	static void sort(QList<QSharedPointer<RssItem>> &items);

	explicit RssChannel(QObject *parent = nullptr);
	explicit RssChannel(const QUrl &feedLink, QObject *parent = nullptr);

	const QString &title() const;
	void setTitle(const QString &title);

	const QString &description() const;
	void setDescription(const QString &description);

	const QString &language() const;
	void setLanguage(const QString &language);

	const QString &copyright() const;
	void setCopyright(const QString &copyright);

	const QString &editorEmail() const;
	void setEditorEmail(const QString &editorEmail);

	const QString &webMasterEmail() const;
	void setWebMasterEmail(const QString &webMasterEmail);

	const QStringList &categoryList() const;
	void setCategoryList(const QStringList &categoryList);

	const QDateTime &publishDate() const;
	void setPublishDate(const QDateTime &publishDate);

	const QDateTime &lastBuildDate() const;
	void setLastBuildDate(const QDateTime &lastBuildDate);

	const QString &skipHours() const;
	void setSkipHours(const QString &skipHours);

	const QString &skipDays() const;
	void setSkipDays(const QString &skipDays);

	const QUrl &feedLink() const;
	void setFeedLink(const QUrl &link);

	const QUrl &link() const;
	void setLink(const QUrl &link);

	const QUrl &image() const;
	void setImage(const QUrl &image);

	bool isFetching() const;

	const_iterator begin() const;
	const_iterator end() const;

	const QSharedPointer<RssItem> &at(int index) const;
	int count() const;
	int countUnread() const;

	bool isMayFetchNewData() const;

	void startFetching();
	void fetchingSucceed(const QByteArray &source);
	void fetchingFailed();

	/// Parse fetched source xml data and return true only when the data is changed
	bool parse();

public slots:

signals:
	void updated();

protected:

private:
	QString _title;
	QString _description;
	QString _language;
	QString _copyright;
	QString _editorEmail;
	QString _webMasterEmail;
	QStringList _categoryList;

	//TODO: bettergram: use publish date and last build date properties
	QDateTime _publishDate;
	QDateTime _lastBuildDate;

	//TODO: bettergram: use skip hours and skip days properties
	QString _skipHours;
	QString _skipDays;

	QUrl _feedLink;
	QUrl _link;
	QUrl _image;

	QByteArray _source;
	QByteArray _lastSourceHash;
	bool _isFetching = false;

	QList<QSharedPointer<RssItem>> _list;

	static bool compare(const QSharedPointer<RssItem> &a, const QSharedPointer<RssItem> &b);

	void setIsFetching(bool isFetching);

	QByteArray countSourceHash(const QByteArray &source) const;

	void parseRss(QXmlStreamReader &xml);
	void parseChannel(QXmlStreamReader &xml);
	void parseChannelImage(QXmlStreamReader &xml);
	void parseItem(QXmlStreamReader &xml);

	void merge(const QSharedPointer<RssItem> &item);
};

} // namespace Bettergram
