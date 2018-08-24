#include "resources_widget.h"

#include <bettergram/bettergramservice.h>
#include <bettergram/resourcegrouplist.h>
#include <bettergram/resourcegroup.h>
#include <bettergram/resourceitem.h>

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

class ResourcesWidget::Footer : public TabbedSelector::InnerFooter
{
public:
	Footer(not_null<ResourcesWidget*> parent);

protected:
	void processPanelHideFinished() override;
	void resizeEvent(QResizeEvent* e) override;
	void onFooterClicked();

private:
	not_null<ResourcesWidget*> _parent;
	object_ptr<Ui::LinkButton> _link;
};

ResourcesWidget::Footer::Footer(not_null<ResourcesWidget*> parent)
	:  InnerFooter(parent)
	, _parent(parent)
	, _link(object_ptr<Ui::LinkButton>(this, lang(lng_resources_footer), st::largeLinkButton))
{
	_link->setClickedCallback([this] { onFooterClicked(); });
}

void ResourcesWidget::Footer::resizeEvent(QResizeEvent* e)
{
	_link->move(rect().center() - _link->rect().center());
}

void ResourcesWidget::Footer::processPanelHideFinished()
{
}

void ResourcesWidget::Footer::onFooterClicked()
{
	QDesktopServices::openUrl(QUrl("https://bettergram.io"));
}

ResourcesWidget::ResourcesWidget(QWidget* parent, not_null<Window::Controller*> controller)
	: Inner(parent, controller)
{
	_lastUpdateLabel = new Ui::FlatLabel(this, st::resourcesPanLastUpdateLabel);

	BettergramService::instance()->getResourceGroupList();

	updateControlsGeometry();
	updateLastUpdateLabel();
	updateRows();

	ResourceGroupList *resourceGroupList = BettergramService::instance()->resourceGroupList();

	connect(resourceGroupList, &ResourceGroupList::lastUpdateChanged,
			this, &ResourcesWidget::onLastUpdateChanged);

	connect(resourceGroupList, &ResourceGroupList::iconChanged,
			this, &ResourcesWidget::onIconChanged);

	connect(resourceGroupList, &ResourceGroupList::updated,
			this, &ResourcesWidget::onResourcesUpdated);

	setMouseTracking(true);
}

void ResourcesWidget::refreshRecent()
{
}

void ResourcesWidget::clearSelection()
{
}

object_ptr<TabbedSelector::InnerFooter> ResourcesWidget::createFooter()
{
	Expects(_footer == nullptr);

	auto res = object_ptr<Footer>(this);

	_footer = res;
	return std::move(res);
}

void ResourcesWidget::afterShown()
{
	startResourcesTimer();

	BettergramService::instance()->getResourceGroupList();
}

void ResourcesWidget::beforeHiding()
{
	stopResourcesTimer();
}

void ResourcesWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _timerId) {
		BettergramService::instance()->getResourceGroupList();
	}
}

void ResourcesWidget::startResourcesTimer()
{
	if (!_timerId) {
		_timerId = startTimer(BettergramService::instance()->resourceGroupList()->freq() * 1000);

		if (!_timerId) {
			LOG(("Can not start timer for %1 ms")
				.arg(BettergramService::instance()->resourceGroupList()->freq()));
		}
	}
}

void ResourcesWidget::stopResourcesTimer()
{
	if (_timerId) {
		killTimer(_timerId);
		_timerId = 0;
	}
}

void ResourcesWidget::setSelectedRow(int selectedRow)
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

int ResourcesWidget::getListAreaTop() const
{
	return _lastUpdateLabel->y() + _lastUpdateLabel->height();
}

void ResourcesWidget::countSelectedRow(const QPoint &point)
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

TabbedSelector::InnerFooter* ResourcesWidget::getFooter() const
{
	return _footer;
}

int ResourcesWidget::countDesiredHeight(int newWidth)
{
	Q_UNUSED(newWidth);

	return _rows.bottom();
}

void ResourcesWidget::mousePressEvent(QMouseEvent *e)
{
	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));

	_pressedRow = _selectedRow;
}

void ResourcesWidget::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() != Qt::LeftButton) {
		return;
	}

	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));

	if (_pressedRow >= 0 && _pressedRow < _rows.count() && _pressedRow == _selectedRow) {
		const Row &row = _rows.at(_pressedRow).userData();

		if (row.isItem()) {
			QUrl link = row.item()->link();

			if (!link.isEmpty()) {
				QDesktopServices::openUrl(link);
			}
		}
	}
}

void ResourcesWidget::mouseMoveEvent(QMouseEvent *e)
{
	QPointF point = e->localPos();

	countSelectedRow(QPoint(static_cast<int>(qRound(point.x())),
							static_cast<int>(qRound(point.y()))));
}

void ResourcesWidget::enterEventHook(QEvent *e)
{
	QPoint point = mapFromGlobal(QCursor::pos());
	countSelectedRow(point);
}

