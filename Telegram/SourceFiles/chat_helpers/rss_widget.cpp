#include "rss_widget.h"

#include "bettergram/bettergramsettings.h"
#include "bettergram/rsschannellist.h"
#include "bettergram/rsschannel.h"
#include "bettergram/rssitem.h"

#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "lang/lang_keys.h"
#include "styles/style_window.h"
#include "core/click_handler_types.h"
#include "styles/style_chat_helpers.h"
#include "styles/style_widgets.h"

#include <QMouseEvent>

namespace ChatHelpers {

using namespace Bettergram;

class RssWidget::Footer : public TabbedSelector::InnerFooter
{
public:
	Footer(not_null<RssWidget*> parent);

protected:
	void processPanelHideFinished() override;
	void resizeEvent(QResizeEvent* e) override;
	void onFooterClicked();

private:
	not_null<RssWidget*> _parent;
	object_ptr<Ui::LinkButton> _link;
};

RssWidget::Footer::Footer(not_null<RssWidget*> parent)
	:  InnerFooter(parent)
	, _parent(parent)
	, _link(object_ptr<Ui::LinkButton>(this, lang(lng_prices_footer), st::largeLinkButton))
{
	_link->setClickedCallback([this] { onFooterClicked(); });
}

void RssWidget::Footer::resizeEvent(QResizeEvent* e)
{
	_link->move(rect().center() - _link->rect().center());
}

void RssWidget::Footer::processPanelHideFinished()
{
}

void RssWidget::Footer::onFooterClicked()
{
	QDesktopServices::openUrl(QUrl("https://www.livecoinwatch.com"));
}

RssWidget::RssWidget(QWidget* parent, not_null<Window::Controller*> controller)
	: Inner(parent, controller)
{
	_siteName = new Ui::IconButton(this, st::pricesPanSiteNameIcon);
	_siteName->setClickedCallback([] { QDesktopServices::openUrl(QUrl("https://www.livecoinwatch.com")); });

	_marketCap = new Ui::FlatLabel(this, st::pricesPanMarketCapLabel);
//	_marketCap->setRichText(textcmdLink(1, lang(lng_prices_market_cap)
//										.arg(BettergramSettings::instance()->cryptoPriceList()->marketCapString())));
	_marketCap->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

	BettergramSettings::instance()->getRssChannelList();

	updateControlsGeometry();

	RssChannelList *rssChannelList = BettergramSettings::instance()->rssChannelList();
	connect(rssChannelList, &RssChannelList::updated, this, &RssWidget::onRssUpdated);

	setMouseTracking(true);
}

void RssWidget::refreshRecent()
{
}

void RssWidget::clearSelection()
{
}

object_ptr<TabbedSelector::InnerFooter> RssWidget::createFooter()
{
	Expects(_footer == nullptr);

	auto res = object_ptr<Footer>(this);

	_footer = res;
	return std::move(res);
}

void RssWidget::afterShown()
{
	startRssTimer();

	BettergramSettings::instance()->getRssChannelList();
}

void RssWidget::beforeHiding()
{
	stopRssTimer();
}

void RssWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _timerId) {
		BettergramSettings::instance()->getRssChannelList();
	}
}

void RssWidget::startRssTimer()
{
	if (!_timerId) {
		_timerId = startTimer(BettergramSettings::instance()->rssChannelList()->freq() * 1000);

		if (!_timerId) {
			LOG(("Can not start timer for %1 ms")
				.arg(BettergramSettings::instance()->rssChannelList()->freq()));
		}
	}
}

void RssWidget::stopRssTimer()
{
	if (_timerId) {
		killTimer(_timerId);
		_timerId = 0;
	}
}

void RssWidget::setSelectedRow(int selectedRow)
{
	if (_selectedRow != selectedRow) {
		_selectedRow = selectedRow;

		if (_selectedRow >= 0) {
			setCursor(style::cur_pointer);
		} else {
			setCursor(style::cur_default);
		}

		update();
	}
}

