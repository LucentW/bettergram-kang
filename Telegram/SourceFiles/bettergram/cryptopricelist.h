#pragma once

#include <QObject>

namespace Bettergram {

class CryptoPrice;

/**
 * @brief The CryptoPriceList class contains list of CryptoPrice instances.
 */
class CryptoPriceList : public QObject {
	Q_OBJECT

public:
	enum class SortOrder {
		Origin,

		NameAscending,
		NameDescending,

		PriceAscending,
		PriceDescending,

		ChangeFor24hAscending,
		ChangeFor24hDescending,
	};

	typedef QList<CryptoPrice*>::const_iterator const_iterator;

	explicit CryptoPriceList(QObject *parent = nullptr);

	double marketCap() const;
	QString marketCapString() const;
	void setMarketCap(double marketCap);

	int freq() const;

	QDateTime lastUpdate() const;
	QString lastUpdateString() const;

	const_iterator begin() const;
	const_iterator end() const;

	CryptoPrice *at(int index) const;
	int count() const;

	SortOrder sortOrder() const;
	void setSortOrder(const SortOrder &sortOrder);

	void updateData(double marketCap, int freq, const QList<CryptoPrice> &priceList);

	void createTestData();

public slots:

signals:
	void marketCapChanged();
	void freqChanged();
	void sortOrderChanged();
	void updated();

protected:

private:
	/// Default frequency of updates in seconds
	static const int _defaultFreq;

	QList<CryptoPrice*> _list;
	double _marketCap = 0.0;

	/// Frequency of updates in seconds
	int _freq;

	QDateTime _lastUpdate;
	QString _lastUpdateString;

	SortOrder _sortOrder = SortOrder::Origin;

	static bool containsName(const QList<CryptoPrice> &priceList, const QString &name);

	static bool sortByOriginSortIndex(const CryptoPrice *price1, const CryptoPrice *price2);
	static bool sortByName(const CryptoPrice *price1, const CryptoPrice *price2);
	static bool sortByPrice(const CryptoPrice *price1, const CryptoPrice *price2);
	static bool sortBy24h(const CryptoPrice *price1, const CryptoPrice *price2);

	void setFreq(int freq);
	void setLastUpdate(const QDateTime &lastUpdate);

	CryptoPrice *findByName(const QString &name);
	void sort();

	void addTestData(const QUrl &url,
					 const QUrl &iconUrl,
					 const QString &name,
					 const QString &shortName,
					 double currentPrice,
					 double changeFor24Hours,
					 bool isCurrentPriceGrown);
	void clear();
};

} // namespace Bettergram