void ResourcesWidget::leaveEventHook(QEvent *e)
{
	Q_UNUSED(e);

	setSelectedRow(-1);
}

void ResourcesWidget::contextMenuEvent(QContextMenuEvent *e)
{
	if (_selectedRow < 0 || _selectedRow >= _rows.count()) {
		return;
	}

	const ListRow<Row> &row = _rows.at(_selectedRow);

	if (!row.userData().isItem()) {
		return;
	}

	_menu = base::make_unique_q<Ui::PopupMenu>(nullptr);

	_menu->addAction(lang(lng_menu_resources_copy_link), [row] {
		Application::clipboard()->setText(row.userData().item()->link().toString());
	});

	_menu->addAction(lang(lng_menu_resources_copy_title), [row] {
		Application::clipboard()->setText(row.userData().item()->title());
	});

	connect(_menu.get(), &QObject::destroyed, [this] {
		leaveEventHook(nullptr);
	});

	_menu->popup(e->globalPos());
	e->accept();
}

void ResourcesWidget::paintEvent(QPaintEvent *event) {
	Painter painter(this);
	QRect r = event ? event->rect() : rect();

	if (r != rect()) {
		painter.setClipRect(r);
	}

	painter.fillRect(r, st::resourcesPanBg);

	const int iconLeft = st::resourcesPanPadding;
	const int iconSize = st::resourcesPanImageSize;

	const int textLeft = iconLeft + iconSize + st::resourcesPanPadding;
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

		if (row.userData().isItem()) {
			if (i == _selectedRow) {
				QRect rowRectangle(0, row.top(), width(), row.height());
				App::roundRect(painter, rowRectangle, st::resourcesPanHover, StickerHoverCorners);
			}

			QRect rowRect(textLeft,
						  row.top() + st::resourcesPanRowVerticalPadding,
						  textWidth,
						  row.height() - 2 * st::resourcesPanRowVerticalPadding);

			painter.setFont(st::semiboldFont);
			painter.setPen(st::resourcesPanItemTitleFg);

			int titleFlags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;
			const QString &title = row.userData().item()->title();

			QRect boundingRect = painter.boundingRect(rowRect, titleFlags, title);

			if (boundingRect.height() >= rowRect.height()) {
				boundingRect.setHeight(rowRect.height());
				TextHelper::drawElidedText(painter, rowRect, title);
			} else {
				painter.drawText(boundingRect, titleFlags, title, &boundingRect);

				painter.setFont(st::normalFont);
				painter.setPen(st::resourcesPanItemDescriptionFg);

				QRect descriptionRect(textLeft, boundingRect.bottom(),
									  textWidth, rowRect.bottom() - boundingRect.bottom());

				TextHelper::drawElidedText(painter,
										   descriptionRect,
										   row.userData().item()->description());
			}

			if (!row.userData().item()->icon().isNull()) {
				QRect targetRect(iconLeft,
								 rowRect.top() + (rowRect.height() - st::resourcesPanImageSize) / 2,
								 st::resourcesPanImageSize,
								 st::resourcesPanImageSize);

				painter.drawPixmap(targetRect, row.userData().item()->icon());
			}
		} else if (row.userData().isGroup()) {
			painter.setFont(st::semiboldFont);
			painter.setPen(st::resourcesPanGroupFg);

			painter.drawText(iconLeft, row.top(), width(), row.height(),
							 Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
							 row.userData().group()->title());
		} else {
			LOG(("Unable to recognize row content"));
			continue;
		}
	}
}

void ResourcesWidget::resizeEvent(QResizeEvent *e)
{
	updateControlsGeometry();
}

void ResourcesWidget::updateControlsGeometry()
{
	updateLastUpdateLabel();

	_lastUpdateLabel->moveToLeft(st::resourcesPanPadding, st::resourcesPanHeader);
}

void ResourcesWidget::updateLastUpdateLabel()
{
	_lastUpdateLabel->setText(lang(lng_resources_last_update)
							  .arg(BettergramService::instance()->resourceGroupList()->lastUpdateString()));
}

void ResourcesWidget::updateRows()
{
	_rows.clear();
	_rows.setTop(getListAreaTop());

	ResourceGroupList *groupList = BettergramService::instance()->resourceGroupList();

	for (const QSharedPointer<ResourceGroup> &group : *groupList) {
		if (group->isEmpty()) {
			continue;
		}

		_rows.add(Row(group), st::resourcesPanGroupRowHeight);

		for (const QSharedPointer<ResourceItem> &item : group->items()) {
			_rows.add(Row(item), st::resourcesPanRowHeight);
		}
	}

	update();
}

void ResourcesWidget::onLastUpdateChanged()
{
	updateLastUpdateLabel();
}

void ResourcesWidget::onIconChanged()
{
	update();
}

void ResourcesWidget::onResourcesUpdated()
{
	updateRows();
}

} // namespace ChatHelpers
