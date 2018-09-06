#include "remotetempdata.h"

namespace Bettergram {

RemoteTempData::RemoteTempData(QObject *parent) :
	AbstractRemoteFile(parent)
{
}

RemoteTempData::RemoteTempData(const QUrl &link, QObject *parent) :
	AbstractRemoteFile(link, parent)
{
}

void RemoteTempData::dataDownloaded(const QByteArray &data)
{
	emit downloaded(data);
}

void RemoteTempData::resetData()
{
}

} // namespace Bettergrams
