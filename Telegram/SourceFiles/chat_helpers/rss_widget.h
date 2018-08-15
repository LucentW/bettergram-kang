#pragma once
#include "chat_helpers/tabbed_selector.h"

namespace Window {
class Controller;
} // namespace Window

namespace Ui {
class FlatLabel;
class IconButton;
} // namespace Ui

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

public slots:

	//signals:

protected:
	TabbedSelector::InnerFooter* getFooter() const override;
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

	int _timerId = 0;
	int _selectedRow = -1;
	int _pressedRow = -1;

	Ui::IconButton *_siteName = nullptr;
	Ui::FlatLabel *_marketCap = nullptr;
	Footer *_footer = nullptr;

	void setSelectedRow(int selectedRow);

	int getTableTop() const;
	int getTableBottom() const;
	int getTableContentTop() const;
	int getTableContentHeight() const;
	int getRowTop(int row) const;

	QRect getTableRectangle() const;
	QRect getTableHeaderRectangle() const;
	QRect getTableContentRectangle() const;
	QRect getRowRectangle(int row) const;

	void countSelectedRow(const QPoint &point);

	void updateControlsGeometry();
	void updateMarketCap();

	void startRssTimer();
	void stopRssTimer();

private slots:
	void onRssUpdated();
};

} // namespace ChatHelpers
