#pragma once

#include "tabbed_selector.h"
#include "list_row_array.h"

namespace Window {
class Controller;
} // namespace Window

namespace Ui {
class FlatLabel;
class IconButton;
} // namespace Ui

namespace Bettergram {
class RssItem;
class RssChannel;
} // namespace Bettergram

namespace ChatHelpers {

/**
 * @brief The RssWidget class shows RSS feeds.
 */
class RssWidget : public TabbedSelector::Inner
{
	Q_OBJECT

public:
	RssWidget(QWidget* parent, not_null<Window::Controller*> controller);

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

	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *e) override;
	void timerEvent(QTimerEvent *event) override;

private:
	class Footer;

	/**
	 * @brief Row class is used to group RSS news by RSS channels
	 */
	class Row
	{
	public:
		explicit Row(const QSharedPointer<Bettergram::RssItem> &item) : _item(item)
		{}

		explicit Row(const QSharedPointer<Bettergram::RssChannel> &channel) : _channel(channel)
		{}

		bool isItem() const
		{
			return !_item.isNull();
		}

		bool isChannel() const
		{
			return !_channel.isNull();
		}

		const QSharedPointer<Bettergram::RssItem> &item() const
		{
			return _item;
		}

		const QSharedPointer<Bettergram::RssChannel> &channel() const
		{
			return _channel;
		}

	private:
		QSharedPointer<Bettergram::RssItem> _item;
		QSharedPointer<Bettergram::RssChannel> _channel;
	};

	ListRowArray<Row> _rows;

	int _timerId = 0;
	int _selectedRow = -1;
	int _pressedRow = -1;
	bool _isSortBySite = false;

	Ui::FlatLabel *_lastUpdate = nullptr;
	Ui::FlatLabel *_sortMode = nullptr;
	Footer *_footer = nullptr;

	ClickHandlerPtr getSortModeClickHandler();

	void toggleIsSortBySite();
	void setIsSortBySite(bool isSortBySite);
	void setSelectedRow(int selectedRow);

	int getListAreaTop() const;

	void countSelectedRow(const QPoint &point);

	void updateControlsGeometry();
	void updateLastUpdateLabel();
	void updateSortModeLabel();

	void startRssTimer();
	void stopRssTimer();

	void fillRowsInSortByTimeMode();
	void fillRowsInSortBySiteMode();

	void updateRows();

private slots:
	void onLastUpdateChanged();
	void onIconChanged();
	void onRssUpdated();
};

} // namespace ChatHelpers
