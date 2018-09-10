#include "prices_list_widget.h"
#include "table_column_header_widget.h"

#include "bettergram/bettergramservice.h"
#include "bettergram/cryptopricelist.h"
#include "bettergram/cryptoprice.h"

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

class PricesListWidget::Footer : public TabbedSelector::InnerFooter
{
public:
	Footer(not_null<PricesListWidget*> parent);

protected:
	void processPanelHideFinished() override;
	void resizeEvent(QResizeEvent* e) override;
	void onFooterClicked();

private:
	not_null<PricesListWidget*> _parent;
	object_ptr<Ui::LinkButton> _link;
};

PricesListWidget::Footer::Footer(not_null<PricesListWidget*> parent)
	:  InnerFooter(parent)
	, _parent(parent)
	, _link(object_ptr<Ui::LinkButton>(this, lang(lng_prices_footer), st::largeLinkButton))
{
	//TODO: bettergram: if a user is paid then we must to hide the "Upgrade Now" text

	_link->setClickedCallback([this] { onFooterClicked(); });
}

void PricesListWidget::Footer::resizeEvent(QResizeEvent* e)
{
	_link->move(rect().center() - _link->rect().center());
}

void PricesListWidget::Footer::processPanelHideFinished()
{
}

void PricesListWidget::Footer::onFooterClicked()
{
	BettergramService::openUrl(QUrl("https://www.livecoinwatch.com"));
}

PricesListWidget::PricesListWidget(QWidget* parent, not_null<Window::Controller*> controller)
	: Inner(parent, controller)
{
	_lastUpdateLabel = new Ui::FlatLabel(this, st::pricesPanLastUpdateLabel);

	_siteName = new Ui::IconButton(this, st::pricesPanSiteNameIcon);
	_siteName->setClickedCallback([] { BettergramService::openUrl(QUrl("https://www.livecoinwatch.com")); });

	_marketCap = new Ui::FlatLabel(this, st::pricesPanMarketCapLabel);
	_marketCap->setRichText(textcmdLink(1, lang(lng_prices_market_cap)
										.arg(BettergramService::instance()->cryptoPriceList()->marketCapString())));
	_marketCap->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

	_coinHeader = new TableColumnHeaderWidget(this);
	_coinHeader->setText(lng_prices_header_coin);
	_coinHeader->setTextFlags(Qt::AlignLeft | Qt::AlignVCenter);

	_priceHeader = new TableColumnHeaderWidget(this);
	_priceHeader->setText(lng_prices_header_price);
	_priceHeader->setTextFlags(Qt::AlignRight | Qt::AlignVCenter);

	_24hHeader = new TableColumnHeaderWidget(this);
	_24hHeader->setText(lng_prices_header_24h);
	_24hHeader->setTextFlags(Qt::AlignRight | Qt::AlignVCenter);

	connect(_coinHeader, &TableColumnHeaderWidget::sortOrderChanged,
			this, &PricesListWidget::onCoinColumnSortOrderChanged);

	connect(_priceHeader, &TableColumnHeaderWidget::sortOrderChanged,
			this, &PricesListWidget::onPriceColumnSortOrderChanged);

	connect(_24hHeader, &TableColumnHeaderWidget::sortOrderChanged,
			this, &PricesListWidget::on24hColumnSortOrderChanged);

	BettergramService::instance()->getCryptoPriceList();

	updateControlsGeometry();
	updateLastUpdateLabel();

	CryptoPriceList *priceList = BettergramService::instance()->cryptoPriceList();
	connect(priceList, &CryptoPriceList::updated, this, &PricesListWidget::onPriceListUpdated);

	setMouseTracking(true);
}

void PricesListWidget::refreshRecent()
{
}

void PricesListWidget::clearSelection()
{
}

object_ptr<TabbedSelector::InnerFooter> PricesListWidget::createFooter()
{
	Expects(_footer == nullptr);

	auto res = object_ptr<Footer>(this);

	_footer = res;
	return std::move(res);
}

void PricesListWidget::afterShown()
{
	startPriceListTimer();

	BettergramService::instance()->getCryptoPriceList();
}

void PricesListWidget::beforeHiding()
{
	stopPriceListTimer();
}

void PricesListWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _timerId) {
		BettergramService::instance()->getCryptoPriceList();
	}
}

void PricesListWidget::startPriceListTimer()
{
	if (!_timerId) {
		_timerId = startTimer(BettergramService::instance()->cryptoPriceList()->freq() * 1000);

		if (!_timerId) {
			LOG(("Can not start timer for %1 ms")
				.arg(BettergramService::instance()->cryptoPriceList()->freq()));
		}
	}
}

void PricesListWidget::stopPriceListTimer()
{
	if (_timerId) {
		killTimer(_timerId);
		_timerId = 0;
	}
}

void PricesListWidget::setSelectedRow(int selectedRow)
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

int PricesListWidget::getTableTop() const
{
	return _marketCap->y() + _marketCap->height() + st::pricesPanPadding;
}

int PricesListWidget::getTableBottom() const
{
	return getTableContentTop() + getTableContentHeight();
}

int PricesListWidget::getTableContentTop() const
{
	return getTableTop() + st::pricesPanTableHeaderHeight;
}

int PricesListWidget::getTableContentHeight() const
{
	return st::pricesPanTableRowHeight * BettergramService::instance()->cryptoPriceList()->count();
}

int PricesListWidget::getRowTop(int row) const
{
	return getTableContentTop() + row * st::pricesPanTableRowHeight;
}

QRect PricesListWidget::getTableRectangle() const
{
	return QRect(0,
				 getTableTop(),
				 width(),
				 getTableBottom());
}

QRect PricesListWidget::getTableHeaderRectangle() const
{
	return QRect(0,
				 getTableTop(),
				 width(),
				 st::pricesPanTableHeaderHeight);
}

QRect PricesListWidget::getTableContentRectangle() const
{
	return QRect(0,
				 getTableContentTop(),
				 width(),
				 getTableContentHeight());
}

QRect PricesListWidget::getRowRectangle(int row) const
{
	return QRect(0,
				 getRowTop(row),
				 width(),
				 st::pricesPanTableRowHeight);
}

void PricesListWidget::countSelectedRow(const QPoint &point)
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

	int rowCount = BettergramService::instance()->cryptoPriceList()->count();

	for (int row = 0; row < rowCount; row++) {
		if (getRowRectangle(row).contains(point)) {
			setSelectedRow(row);
			return;
		}
	}

	setSelectedRow(-1);
}

TabbedSelector::InnerFooter* PricesListWidget::getFooter() const
{
	return _footer;
}

int PricesListWidget::countDesiredHeight(int newWidth)
{
	Q_UNUSED(newWidth);

	return getTableBottom();
}

void PricesListWidget::mousePressEvent(QMouseEvent *e)
{
	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));

	_pressedRow = _selectedRow;
}

void PricesListWidget::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() != Qt::LeftButton) {
		return;
	}

	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));

	CryptoPriceList *priceList = BettergramService::instance()->cryptoPriceList();

	if (_pressedRow >= 0 && _pressedRow < priceList->count() && _pressedRow == _selectedRow) {
		QUrl url = priceList->at(_pressedRow)->url();
		if (!url.isEmpty()) {
			BettergramService::openUrl(url);
		}
	}
}

void PricesListWidget::mouseMoveEvent(QMouseEvent *e)
{
	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));
}

void PricesListWidget::enterEventHook(QEvent *e)
{
	QPoint point = mapFromGlobal(QCursor::pos());
	countSelectedRow(point);
}

void PricesListWidget::leaveEventHook(QEvent *e)
{
	Q_UNUSED(e);

	setSelectedRow(-1);
}

