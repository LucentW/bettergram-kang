/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/

// This file is copied from tabbed_panel.h.
// We have to do this instead of just using the TabbedPanel class
// because that class with TabbedSelector were not created as reusable things
// and it is safer to just copy/paste this instead of
// trying to adapt the existing classes

#pragma once

#include "ui/rp_widget.h"
#include "base/timer.h"

namespace Window {
class Controller;
} // namespace Window

namespace Ui {
class PanelAnimation;
} // namespace Ui

namespace ChatHelpers {

class BettergramTabbedSelector;

class BettergramTabbedPanel : public Ui::RpWidget{
	Q_OBJECT

public:
	BettergramTabbedPanel(QWidget *parent, not_null<Window::Controller*> controller);
	BettergramTabbedPanel(QWidget *parent, not_null<Window::Controller*> controller, object_ptr<BettergramTabbedSelector> selector);

	object_ptr<BettergramTabbedSelector> takeSelector();
	QPointer<BettergramTabbedSelector> getSelector() const;
	void moveBottom(int bottom);

	void hideFast();
	bool hiding() const {
		return _hiding || _hideTimer.isActive();
	}

	bool overlaps(const QRect &globalRect) const;

	void showAnimated();
	void hideAnimated();
	void toggleAnimated();

	~BettergramTabbedPanel();

protected:
	void enterEventHook(QEvent *e) override;
	void leaveEventHook(QEvent *e) override;
	void otherEnter();
	void otherLeave();

	void paintEvent(QPaintEvent *e) override;
	bool eventFilter(QObject *obj, QEvent *e) override;

private slots:
	void onWndActiveChanged();

private:
	void hideByTimerOrLeave();
	void moveByBottom();
	bool isDestroying() const {
		return !_selector;
	}
	void showFromSelector();

	style::margins innerPadding() const;

	// Rounded rect which has shadow around it.
	QRect innerRect() const;

	// Inner rect with removed st::buttonRadius from top and bottom.
	// This one is allowed to be not rounded.
	QRect horizontalRect() const;

	// Inner rect with removed st::buttonRadius from left and right.
	// This one is allowed to be not rounded.
	QRect verticalRect() const;

	QImage grabForAnimation();
	void startShowAnimation();
	void startOpacityAnimation(bool hiding);
	void prepareCache();

	void opacityAnimationCallback();

	void hideFinished();
	void showStarted();

	bool preventAutoHide() const;
	void updateContentHeight();

	not_null<Window::Controller*> _controller;
	object_ptr<BettergramTabbedSelector> _selector;

	int _contentMaxHeight = 0;
	int _contentHeight = 0;
	int _bottom = 0;

	std::unique_ptr<Ui::PanelAnimation> _showAnimation;
	Animation _a_show;

	bool _hiding = false;
	bool _hideAfterSlide = false;
	QPixmap _cache;
	Animation _a_opacity;
	base::Timer _hideTimer;
};

} // namespace ChatHelpers