int RssWidget::getTableTop() const
{
	//TODO: bettergram: realize it?
	return 0;
	//return _marketCap->y() + _marketCap->height() + st::pricesPanPadding;
}

int RssWidget::getTableBottom() const
{
	//TODO: bettergram: realize it?
	return 0;
	//return getTableContentTop() + getTableContentHeight();
}

int RssWidget::getTableContentTop() const
{
	return getTableTop() + st::pricesPanTableHeaderHeight;
}

int RssWidget::getRowTop(int row) const
{
	return getTableContentTop() + row * st::pricesPanTableRowHeight;
}

QRect RssWidget::getTableRectangle() const
{
	return QRect(0,
				 getTableTop(),
				 width(),
				 getTableBottom());
}

QRect RssWidget::getTableHeaderRectangle() const
{
	return QRect(0,
				 getTableTop(),
				 width(),
				 st::pricesPanTableHeaderHeight);
}

QRect RssWidget::getTableContentRectangle() const
{
	//TODO: bettergram: realize it?
	return QRect();
//	return QRect(0,
//				 getTableContentTop(),
//				 width(),
//				 getTableContentHeight());
}

QRect RssWidget::getRowRectangle(int row) const
{
	return QRect(0,
				 getRowTop(row),
				 width(),
				 st::pricesPanTableRowHeight);
}

void RssWidget::countSelectedRow(const QPoint &point)
{
	if (_selectedRow == -1) {
		if (!getTableContentRectangle().contains(point)) {
			// Nothing changed
			return;
		}
	} else if (getRowRectangle(_selectedRow).contains(point)) {
		// Nothing changed
		return;
	}

	if (!getTableContentRectangle().contains(point)) {
		setSelectedRow(-1);
		return;
	}

	int rowCount = BettergramSettings::instance()->rssChannelList()->count();

	for (int row = 0; row < rowCount; row++) {
		if (getRowRectangle(row).contains(point)) {
			setSelectedRow(row);
			return;
		}
	}

	setSelectedRow(-1);
}

TabbedSelector::InnerFooter* RssWidget::getFooter() const
{
	return _footer;
}

int RssWidget::countDesiredHeight(int newWidth)
{
	Q_UNUSED(newWidth);

	return getTableBottom();
}

void RssWidget::mousePressEvent(QMouseEvent *e)
{
	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));

	_pressedRow = _selectedRow;
}

void RssWidget::mouseReleaseEvent(QMouseEvent *e)
{
	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));

//	RssChannelList *rssChannelList = BettergramSettings::instance()->cryptoPriceList();

//	if (_pressedRow >= 0 && _pressedRow < priceList->count() && _pressedRow == _selectedRow) {
//		QUrl url = priceList->at(_pressedRow)->url();
//		if (!url.isEmpty()) {
//			QDesktopServices::openUrl(url);
//		}
//	}
}

void RssWidget::mouseMoveEvent(QMouseEvent *e)
{
	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));
}

void RssWidget::enterEventHook(QEvent *e)
{
	QPoint point = mapFromGlobal(QCursor::pos());
	countSelectedRow(point);
}

void RssWidget::leaveEventHook(QEvent *e)
{
	Q_UNUSED(e);

	setSelectedRow(-1);
}

