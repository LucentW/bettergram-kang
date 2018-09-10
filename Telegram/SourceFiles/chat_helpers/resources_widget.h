#pragma once

#include "tabbed_selector.h"
#include "list_row_array.h"

namespace Window {
class Controller;
} // namespace Window

namespace Ui {
class FlatLabel;
class IconButton;
class PopupMenu;
} // namespace Ui

namespace Bettergram {
class ResourceItem;
class ResourceGroup;
} // namespace Bettergram

namespace ChatHelpers {

/**
 * @brief The ResourcesWidget class shows RSS feeds.
 */
class ResourcesWidget : public TabbedSelector::Inner
{
	Q_OBJECT

public:
	ResourcesWidget(QWidget* parent, not_null<Window::Controller*> controller);

	void refreshRecent() override;
	void clearSelection() override;
	object_ptr<TabbedSelector::InnerFooter> createFooter() override;

	void afterShown() override;
	void beforeHiding() override;

protected:
	TabbedSelector::InnerFooter *getFooter() const override;
	int countDesiredHeight(int newWidth) override;

	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void enterEventHook(QEvent *e) override;
	void leaveEventHook(QEvent *e) override;
	void contextMenuEvent(QContextMenuEvent *e) override;

	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *e) override;
	void timerEvent(QTimerEvent *event) override;

private:
	class Footer;

	/**
	 * @brief Row class is used to show resource items and groups
	 */
	class Row
	{
	public:
		explicit Row(const QSharedPointer<Bettergram::ResourceItem> &item) : _item(item)
		{}

		explicit Row(const QSharedPointer<Bettergram::ResourceGroup> &group) : _group(group)
		{}

		bool isItem() const
		{
			return !_item.isNull();
		}

		bool isGroup() const
		{
			return !_group.isNull();
		}

		const QSharedPointer<Bettergram::ResourceItem> &item() const
		{
			return _item;
		}

		const QSharedPointer<Bettergram::ResourceGroup> &group() const
		{
			return _group;
		}

	private:
		QSharedPointer<Bettergram::ResourceItem> _item;
		QSharedPointer<Bettergram::ResourceGroup> _group;
	};

	ListRowArray<Row> _rows;

	int _timerId = 0;
	int _selectedRow = -1;
	int _pressedRow = -1;

	Ui::FlatLabel *_lastUpdateLabel = nullptr;
	Footer *_footer = nullptr;
	base::unique_qptr<Ui::PopupMenu> _menu = nullptr;

	void setSelectedRow(int selectedRow);

	int getListAreaTop() const;

	void countSelectedRow(const QPoint &point);

	void updateControlsGeometry();
	void updateLastUpdateLabel();

	void startResourcesTimer();
	void stopResourcesTimer();

	void updateRows();

private slots:
	void onLastUpdateChanged();
	void onIconChanged();
	void onResourcesUpdated();
};

} // namespace ChatHelpers
