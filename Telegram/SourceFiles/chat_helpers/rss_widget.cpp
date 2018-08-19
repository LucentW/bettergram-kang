#include "rss_widget.h"

#include "bettergram/bettergramsettings.h"
#include "bettergram/rsschannellist.h"
#include "bettergram/rsschannel.h"
#include "bettergram/rssitem.h"

#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/text/text_helper.h"
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
	, _link(object_ptr<Ui::LinkButton>(this, lang(lng_news_footer), st::largeLinkButton))
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
	QDesktopServices::openUrl(QUrl("https://bettergram.io"));
}

RssWidget::RssWidget(QWidget* parent, not_null<Window::Controller*> controller)
	: Inner(parent, controller)
{
	_lastUpdate = new Ui::FlatLabel(this, st::newsPanLastUpdateLabel);
	_sortMode = new Ui::FlatLabel(this, st::newsPanSortModeLabel);

	BettergramSettings::instance()->getRssChannelList();

	updateControlsGeometry();
	updateLastUpdateLabel();
	updateSortModeLabel();

	RssChannelList *rssChannelList = BettergramSettings::instance()->rssChannelList();

	connect(rssChannelList, &RssChannelList::lastUpdateChanged,
			this, &RssWidget::onLastUpdateChanged);

	connect(rssChannelList, &RssChannelList::iconChanged,
			this, &RssWidget::onIconChanged);

	connect(rssChannelList, &RssChannelList::updated,
			this, &RssWidget::onRssUpdated);

	setMouseTracking(true);
}

ClickHandlerPtr RssWidget::getSortModeClickHandler()
{
	return std::make_shared<LambdaClickHandler>([this] {
		toggleIsSortBySite();
	});
}

void RssWidget::toggleIsSortBySite()
{
	setIsSortBySite(!_isSortBySite);
}