void PricesListWidget::paintEvent(QPaintEvent *event) {
	Painter painter(this);
	QRect r = event ? event->rect() : rect();

	if (r != rect()) {
		painter.setClipRect(r);
	}

	painter.fillRect(r, st::pricesPanBg);

	int top = getTableContentTop();

	int columnCoinWidth = _coinHeader->width()
			- _coinHeader->contentsMargins().left()
			- _coinHeader->contentsMargins().right()
			- st::pricesPanTableImageSize
			- st::pricesPanTablePadding;

	int columnPriceWidth = _priceHeader->width()
			- _priceHeader->contentsMargins().left()
			- _priceHeader->contentsMargins().right();

	int column24hWidth = _24hHeader->width()
			- _24hHeader->contentsMargins().left()
			- _24hHeader->contentsMargins().right();

	int columnCoinLeft = _coinHeader->x() + _coinHeader->contentsMargins().left();
	int columnPriceLeft = _priceHeader->x() + _priceHeader->contentsMargins().left();
	int column24hLeft = _24hHeader->x() + _24hHeader->contentsMargins().left();

	// Draw rows

	int columnCoinTextLeft = columnCoinLeft + st::pricesPanTableImageSize + st::pricesPanTablePadding;
	int rowCount = BettergramService::instance()->cryptoPriceList()->count();

	if (_selectedRow != -1 && _selectedRow < rowCount) {
		QRect rowRectangle(0, getRowTop(_selectedRow), width(), st::pricesPanTableRowHeight);
		App::roundRect(painter, rowRectangle, st::pricesPanHover, StickerHoverCorners);
	}

	painter.setFont(st::semiboldFont);

	for (const CryptoPrice *price : *BettergramService::instance()->cryptoPriceList()) {
		if (!price->icon().isNull()) {
			QRect targetRect(columnCoinLeft,
							 top + (st::pricesPanTableRowHeight - st::pricesPanTableImageSize) / 2,
							 st::pricesPanTableImageSize,
							 st::pricesPanTableImageSize);

			painter.drawPixmap(targetRect, price->icon());
		}

		painter.setPen(st::pricesPanTableCryptoNameFg);

		painter.drawText(columnCoinTextLeft, top, columnCoinWidth, st::pricesPanTableRowHeight / 2,
						 Qt::AlignLeft | Qt::AlignBottom, price->name());

		painter.setPen(st::pricesPanTableCryptoShortNameFg);

		painter.drawText(columnCoinTextLeft, top + st::pricesPanTableRowHeight / 2, columnCoinWidth, st::pricesPanTableRowHeight / 2,
						 Qt::AlignLeft | Qt::AlignTop, price->shortName());

		if (price->isCurrentPriceGrown()) {
			painter.setPen(st::pricesPanTableUpFg);
		} else {
			painter.setPen(st::pricesPanTableDownFg);
		}

		painter.drawText(columnPriceLeft, top, columnPriceWidth, st::pricesPanTableRowHeight,
						 Qt::AlignRight | Qt::AlignVCenter, price->currentPriceString());

		if (price->isChangeFor24HoursGrown()) {
			painter.setPen(st::pricesPanTableUpFg);
		} else {
			painter.setPen(st::pricesPanTableDownFg);
		}

		painter.drawText(column24hLeft, top, column24hWidth, st::pricesPanTableRowHeight,
						 Qt::AlignRight | Qt::AlignVCenter, price->changeFor24HoursString());

		top += st::pricesPanTableRowHeight;
	}
}

void PricesListWidget::resizeEvent(QResizeEvent *e)
{
	updateControlsGeometry();
}

void PricesListWidget::updateControlsGeometry()
{
	_lastUpdateLabel->moveToLeft(st::pricesPanPadding, st::pricesPanHeader);

	_siteName->moveToLeft((width() - _siteName->width()) / 2,
						  _lastUpdateLabel->y() + _lastUpdateLabel->height() + st::pricesPanPadding / 2);

	updateMarketCap();

	int columnCoinWidth = width() - st::pricesPanColumnPriceWidth - st::pricesPanColumn24hWidth;

	_coinHeader->resize(columnCoinWidth, st::pricesPanTableHeaderHeight);
	_coinHeader->setContentsMargins(st::pricesPanTablePadding, 0, st::pricesPanTablePadding, 0);

	_priceHeader->resize(st::pricesPanColumnPriceWidth, st::pricesPanTableHeaderHeight);
	_priceHeader->setContentsMargins(0, 0, st::pricesPanTablePadding, 0);

	_24hHeader->resize(st::pricesPanColumn24hWidth, st::pricesPanTableHeaderHeight);
	_24hHeader->setContentsMargins(0, 0, st::pricesPanTablePadding, 0);

	int headerTop = getTableTop();

	_coinHeader->moveToLeft(0, headerTop);
	_priceHeader->moveToLeft(_coinHeader->x() + _coinHeader->width(), headerTop);
	_24hHeader->moveToLeft(_priceHeader->x() + _priceHeader->width(), headerTop);
}