void RssWidget::paintEvent(QPaintEvent *event) {
	Painter painter(this);
	QRect r = event ? event->rect() : rect();

	if (r != rect()) {
		painter.setClipRect(r);
	}

	painter.fillRect(r, st::pricesPanBg);

	int top = getTableContentTop();

//	int columnCoinWidth = _coinHeader->width()
//			- _coinHeader->contentsMargins().left()
//			- _coinHeader->contentsMargins().right()
//			- st::pricesPanTableImageSize
//			- st::pricesPanTablePadding;

//	int columnPriceWidth = _priceHeader->width()
//			- _priceHeader->contentsMargins().left()
//			- _priceHeader->contentsMargins().right();

//	int column24hWidth = _24hHeader->width()
//			- _24hHeader->contentsMargins().left()
//			- _24hHeader->contentsMargins().right();

//	int columnCoinLeft = _coinHeader->x() + _coinHeader->contentsMargins().left();
//	int columnPriceLeft = _priceHeader->x() + _priceHeader->contentsMargins().left();
//	int column24hLeft = _24hHeader->x() + _24hHeader->contentsMargins().left();

//	// Draw rows

//	int columnCoinTextLeft = columnCoinLeft + st::pricesPanTableImageSize + st::pricesPanTablePadding;
//	int rowCount = BettergramSettings::instance()->cryptoPriceList()->count();

//	if (_selectedRow != -1 && _selectedRow < rowCount) {
//		QRect rowRectangle(0, getRowTop(_selectedRow), width(), st::pricesPanTableRowHeight);
//		App::roundRect(painter, rowRectangle, st::pricesPanHover, StickerHoverCorners);
//	}

//	painter.setFont(st::semiboldFont);

//	for (const CryptoPrice *price : *BettergramSettings::instance()->cryptoPriceList()) {
//		if (!price->icon().isNull()) {
//			QRect targetRect(columnCoinLeft,
//							 top + (st::pricesPanTableRowHeight - st::pricesPanTableImageSize) / 2,
//							 st::pricesPanTableImageSize,
//							 st::pricesPanTableImageSize);

//			painter.drawPixmap(targetRect, price->icon());
//		}

//		painter.setPen(st::pricesPanTableCryptoNameFg);

//		painter.drawText(columnCoinTextLeft, top, columnCoinWidth, st::pricesPanTableRowHeight / 2,
//						 Qt::AlignLeft | Qt::AlignBottom, price->name());

//		painter.setPen(st::pricesPanTableCryptoShortNameFg);

//		painter.drawText(columnCoinTextLeft, top + st::pricesPanTableRowHeight / 2, columnCoinWidth, st::pricesPanTableRowHeight / 2,
//						 Qt::AlignLeft | Qt::AlignTop, price->shortName());

//		if (price->isCurrentPriceGrown()) {
//			painter.setPen(st::pricesPanTableUpFg);
//		} else {
//			painter.setPen(st::pricesPanTableDownFg);
//		}

//		painter.drawText(columnPriceLeft, top, columnPriceWidth, st::pricesPanTableRowHeight,
//						 Qt::AlignRight | Qt::AlignVCenter, price->currentPriceString());

//		if (price->isChangeFor24HoursGrown()) {
//			painter.setPen(st::pricesPanTableUpFg);
//		} else {
//			painter.setPen(st::pricesPanTableDownFg);
//		}

//		painter.drawText(column24hLeft, top, column24hWidth, st::pricesPanTableRowHeight,
//						 Qt::AlignRight | Qt::AlignVCenter, price->changeFor24HoursString());

//		top += st::pricesPanTableRowHeight;
//	}
}

void RssWidget::resizeEvent(QResizeEvent *e)
{
	updateControlsGeometry();
}

void RssWidget::updateControlsGeometry()
{
	_siteName->moveToLeft((width() - _siteName->width()) / 2, st::pricesPanHeader);

	updateMarketCap();

	int columnCoinWidth = width() - st::pricesPanColumnPriceWidth - st::pricesPanColumn24hWidth;

	int headerTop = getTableTop();
}

void RssWidget::updateMarketCap()
{
//	_marketCap->setRichText(textcmdLink(1, lang(lng_prices_market_cap)
//										.arg(BettergramSettings::instance()->cryptoPriceList()->marketCapString())));
//	_marketCap->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

//	_marketCap->moveToLeft((width() - _marketCap->width()) / 2,
//						   _siteName->y() + _siteName->height() + st::pricesPanPadding);
}

void RssWidget::onRssUpdated()
{
	updateMarketCap();
	update();
}

} // namespace ChatHelpers
