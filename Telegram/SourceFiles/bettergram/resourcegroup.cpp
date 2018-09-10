#include "resourcegroup.h"
#include "resourceitem.h"

#include <logs.h>
#include <styles/style_chat_helpers.h>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace Bettergram {

ResourceGroup::ResourceGroup(QObject *parent) :
	QObject(parent)
{
}

const QString &ResourceGroup::title() const
{
	return _title;
}

ResourceGroup::const_iterator ResourceGroup::begin() const
{
	return _list.begin();
}

ResourceGroup::const_iterator ResourceGroup::end() const
{
	return _list.end();
}

const QSharedPointer<ResourceItem> &ResourceGroup::at(int index) const
{
	if (index < 0 || index >= _list.size()) {
		throw std::out_of_range("Unable to get ResourceItem at wrong index");
	}

	return _list.at(index);
}

int ResourceGroup::count() const
{
	return _list.count();
}

bool ResourceGroup::isEmpty() const
{
	return _list.isEmpty();
}

const QList<QSharedPointer<ResourceItem> > &ResourceGroup::items() const
{
	return _list;
}

void ResourceGroup::parse(const QJsonObject &json)
{
	if (json.isEmpty()) {
		return;
	}

	_title = json.value("title").toString();

	for (const QJsonValue &value : json.value("items").toArray()) {
		if (!value.isObject()) {
			LOG(("Unable to get json object for resource item"));
			continue;
		}

		QSharedPointer<ResourceItem> item(new ResourceItem(this));

		connect(item.data(), &ResourceItem::iconChanged, this, &ResourceGroup::iconChanged);

		item->parse(value.toObject());
		_list.push_back(item);
	}
}

} // namespace Bettergrams
