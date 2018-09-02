#include "rss_widget.h"

#include <bettergram/bettergramservice.h>
#include <bettergram/rsschannellist.h>
#include <bettergram/rsschannel.h>
#include <bettergram/rssitem.h>

#include <application.h>
#include <ui/widgets/buttons.h>
#include <ui/widgets/labels.h>
#include <ui/widgets/popup_menu.h>
#include <ui/text/text_helper.h>
#include <lang/lang_keys.h>
#include <styles/style_window.h>
#include <core/click_handler_types.h>
#include <styles/style_chat_helpers.h>
#include <styles/style_widgets.h>

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
	BettergramService::openUrl(QUrl("https://bettergram.io"));
}

const style::color &RssWidget::getNewsHeaderColor(const QSharedPointer<RssItem> &item)
{
	if (item->isRead()) {
		return st::newsPanNewsReadFg;
	} else {
		return st::newsPanNewsHeaderFg;
	}
}

const style::color &RssWidget::getNewsBodyColor(const QSharedPointer<RssItem> &item)
{
	if (item->isRead()) {
		return st::newsPanNewsReadFg;
	} else {
		return st::newsPanNewsBodyFg;
	}
}

RssWidget::RssWidget(QWidget* parent, not_null<Window::Controller*> controller)
	: Inner(parent, controller)
{
	_lastUpdateLabel = new Ui::FlatLabel(this, st::newsPanLastUpdateLabel);
	_sortModeLabel = new Ui::FlatLabel(this, st::newsPanSortModeLabel);
	_isShowReadLabel = new Ui::FlatLabel(this, st::newsPanIsShowReadLabel);

	BettergramService::instance()->getRssChannelList();

	updateControlsGeometry();
	updateLastUpdateLabel();
	updateSortModeLabel();
	updateIsShowReadLabel();
	updateRows();

	RssChannelList *rssChannelList = BettergramService::instance()->rssChannelList();

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

ClickHandlerPtr RssWidget::getIsShowReadClickHandler()
{
	return std::make_shared<LambdaClickHandler>([this] {
		toggleIsShowRead();
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
		resizeToWidth(width());
	}
}

void RssWidget::toggleIsShowRead()
{
	setIsShowRead(!_isShowRead);
}

void RssWidget::setIsShowRead(bool isShowRead)
{
	if (_isShowRead != isShowRead) {
		_isShowRead = isShowRead;

		updateIsShowReadLabel();
		_isShowReadLabel->moveToRight(st::newsPanPadding, _sortModeLabel->y());

		updateRows();
		resizeToWidth(width());
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

	BettergramService::instance()->getRssChannelList();
}

void RssWidget::beforeHiding()
{
	stopRssTimer();
}

void RssWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _timerId) {
		BettergramService::instance()->getRssChannelList();
	}
}

void RssWidget::startRssTimer()
{
	if (!_timerId) {
		_timerId = startTimer(BettergramService::instance()->rssChannelList()->freq() * 1000);

		if (!_timerId) {
			LOG(("Can not start timer for %1 ms")
				.arg(BettergramService::instance()->rssChannelList()->freq()));
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
	return _sortModeLabel->y() + _sortModeLabel->height() + st::newsPanPadding;
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
	if (e->button() != Qt::LeftButton) {
		return;
	}

	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));

	if (_pressedRow >= 0 && _pressedRow < _rows.count() && _pressedRow == _selectedRow) {
		const Row &row = _rows.at(_pressedRow).userData();

		QUrl link;

		if (row.isItem()) {
			link = row.item()->link();
			row.item()->markAsRead();

			if (!_isShowRead) {
				updateRows();
			}
		} else if(row.isChannel()) {
			link = row.channel()->link();
		} else {
			LOG(("Unable to recognize row content"));
			return;
		}

		if (!link.isEmpty()) {
			BettergramService::openUrl(link);
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

void RssWidget::contextMenuEvent(QContextMenuEvent *e)
{
	if (_selectedRow < 0 || _selectedRow >= _rows.count()) {
		return;
	}

	const ListRow<Row> &row = _rows.at(_selectedRow);

	_menu = base::make_unique_q<Ui::PopupMenu>(nullptr);

	_menu->addAction(lang(lng_menu_news_copy_link), [row] {
		QString link;

		if (row.userData().isItem()) {
			link = row.userData().item()->link().toString();
		} else if (row.userData().isChannel()) {
			link = row.userData().channel()->link().toString();
		} else {
			LOG(("Unable to recognize type of row content"));
			return;
		}

		Application::clipboard()->setText(link);
	});

	_menu->addAction(lang(lng_menu_news_copy_title), [row] {
		QString title;

		if (row.userData().isItem()) {
			title = row.userData().item()->title();
		} else if (row.userData().isChannel()) {
			title = row.userData().channel()->title();
		} else {
			LOG(("Unable to recognize type of row content"));
			return;
		}

		Application::clipboard()->setText(title);
	});

	if (row.userData().isItem()) {
		_menu->addAction(lang(lng_menu_news_mark_as_read), [row] {
			if (row.userData().isItem()) {
				row.userData().item()->markAsRead();
			} else {
				LOG(("Unable to recognize type of row content"));
			}
		});
	}

	_menu->addAction(lang(lng_menu_news_mark_all_site_news_as_read), [row] {
		if (row.userData().isItem()) {
			row.userData().item()->markAllNewsAtSiteAsRead();
		} else if (row.userData().isChannel()) {
			row.userData().channel()->markAsRead();
		} else {
			LOG(("Unable to recognize type of row content"));
		}
	});

	_menu->addAction(lang(lng_menu_news_mark_all_news_as_read), [] {
		BettergramService::instance()->rssChannelList()->markAsRead();
	});

	connect(_menu.get(), &QObject::destroyed, [this] {
		leaveEventHook(nullptr);
	});

	_menu->popup(e->globalPos());
	e->accept();
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

	const int textLeft = iconLeft + iconSize + st::newsPanPadding / 2;
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
			painter.setPen(getNewsHeaderColor(row.userData().item()));

			int titleFlags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;
			const QString &title = row.userData().item()->title();

			QRect boundingRect = painter.boundingRect(rowRect, titleFlags, title);

			// We decided do not show description text, only title
#if 1
			boundingRect.setHeight(rowRect.height());
			TextHelper::drawElidedText(painter, rowRect, title);

			painter.setFont(st::normalFont);
			painter.setPen(getNewsBodyColor(row.userData().item()));
#else
			if (boundingRect.height() >= rowRect.height()) {
				boundingRect.setHeight(rowRect.height());
				TextHelper::drawElidedText(painter, rowRect, title);

				painter.setFont(st::normalFont);
				painter.setPen(getNewsBodyColor(row.userData().item()));
			} else {
				painter.drawText(boundingRect, titleFlags, title, &boundingRect);

				painter.setFont(st::normalFont);
				painter.setPen(getNewsBodyColor(row.userData().item()));

				QRect descriptionRect(textLeft, boundingRect.bottom(),
									  textWidth, rowRect.bottom() - boundingRect.bottom());

				TextHelper::drawElidedText(painter,
										   descriptionRect,
										   row.userData().item()->description());
			}
#endif

			painter.drawText(textLeft,
							 row.bottom() - st::newsPanDateHeight - st::newsPanRowVerticalPadding,
							 textWidth,
							 st::newsPanDateHeight,
							 Qt::AlignLeft | Qt::AlignBottom,
							 row.userData().item()->publishDateString());

			const QPixmap &image = row.userData().item()->image();

			if (!image.isNull()) {
				QRect targetRect(iconLeft,
								 row.top() + (row.height() - st::newsPanImageSize) / 2,
								 st::newsPanImageSize,
								 st::newsPanImageSize);

				QRect sourceRect(image.width() > st::newsPanImageSize ? (image.width() - st::newsPanImageSize) / 2 : 0,
								 image.height() > st::newsPanImageSize ? (image.height() - st::newsPanImageSize) / 2 : 0,
								 st::newsPanImageSize,
								 st::newsPanImageSize);

				painter.drawPixmap(targetRect, image, sourceRect);
			}
		} else if (row.userData().isChannel()) {
			painter.setFont(st::semiboldFont);
			painter.setPen(st::newsPanSiteNameFg);

			painter.drawText(textLeft, row.top(), textWidth, row.height(),
							 Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
							 row.userData().channel()->title());

			const QPixmap &image = row.userData().channel()->icon();

			if (!image.isNull()) {
				QRect targetRect(iconLeft,
								 row.top() + (row.height() - st::newsPanImageSize) / 2,
								 st::newsPanImageSize,
								 st::newsPanImageSize);

				QRect sourceRect(image.width() > st::newsPanImageSize ? (image.width() - st::newsPanImageSize) / 2 : 0,
								 image.height() > st::newsPanImageSize ? (image.height() - st::newsPanImageSize) / 2 : 0,
								 st::newsPanImageSize,
								 st::newsPanImageSize);

				painter.drawPixmap(targetRect, image, sourceRect);
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
	updateLastUpdateLabel();
	updateSortModeLabel();
	updateIsShowReadLabel();

	_lastUpdateLabel->moveToLeft(st::newsPanPadding, st::newsPanHeader);
	_sortModeLabel->moveToLeft(st::newsPanPadding,
							   _lastUpdateLabel->y() + _lastUpdateLabel->height() + st::newsPanPadding / 2);

	_isShowReadLabel->moveToRight(st::newsPanPadding, _sortModeLabel->y());
}

void RssWidget::updateLastUpdateLabel()
{
	_lastUpdateLabel->setText(lang(lng_news_last_update)
							  .arg(BettergramService::instance()->rssChannelList()->lastUpdateString()));
}

void RssWidget::updateSortModeLabel()
{
	if (_isSortBySite) {
		_sortModeLabel->setRichText(textcmdLink(1, lang(lng_news_sort_by_time)));
	} else {
		_sortModeLabel->setRichText(textcmdLink(1, lang(lng_news_sort_by_site)));
	}

	_sortModeLabel->setLink(1, getSortModeClickHandler());
}

void RssWidget::updateIsShowReadLabel()
{
	if (_isShowRead) {
		_isShowReadLabel->setRichText(textcmdLink(1, lang(lng_news_show_only_unread_news)));
	} else {
		_isShowReadLabel->setRichText(textcmdLink(1, lang(lng_news_show_all_news)));
	}

	_isShowReadLabel->setLink(1, getIsShowReadClickHandler());
}

void RssWidget::fillRowsInSortByTimeMode()
{
	RssChannelList *channelList = BettergramService::instance()->rssChannelList();
	QList<QSharedPointer<RssItem>> items;

	if (_isShowRead) {
		items = channelList->getAllItems();
	} else {
		items = channelList->getAllUnreadItems();
	}

	RssChannel::sort(items);

	for (const QSharedPointer<RssItem> &item : items) {
		_rows.add(Row(item), st::newsPanRowHeight);
	}
}

void RssWidget::fillRowsInSortBySiteMode()
{
	RssChannelList *channelList = BettergramService::instance()->rssChannelList();

	for (const QSharedPointer<RssChannel> &channel : *channelList) {
		_rows.add(Row(channel), st::newsPanChannelRowHeight);

		QList<QSharedPointer<RssItem>> items;

		if (_isShowRead) {
			items = channel->getAllItems();
		} else {
			items = channel->getAllUnreadItems();
		}

		for (const QSharedPointer<RssItem> &item : items) {
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