void PricesListWidget::updateLastUpdateLabel()
{
	_lastUpdateLabel->setText(lang(lng_prices_last_update)
							  .arg(BettergramService::instance()->cryptoPriceList()->lastUpdateString()));
}

void PricesListWidget::updateMarketCap()
{
	_marketCap->setRichText(textcmdLink(1, lang(lng_prices_market_cap)
										.arg(BettergramService::instance()->cryptoPriceList()->marketCapString())));
	_marketCap->setLink(1, std::make_shared<UrlClickHandler>(qsl("https://www.livecoinwatch.com")));

	_marketCap->moveToLeft((width() - _marketCap->width()) / 2,
						   _siteName->y() + _siteName->height() + st::pricesPanPadding);
}

void PricesListWidget::onCoinColumnSortOrderChanged()
{
	_priceHeader->resetSortOrder(false);
	_24hHeader->resetSortOrder(false);

	CryptoPriceList::SortOrder sortOrder = CryptoPriceList::SortOrder::Origin;

	switch (_coinHeader->sortOrder())
	{
	case (TableColumnHeaderWidget::SortOrder::None):
		sortOrder = CryptoPriceList::SortOrder::Origin;
		break;
	case (TableColumnHeaderWidget::SortOrder::Ascending):
		sortOrder = CryptoPriceList::SortOrder::NameAscending;
		break;
	case (TableColumnHeaderWidget::SortOrder::Descending):
		sortOrder = CryptoPriceList::SortOrder::NameDescending;
		break;
	default:
		sortOrder = CryptoPriceList::SortOrder::Origin;
		LOG(("Can not recognize sort order %1").arg(static_cast<int>(_coinHeader->sortOrder())));
		break;
	}

	BettergramService::instance()->cryptoPriceList()->setSortOrder(sortOrder);
}

void PricesListWidget::onPriceColumnSortOrderChanged()
{
	_coinHeader->resetSortOrder(false);
	_24hHeader->resetSortOrder(false);

	CryptoPriceList::SortOrder sortOrder = CryptoPriceList::SortOrder::Origin;

	switch (_priceHeader->sortOrder())
	{
	case (TableColumnHeaderWidget::SortOrder::None):
		sortOrder = CryptoPriceList::SortOrder::Origin;
		break;
	case (TableColumnHeaderWidget::SortOrder::Ascending):
		sortOrder = CryptoPriceList::SortOrder::PriceAscending;
		break;
	case (TableColumnHeaderWidget::SortOrder::Descending):
		sortOrder = CryptoPriceList::SortOrder::PriceDescending;
		break;
	default:
		sortOrder = CryptoPriceList::SortOrder::Origin;
		LOG(("Can not recognize sort order %1").arg(static_cast<int>(_priceHeader->sortOrder())));
		break;
	}

	BettergramService::instance()->cryptoPriceList()->setSortOrder(sortOrder);
}

void PricesListWidget::on24hColumnSortOrderChanged()
{
	_coinHeader->resetSortOrder(false);
	_priceHeader->resetSortOrder(false);

	CryptoPriceList::SortOrder sortOrder = CryptoPriceList::SortOrder::Origin;

	switch (_24hHeader->sortOrder())
	{
	case (TableColumnHeaderWidget::SortOrder::None):
		sortOrder = CryptoPriceList::SortOrder::Origin;
		break;
	case (TableColumnHeaderWidget::SortOrder::Ascending):
		sortOrder = CryptoPriceList::SortOrder::ChangeFor24hAscending;
		break;
	case (TableColumnHeaderWidget::SortOrder::Descending):
		sortOrder = CryptoPriceList::SortOrder::ChangeFor24hDescending;
		break;
	default:
		sortOrder = CryptoPriceList::SortOrder::Origin;
		LOG(("Can not recognize sort order %1").arg(static_cast<int>(_24hHeader->sortOrder())));
		break;
	}

	BettergramService::instance()->cryptoPriceList()->setSortOrder(sortOrder);
}

void PricesListWidget::onPriceListUpdated()
{
	updateLastUpdateLabel();
	updateMarketCap();
	update();
}

} // namespace ChatHelpers
