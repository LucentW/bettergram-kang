#include "bettergramservice.h"
#include "cryptopricelist.h"
#include "cryptoprice.h"
#include "rsschannellist.h"
#include "rsschannel.h"
#include "resourcegrouplist.h"
#include "aditem.h"

#include <messenger.h>
#include <lang/lang_keys.h>
#include <styles/style_chat_helpers.h>

#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace Bettergram {

BettergramService *BettergramService::_instance = nullptr;
const QString BettergramService::_defaultLastUpdateString = "...";

BettergramService *BettergramService::init()
{
	return instance();
}

BettergramService *BettergramService::instance()
{
	if (!_instance) {
		_instance = new BettergramService();
	}

	return _instance;
}

const QString &BettergramService::defaultLastUpdateString()
{
	return _defaultLastUpdateString;
}

QString BettergramService::generateLastUpdateString(const QDateTime &dateTime, bool isShowSeconds)
{
	if (dateTime.isNull()) {
		return _defaultLastUpdateString;
	}

	qint64 daysBefore = QDateTime::currentDateTime().daysTo(dateTime);

	const QString timeFormat = isShowSeconds ? "hh:mm:ss" : "hh:mm";
	const QString timeString = dateTime.toString(timeFormat);

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

void BettergramService::openUrl(const QUrl &url)
{
	QString urlString = url.toString();

	if (urlString.startsWith(QLatin1String("tg://"), Qt::CaseInsensitive)) {
		Messenger::Instance().openLocalUrl(urlString);
	} else {
		QDesktopServices::openUrl(url);
	}
}

Bettergram::BettergramService::BettergramService(QObject *parent) :
	QObject(parent),
	_cryptoPriceList(new CryptoPriceList(this)),
	_rssChannelList(new RssChannelList("news", st::newsPanImageSize, st::newsPanImageSize, this)),
	_videoChannelList(new RssChannelList("videos", st::newsPanImageSize, st::newsPanImageSize, this)),
	_resourceGroupList(new ResourceGroupList(this)),
	_currentAd(new AdItem(this))
{
	getIsPaid();
	getNextAd(true);

	_rssChannelList->load();

	if (_rssChannelList->isEmpty()) {
		_rssChannelList->add(QUrl("https://news.livecoinwatch.com/feed/"));
		_rssChannelList->add(QUrl("https://coincentral.com/feed/"));
		_rssChannelList->add(QUrl("https://www.coindesk.com/feed/"));
		_rssChannelList->add(QUrl("https://www.ccn.com/feed/"));
	}

	_videoChannelList->load();

	if (_videoChannelList->isEmpty()) {
		_videoChannelList->add(QUrl("https://www.youtube.com/channel/UCyC_4jvPzLiSkJkLIkA7B8g"));
	}

	getRssChannelList();
	getVideoChannelList();

	_resourceGroupList->parseFile(":/bettergram/default-resources.json");
	getResourceGroupList();
}

bool BettergramService::isPaid() const
{
	return _isPaid;
}

void BettergramService::setIsPaid(bool isPaid)
{
	if (_isPaid != isPaid) {
		_isPaid = isPaid;

		emit isPaidChanged();
		_isPaidObservable.notify();
	}
}

BettergramService::BillingPlan BettergramService::billingPlan() const
{
	return _billingPlan;
}

void BettergramService::setBillingPlan(BillingPlan billingPlan)
{
	if (_billingPlan != billingPlan) {
		_billingPlan = billingPlan;

		emit billingPlanChanged();
		_billingPlanObservable.notify();
	}
}

CryptoPriceList *BettergramService::cryptoPriceList() const
{
	return _cryptoPriceList;
}

RssChannelList *BettergramService::rssChannelList() const
{
	return _rssChannelList;
}

RssChannelList *BettergramService::videoChannelList() const
{
	return _videoChannelList;
}

ResourceGroupList *BettergramService::resourceGroupList() const
{
	return _resourceGroupList;
}

AdItem *BettergramService::currentAd() const
{
	return _currentAd;
}

bool BettergramService::isWindowActive() const
{
	return _isWindowActive;
}

void BettergramService::setIsWindowActive(bool isWindowActive)
{
	if (_isWindowActive != isWindowActive) {
		_isWindowActive = isWindowActive;

		if (_isWindowActiveHandler) {
			_isWindowActiveHandler();
		}
	}
}

base::Observable<void> &BettergramService::isPaidObservable()
{
	return _isPaidObservable;
}

base::Observable<void> &BettergramService::billingPlanObservable()
{
	return _billingPlanObservable;
}

void BettergramService::getIsPaid()
{
	//TODO: bettergram: ask server and get know if the instance is paid or not and the current billing plan.
	//					If the application is not paid then call getNextAd();
}

void BettergramService::getCryptoPriceList()
{
	QUrl url("https://http-api.livecoinwatch.com/bettergram/top10");

	QNetworkRequest request;
	request.setUrl(url);

	QNetworkReply *reply = _networkManager.get(request);

	connect(reply, &QNetworkReply::finished,
			this, &BettergramService::onGetCryptoPriceListFinished);

	connect(reply, &QNetworkReply::sslErrors,
			this, &BettergramService::onGetCryptoPriceListSslFailed);
}

void BettergramService::getRssChannelList()
{
	for (const QSharedPointer<RssChannel> &channel : *_rssChannelList) {
		if (channel->isMayFetchNewData()) {
			getRssFeeds(_rssChannelList, channel);
		}
	}
}

void BettergramService::getVideoChannelList()
{
	for (const QSharedPointer<RssChannel> &channel : *_videoChannelList) {
		if (channel->isMayFetchNewData()) {
			getRssFeeds(_videoChannelList, channel);
		}
	}
}

void BettergramService::getRssFeeds(RssChannelList *rssChannelList,
									const QSharedPointer<RssChannel> &channel)
{
	channel->startFetching();

	QNetworkRequest request;
	request.setUrl(channel->feedLink());

	QNetworkReply *reply = _networkManager.get(request);

	connect(reply, &QNetworkReply::finished, this, [rssChannelList, reply, channel] {
		if(reply->error() == QNetworkReply::NoError) {
			channel->fetchingSucceed(reply->readAll());
		} else {
			LOG(("Can not get RSS feeds from the channel %1. %2 (%3)")
				.arg(channel->feedLink().toString())
				.arg(reply->errorString())
				.arg(reply->error()));

			channel->fetchingFailed();
		}

		rssChannelList->parse();

		reply->deleteLater();
	});

	connect(reply, &QNetworkReply::sslErrors, this, [channel] (QList<QSslError> errors) {
		LOG(("Got SSL errors in during getting RSS feeds from the channel: %1")
			.arg(channel->feedLink().toString()));

		for(const QSslError &error : errors) {
			LOG(("%1").arg(error.errorString()));
		}
	});
}

void BettergramService::getResourceGroupList()
{
	//TODO: bettergram: We should get resources from server when it will be ready on server side
#if 0
	QUrl url("https://api.bettergram.io/v1/resources");

	QNetworkRequest request;
	request.setUrl(url);

	QNetworkReply *reply = _networkManager.get(request);

	connect(reply, &QNetworkReply::finished,
			this, &BettergramService::onGetResourceGroupListFinished);

	connect(reply, &QNetworkReply::sslErrors,
			this, &BettergramService::onGetResourceGroupListSslFailed);
#endif
}

void BettergramService::parseCryptoPriceList(const QByteArray &byteArray)
{
	if (byteArray.isEmpty()) {
		LOG(("Can not get crypto price list. Response is emtpy"));
		return;
	}

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);

	if (!doc.isObject()) {
		LOG(("Can not get crypto price list. Response is wrong. %1 (%2). Response: %3")
			.arg(parseError.errorString())
			.arg(parseError.error)
			.arg(QString::fromUtf8(byteArray)));
		return;
	}

	QJsonObject json = doc.object();

	if (json.isEmpty()) {
		LOG(("Can not get crypto price list. Response is emtpy or wrong"));
		return;
	}

	bool success = json.value("success").toBool();

	if (!success) {
		QString errorMessage = json.value("message").toString("Unknown error");
		LOG(("Can not get crypto price list. %1").arg(errorMessage));
		return;
	}

	double marketCap = json.value("marketCap").toDouble();

	// It is optionally parameter.
	// This parameter may contain number of seconds for the next update
	// (5, 60, 90 seconds and etc.).
	int freq = qAbs(json.value("freq").toInt());

	QList<CryptoPrice> priceList;

	QJsonArray priceListJson = json.value("prices").toArray();
	int i = 0;

	for (const QJsonValue &jsonValue : priceListJson) {
		QJsonObject priceJson = jsonValue.toObject();

		if (priceJson.isEmpty()) {
			LOG(("Price json is empty"));
			continue;
		}

		QString name = priceJson.value("name").toString();
		if (name.isEmpty()) {
			LOG(("Price name is empty"));
			continue;
		}

		QString shortName = priceJson.value("code").toString();
		if (shortName.isEmpty()) {
			LOG(("Price code is empty"));
			continue;
		}

		QString url = priceJson.value("url").toString();
		if (url.isEmpty()) {
			LOG(("Price url is empty"));
			continue;
		}

		QString iconUrl = priceJson.value("iconUrl").toString();
		if (iconUrl.isEmpty()) {
			LOG(("Price icon url is empty"));
			continue;
		}

		double price = priceJson.value("price").toDouble();
		double changeFor24Hours = priceJson.value("day").toDouble();
		bool isCurrentPriceGrown = priceJson.value("isGrown").toBool();

		CryptoPrice cryptoPrice(url, iconUrl, name, shortName, price, changeFor24Hours, isCurrentPriceGrown, i);
		priceList.push_back(cryptoPrice);

		i++;
	}

	_cryptoPriceList->updateData(marketCap, freq, priceList);
}

