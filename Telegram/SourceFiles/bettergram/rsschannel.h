#pragma once

#include <QObject>

namespace Bettergram {

class RssItem;

/**
 * @brief The RssChannel class contains information from a RSS channel.
 */
class RssChannel : public QObject {
	Q_OBJECT

public:
	typedef QList<RssItem*>::const_iterator const_iterator;

	explicit RssChannel(QObject *parent = nullptr);

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

	const QUrl &link() const;
	void setLink(const QUrl &link);

	const QUrl &image() const;
	void setImage(const QUrl &image);

	bool isFetching() const;

	const_iterator begin() const;
	const_iterator end() const;

	RssItem *at(int index) const;
	int count() const;

	bool isMayFetchNewData() const;

	void startFetching();
	void fetchingSucceed(const QString &source);
	void fetchingFailed();

	void parse();
	void updateData(const QList<RssItem*> &rssItems);

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
	QDateTime _publishDate;
	QDateTime _lastBuildDate;
	QString _skipHours;
	QString _skipDays;

	QUrl _link;
	QUrl _image;

	QString _source;
	bool _isFetching = false;

	QList<RssItem*> _list;

	void setIsFetching(bool isFetching);
};

} // namespace Bettergram
