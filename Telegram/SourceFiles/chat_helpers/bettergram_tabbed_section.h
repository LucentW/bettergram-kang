/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/

// This file is copied from tabbed_section.h.
// We have to do this instead of just using the TabbedSection class
// because that class with TabbedSelector were not created as reusable things
// and it is safer to just copy/paste this instead of
// trying to adapt the existing classes

#pragma once

#include "window/section_widget.h"
#include "window/section_memento.h"

namespace ChatHelpers {

class BettergramTabbedSelector;

class BettergramTabbedMemento : public Window::SectionMemento {
public:
	BettergramTabbedMemento(
		object_ptr<BettergramTabbedSelector> selector,
		Fn<void(object_ptr<BettergramTabbedSelector>)> returnMethod);
	BettergramTabbedMemento(BettergramTabbedMemento &&other) = default;
	BettergramTabbedMemento &operator=(BettergramTabbedMemento &&other) = default;

	object_ptr<Window::SectionWidget> createWidget(
		QWidget *parent,
		not_null<Window::Controller*> controller,
		Window::Column column,
		const QRect &geometry) override;

	~BettergramTabbedMemento();

private:
	object_ptr<BettergramTabbedSelector> _selector;
	Fn<void(object_ptr<BettergramTabbedSelector>)> _returnMethod;
};

class BettergramTabbedSection : public Window::SectionWidget {
public:
	BettergramTabbedSection(
		QWidget *parent,
		not_null<Window::Controller*> controller);
	BettergramTabbedSection(
		QWidget *parent,
		not_null<Window::Controller*> controller,
		object_ptr<BettergramTabbedSelector> selector,
		Fn<void(object_ptr<BettergramTabbedSelector>)> returnMethod);

	void beforeHiding();
	void afterShown();
	void setCancelledCallback(Fn<void()> callback) {
		_cancelledCallback = std::move(callback);
	}

	object_ptr<BettergramTabbedSelector> takeSelector();
	QPointer<BettergramTabbedSelector> getSelector() const;

	bool showInternal(
		not_null<Window::SectionMemento*> memento,
		const Window::SectionShow &params) override;
	bool forceAnimateBack() const override {
		return true;
	}
	// Float player interface.
	bool wheelEventFromFloatPlayer(QEvent *e) override;
	QRect rectForFloatPlayer() const override;

	~BettergramTabbedSection();

protected:
	void resizeEvent(QResizeEvent *e) override;

	void showFinishedHook() override {
		afterShown();
	}

private:
	object_ptr<BettergramTabbedSelector> _selector;
	Fn<void()> _cancelledCallback;
	Fn<void(object_ptr<BettergramTabbedSelector>)> _returnMethod;
};

} // namespace ChatHelpers
