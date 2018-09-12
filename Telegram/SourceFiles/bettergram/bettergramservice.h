#pragma once

#include "base/observer.h"
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <functional>

namespace Bettergram {

class CryptoPriceList;
class RssChannelList;
class RssChannel;
class ResourceGroupList;
class AdItem;

/**
 * @brief The BettergramService class contains Bettergram specific classes and settings
 */
class BettergramService : public QObject {
	Q_OBJECT

public:
	enum class BillingPlan {
		Unknown,
		Monthly,
		Quarterly,
		Yearly
	};

	static BettergramService *init();
	static BettergramService *instance();

	static const QString &defaultLastUpdateString();
	static QString generateLastUpdateString(const QDateTime &dateTime, bool isShowSeconds);

	static void openUrl(const QUrl &url);

	bool isPaid() const;
	BillingPlan billingPlan() const;

	CryptoPriceList *cryptoPriceList() const;
	RssChannelList *rssChannelList() const;
	RssChannelList *videoChannelList() const;
	ResourceGroupList *resourceGroupList() const;
	AdItem *currentAd() const;

	bool isWindowActive() const;
	void setIsWindowActive(bool isWindowActive);

	base::Observable<void> &isPaidObservable();
	base::Observable<void> &billingPlanObservable();

	/// Download and parse crypto price list
	void getCryptoPriceList();

	/// Download and parse all RSS feeds
	void getRssChannelList();

	/// Download and parse all Video feeds
	void getVideoChannelList();

	/// Download and parse resource group list
	void getResourceGroupList();

public slots:

signals:
	void isPaidChanged();
	void billingPlanChanged();

protected:
	void timerEvent(QTimerEvent *timerEvent) override;

private:
	static BettergramService *_instance;
	static const QString _defaultLastUpdateString;
	static const int _checkForFirstUpdatesDelay;
	static const int _checkForUpdatesPeriod;

	QNetworkAccessManager _networkManager;

	bool _isPaid = false;
	BillingPlan _billingPlan = BillingPlan::Unknown;

	CryptoPriceList *_cryptoPriceList = nullptr;
	RssChannelList *_rssChannelList = nullptr;
	RssChannelList *_videoChannelList = nullptr;
	ResourceGroupList *_resourceGroupList = nullptr;
	AdItem *_currentAd = nullptr;
	int _checkForUpdatesTimerId = 0;
	bool _isWindowActive = true;
	std::function<void()> _isWindowActiveHandler = nullptr;

	static void checkForNewUpdates();

	base::Observable<void> _isPaidObservable;
	base::Observable<void> _billingPlanObservable;

	explicit BettergramService(QObject *parent = nullptr);

	void setIsPaid(bool isPaid);
	void setBillingPlan(BillingPlan billingPlan);

	void getIsPaid();
	void getNextAd(bool reset);
	void getNextAdLater(bool reset = false);

	void parseCryptoPriceList(const QByteArray &byteArray);
	bool parseNextAd(const QByteArray &byteArray);

	void getRssFeeds(RssChannelList *rssChannelList, const QSharedPointer<RssChannel> &channel);

private slots:
	void onUpdateRssChannelList();
	void onUpdateVideoChannelList();

	void onGetCryptoPriceListFinished();
	void onGetCryptoPriceListSslFailed(QList<QSslError> errors);

	void onGetNextAdFinished();
	void onGetNextAdSslFailed(QList<QSslError> errors);

	void onGetResourceGroupListFinished();
	void onGetResourceGroupListSslFailed(QList<QSslError> errors);
};

} // namespace Bettergram
