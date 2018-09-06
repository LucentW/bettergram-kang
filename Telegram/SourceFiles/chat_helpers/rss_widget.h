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
class RssItem;
class RssChannel;
class RssChannelList;
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
	RssWidget(QWidget* parent,
			  not_null<Window::Controller*> controller,
			  Bettergram::RssChannelList *rssChannelList,
			  const style::color &rowReadFg,
			  const style::color &rowBodyFg,
			  const style::color &rowHeaderFg,
			  const style::color &siteNameFg,
			  const style::color &bg,
			  const style::color &hover,
			  int padding,
			  int headerPadding,
			  int imageWidth,
			  int imageHeight,
			  int rowVerticalPadding,
			  int rowHeight,
			  int channelRowHeight,
			  int dateTimeHeight,
			  bool isShowDescriptions);

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

	Bettergram::RssChannelList *const _rssChannelList;

	const style::color _rowReadFg;
	const style::color _rowBodyFg;
	const style::color _rowHeaderFg;
	const style::color _siteNameFg;
	const style::color _bg;
	const style::color _hover;

	const int _padding;
	const int _headerPadding;
	const int _imageWidth;
	const int _imageHeight;
	const int _rowVerticalPadding;
	const int _rowHeight;
	const int _channelRowHeight;
	const int _dateTimeHeight;

	const bool _isShowDescriptions;

	ListRowArray<Row> _rows;

	int _timerId = 0;
	int _selectedRow = -1;
	int _pressedRow = -1;
	bool _isSortBySite = false;
	bool _isShowRead = true;

	Ui::FlatLabel *_lastUpdateLabel = nullptr;
	Ui::FlatLabel *_sortModeLabel = nullptr;
	Ui::FlatLabel *_isShowReadLabel = nullptr;
	Footer *_footer = nullptr;
	base::unique_qptr<Ui::PopupMenu> _menu = nullptr;

	const style::color &getNewsHeaderColor(const QSharedPointer<Bettergram::RssItem> &item) const;
	const style::color &getNewsBodyColor(const QSharedPointer<Bettergram::RssItem> &item) const;

	ClickHandlerPtr getSortModeClickHandler();
	ClickHandlerPtr getIsShowReadClickHandler();

	void toggleIsSortBySite();
	void setIsSortBySite(bool isSortBySite);

	void toggleIsShowRead();
	void setIsShowRead(bool isShowRead);

	void setSelectedRow(int selectedRow);

	int getListAreaTop() const;

	void countSelectedRow(const QPoint &point);

	void updateControlsGeometry();
	void updateLastUpdateLabel();
	void updateSortModeLabel();
	void updateIsShowReadLabel();

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
