#pragma once

#include <QObject>

namespace Bettergram {

/**
 * @brief The AbstractRemoteFile class is used to download and use remote files.
 */
class AbstractRemoteFile : public QObject {
	Q_OBJECT

public:
	explicit AbstractRemoteFile(QObject *parent = nullptr);
	explicit AbstractRemoteFile(const QUrl &link, QObject *parent = nullptr);

	const QUrl &link() const;
	void setLink(const QUrl &link);

public slots:

signals:
	void linkChanged();

protected:
	virtual void dataDownloaded(const QByteArray &data) = 0;
	virtual void resetData() = 0;

	void download();

private:
	QUrl _link;

	void downloadLater();
};

} // namespace Bettergram