void BettergramService::onGetCryptoPriceListFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	if(reply->error() == QNetworkReply::NoError) {
		parseCryptoPriceList(reply->readAll());
	} else {
		LOG(("Can not get crypto price list. %1 (%2)")
			.arg(reply->errorString())
			.arg(reply->error()));
	}

	reply->deleteLater();
}

void BettergramService::onGetCryptoPriceListSslFailed(QList<QSslError> errors)
{
	for(const QSslError &error : errors) {
		LOG(("%1").arg(error.errorString()));
	}
}

void BettergramService::onGetResourceGroupListFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	if(reply->error() == QNetworkReply::NoError) {
		_resourceGroupList->parse(reply->readAll());
	} else {
		LOG(("Can not get resource group list. %1 (%2)")
			.arg(reply->errorString())
			.arg(reply->error()));
	}

	reply->deleteLater();
}

void BettergramService::onGetResourceGroupListSslFailed(QList<QSslError> errors)
{
	for(const QSslError &error : errors) {
		LOG(("%1").arg(error.errorString()));
	}
}

void BettergramService::getNextAd(bool reset)
{
	if(_isPaid) {
		_currentAd->clear();
		return;
	}

	QString url = "https://api.bettergram.io/v1/ads/next";

	if (!reset && !_currentAd->isEmpty()) {
		url += "?last=";
		url += _currentAd->id();
	}

	QNetworkRequest request;
	request.setUrl(url);

	QNetworkReply *reply = _networkManager.get(request);

	connect(reply, &QNetworkReply::finished,
			this, &BettergramService::onGetNextAdFinished);

	connect(reply, &QNetworkReply::sslErrors,
			this, &BettergramService::onGetNextAdSslFailed);
}