void RssWidget::setIsSortBySite(bool isSortBySite)
{
	if (_isSortBySite != isSortBySite) {
		_isSortBySite = isSortBySite;

		updateSortModeLabel();
		updateRows();
	}
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

int RssWidget::getListAreaTop() const
{
	return _sortMode->y() + _sortMode->height() + st::pricesPanPadding;
}

void RssWidget::countSelectedRow(const QPoint &point)
{
	if (point.x() < 0 || point.x() >= width()) {
		setSelectedRow(-1);
		return;
	}

	if (_selectedRow == -1) {
		if (!_rows.contains(point.y())) {
			// Nothing changed
			return;
		}
	} else if (_rows.at(_selectedRow).contains(point.y())) {
		// Nothing changed
		return;
	}

	setSelectedRow(_rows.findRowIndex(point.y()));
}

TabbedSelector::InnerFooter* RssWidget::getFooter() const
{
	return _footer;
}

int RssWidget::countDesiredHeight(int newWidth)
{
	Q_UNUSED(newWidth);

	return _rows.bottom();
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

	if (_pressedRow >= 0 && _pressedRow < _rows.count() && _pressedRow == _selectedRow) {
		const Row &row = _rows.at(_pressedRow).userData();

		QUrl link;

		if (row.isItem()) {
			link = row.item()->link();

			//TODO: bettergram: mark the rss item as read and remove it from the list
			row.item()->markAsRead();
		} else if(row.isChannel()) {
			link = row.channel()->link();
		} else {
			LOG(("Unable to recognize row content"));
			return;
		}

		if (!link.isEmpty()) {
			QDesktopServices::openUrl(link);
		}
	}
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

	painter.fillRect(r, st::newsPanBg);

	const int iconLeft = st::newsPanPadding;
	const int iconSize = st::newsPanImageSize;

	const int textLeft = iconLeft + iconSize + st::newsPanPadding;
	const int textRight = width();
	const int textWidth = textRight - textLeft;

	// Draw rows

	for (int i = 0; i < _rows.count(); i++) {
		const ListRow<Row> &row = _rows.at(i);

		if (row.bottom() < r.top()) {
			continue;
		}

		if (row.top() > r.bottom()) {
			break;
		}

		if (i == _selectedRow) {
			QRect rowRectangle(0, row.top(), width(), row.height());
			App::roundRect(painter, rowRectangle, st::newsPanHover, StickerHoverCorners);
		}

		if (row.userData().isItem()) {
			QRect rowRect(textLeft,
						 row.top() + st::newsPanRowVerticalPadding,
						 textWidth,
						 row.height() - st::newsPanDateHeight - 2 * st::newsPanRowVerticalPadding);

			painter.setFont(st::semiboldFont);
			painter.setPen(st::newsPanNewsHeaderFg);

			int titleFlags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;
			const QString &title = row.userData().item()->title();

			QRect boundingRect = painter.boundingRect(rowRect, titleFlags, title);

			if (boundingRect.height() >= rowRect.height()) {
				boundingRect.setHeight(rowRect.height());
				TextHelper::drawElidedText(painter, rowRect, title);

				painter.setFont(st::normalFont);
				painter.setPen(st::newsPanNewsBodyFg);
			} else {
				painter.drawText(boundingRect, titleFlags, title, &boundingRect);

				painter.setFont(st::normalFont);
				painter.setPen(st::newsPanNewsBodyFg);

				QRect descriptionRect(textLeft, boundingRect.bottom(),
						 textWidth, rowRect.bottom() - boundingRect.bottom());

				TextHelper::drawElidedText(painter,
						 descriptionRect,
						 row.userData().item()->description());
			}

			painter.drawText(textLeft,
						 row.bottom() - st::newsPanDateHeight - st::newsPanRowVerticalPadding,
						 textWidth,
						 st::newsPanDateHeight,
						 Qt::AlignLeft | Qt::AlignBottom,
						 row.userData().item()->publishDateString());

			if (!row.userData().item()->icon().isNull()) {
				QRect targetRect(iconLeft,
								 rowRect.top() + (rowRect.height() - st::newsPanImageSize) / 2,
								 st::newsPanImageSize,
								 st::newsPanImageSize);

				painter.drawPixmap(targetRect, row.userData().item()->icon());
			}
		} else if (row.userData().isChannel()) {
			painter.setFont(st::semiboldFont);
			painter.setPen(st::newsPanSiteNameFg);

			painter.drawText(textLeft, row.top(), textWidth, row.height(),
						 Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
						 row.userData().channel()->title());

			if (!row.userData().channel()->icon().isNull()) {
				QRect targetRect(iconLeft,
								 row.top() + (row.height() - st::newsPanImageSize) / 2,
								 st::newsPanImageSize,
								 st::newsPanImageSize);

				painter.drawPixmap(targetRect, row.userData().channel()->icon());
			}
		} else {
			LOG(("Unable to recognize row content"));
			continue;
		}
	}
}

void RssWidget::resizeEvent(QResizeEvent *e)
{
	updateControlsGeometry();
}

void RssWidget::updateControlsGeometry()
{
	_lastUpdate->moveToLeft(st::newsPanPadding, st::newsPanHeader);
	_sortMode->moveToLeft(st::newsPanPadding,
							 _lastUpdate->y() + _lastUpdate->height() + st::newsPanPadding / 2);

	updateLastUpdateLabel();
	updateSortModeLabel();
}

void RssWidget::updateLastUpdateLabel()
{
	_lastUpdate->setText(lang(lng_news_last_update)
		.arg(BettergramSettings::instance()->rssChannelList()->lastUpdateString()));
}

void RssWidget::updateSortModeLabel()
{
	if (_isSortBySite) {
		_sortMode->setRichText(textcmdLink(1, lang(lng_news_sort_by_time)));
	} else {
		_sortMode->setRichText(textcmdLink(1, lang(lng_news_sort_by_site)));
	}

	_sortMode->setLink(1, getSortModeClickHandler());
}

void RssWidget::fillRowsInSortByTimeMode()
{
	RssChannelList *channelList = BettergramSettings::instance()->rssChannelList();
	QList<QSharedPointer<RssItem>> items = channelList->getAllItems();

	RssChannel::sort(items);

	// We use getAllItems() instead of getAllUnreadItems() because we just fetched them
	for (const QSharedPointer<RssItem> &item : items) {
		_rows.add(Row(item), st::newsPanRowHeight);
	}
}

void RssWidget::fillRowsInSortBySiteMode()
{
	RssChannelList *channelList = BettergramSettings::instance()->rssChannelList();

	for (const QSharedPointer<RssChannel> &channel : *channelList) {
		_rows.add(Row(channel), st::newsPanChannelRowHeight);

		// We use getAllItems() instead of getAllUnreadItems() because we just fetched them
		for (const QSharedPointer<RssItem> &item : channel->getAllItems()) {
			_rows.add(Row(item), st::newsPanRowHeight);
		}
	}
}

void RssWidget::updateRows()
{
	_rows.clear();
	_rows.setTop(getListAreaTop());

	if (_isSortBySite) {
		fillRowsInSortBySiteMode();
	} else {
		fillRowsInSortByTimeMode();
	}

	update();
}

void RssWidget::onLastUpdateChanged()
{
	updateLastUpdateLabel();
}

void RssWidget::onIconChanged()
{
	update();
}

void RssWidget::onRssUpdated()
{
	updateRows();
}

} // namespace ChatHelpers
