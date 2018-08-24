#include "resourcegrouplist.h"
#include "resourcegroup.h"

#include <bettergram/bettergramservice.h>
#include <logs.h>

#include <QJsonDocument>

namespace Bettergram {

const int ResourceGroupList::_defaultFreq = 60;

ResourceGroupList::ResourceGroupList(QObject *parent) :
	QObject(parent),
	_freq(_defaultFreq),
	_lastUpdateString(BettergramService::defaultLastUpdateString())
{
}

int ResourceGroupList::freq() const
{
	return _freq;
}

void ResourceGroupList::setFreq(int freq)
{
	if (freq == 0) {
		freq = _defaultFreq;
	}

	if (_freq != freq) {
		_freq = freq;
		emit freqChanged();
	}
}

QDateTime ResourceGroupList::lastUpdate() const
{
	return _lastUpdate;
}

QString ResourceGroupList::lastUpdateString() const
{
	return _lastUpdateString;
}

void ResourceGroupList::setLastUpdate(const QDateTime &lastUpdate)
{
	if (_lastUpdate != lastUpdate) {
		_lastUpdate = lastUpdate;

		_lastUpdateString = BettergramService::generateLastUpdateString(_lastUpdate, true);
		emit lastUpdateChanged();
	}
}

ResourceGroupList::const_iterator ResourceGroupList::begin() const
{
	return _list.begin();
}

ResourceGroupList::const_iterator ResourceGroupList::end() const
{
	return _list.end();
}

const QSharedPointer<ResourceGroup> &ResourceGroupList::at(int index) const
{
	if (index < 0 || index >= _list.size()) {
		LOG(("Index is out of bounds"));
		throw std::out_of_range("resource group index is out of range");
	}

	return _list.at(index);
}

bool ResourceGroupList::isEmpty() const
{
	return _list.isEmpty();
}

int ResourceGroupList::count() const
{
	return _list.count();
}

void ResourceGroupList::parseFile(const QString &filePath)
{
	QFile file(filePath);

	if (!file.open(QIODevice::ReadOnly)) {
		LOG(("Unable to open file '%1' with resource group list").arg(filePath));
		return;
	}

	parse(file.readAll());
}

void ResourceGroupList::parse(const QByteArray &byteArray)
{
	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);

	if (!doc.isObject()) {
		LOG(("Can not get resource group list. Data is wrong. %1 (%2). Data: %3")
			.arg(parseError.errorString())
			.arg(parseError.error)
			.arg(QString::fromUtf8(byteArray)));

		return;
	}

	QJsonObject json = doc.object();

	if (json.isEmpty()) {
		LOG(("Can not get resource group list. Data is emtpy or wrong"));
		return;
	}

	parse(json);
}

void ResourceGroupList::parse(const QJsonObject &json)
{
	if (json.isEmpty()) {
		return;
	}

	_list.clear();

	for (QJsonObject::const_iterator it = json.begin(); it != json.end(); ++it) {
		QJsonValue value = it.value();

		if (!value.isArray()) {
			LOG(("Unable to get json array for resource group"));
			continue;
		}

		QJsonArray jsonArray = value.toArray();

		QSharedPointer<ResourceGroup> group(new ResourceGroup(this));

		group->parse(it.key(), jsonArray);
		_list.push_back(group);
	}

	setLastUpdate(QDateTime::currentDateTime());
	emit updated();
}

} // namespace Bettergrams