void BettergramService::getNextAdLater(bool reset)
{
	int delay = _currentAd->duration();

	if (delay <= 0) {
		delay = AdItem::defaultDuration();
	}

	QTimer::singleShot(delay * 1000, this, [this, reset]() {
		if (_isWindowActive) {
			_isWindowActiveHandler = nullptr;
			getNextAd(reset);
		} else {
			_isWindowActiveHandler = [this, reset]() {
				if (_isWindowActive) {
					_isWindowActiveHandler = nullptr;
					getNextAd(reset);
				}
			};
		}
	});
}

bool BettergramService::parseNextAd(const QByteArray &byteArray)
{
	if (byteArray.isEmpty()) {
		LOG(("Can not get next ad. Response is emtpy"));
		return false;
	}

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);

	if (!doc.isObject()) {
		LOG(("Can not get next ad. Response is wrong. %1 (%2). Response: %3")
			.arg(parseError.errorString())
			.arg(parseError.error)
			.arg(QString::fromUtf8(byteArray)));
		return false;
	}

	QJsonObject json = doc.object();

	if (json.isEmpty()) {
		LOG(("Can not get next ad. Response is emtpy or wrong"));
		return false;
	}

	bool isSuccess = json.value("success").toBool();

	if (!isSuccess) {
		QString errorMessage = json.value("message").toString("Unknown error");
		LOG(("Can not get next ad. %1").arg(errorMessage));
		return false;
	}

	QJsonObject adJson = json.value("ad").toObject();

	if (adJson.isEmpty()) {
		LOG(("Can not get next ad. Ad json is empty"));
		return false;
	}

	QString id = adJson.value("_id").toString();
	if (id.isEmpty()) {
		LOG(("Can not get next ad. Id is empty"));
		return false;
	}

	QString text = adJson.value("text").toString();
	if (text.isEmpty()) {
		LOG(("Can not get next ad. Text is empty"));
		return false;
	}

	QString url = adJson.value("url").toString();
	if (url.isEmpty()) {
		LOG(("Can not get next ad. Url is empty"));
		return false;
	}

	int duration = adJson.value("duration").toInt(AdItem::defaultDuration());

	AdItem adItem(id, text, url, duration);

	_currentAd->update(adItem);

	return true;
}

void BettergramService::onGetNextAdFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	if(reply->error() == QNetworkReply::NoError) {
		if (parseNextAd(reply->readAll())) {
			getNextAdLater();
		} else {
			// Try to get new ad without previous ad id
			getNextAdLater(true);
		}
	} else {
		//	LOG(("Can not get next ad item. %1 (%2)")
		//				  .arg(reply->errorString())
		//				  .arg(reply->error()));

		getNextAdLater();
	}

	reply->deleteLater();
}

void BettergramService::onGetNextAdSslFailed(QList<QSslError> errors)
{
	for(const QSslError &error : errors) {
		LOG(("%1").arg(error.errorString()));
	}
}

} // namespace Bettergrams
