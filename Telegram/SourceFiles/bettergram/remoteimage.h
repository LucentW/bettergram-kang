#pragma once

#include "abstractremotefile.h"

namespace Bettergram {

/**
 * @brief The RemoteImage class is used to download and use remote images.
 */
class RemoteImage : public AbstractRemoteFile {
	Q_OBJECT

public:
	explicit RemoteImage(QObject *parent = nullptr);

	explicit RemoteImage(const QUrl &link, QObject *parent = nullptr);

	explicit RemoteImage(const QUrl &link,
						 int scaledWidth,
						 int scaledHeight,
						 QObject *parent = nullptr);

	explicit RemoteImage(int scaledWidth,
						 int scaledHeight,
						 QObject *parent = nullptr);

	int scaledWidth() const;
	void setScaledWidth(int scaledWidth);

	int scaledHeight() const;
	void setScaledHeight(int scaledHeight);

	void setScaledSize(int scaledWidth, int scaledHeight);

	const QPixmap &image() const;

public slots:

signals:
	void imageChanged();

protected:
	void dataDownloaded(const QByteArray &data) override;
	void resetData() override;

private:
	/// If _scaledWidth or _scaledHeight is not 0 then we scale fetched image
	int _scaledWidth = 0;
	int _scaledHeight = 0;

	QPixmap _image;

	void setImage(const QPixmap &image);
};

} // namespace Bettergram
