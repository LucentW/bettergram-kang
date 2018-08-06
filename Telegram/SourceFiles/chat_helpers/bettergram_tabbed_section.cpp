/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "chat_helpers/bettergram_tabbed_section.h"

#include "styles/style_chat_helpers.h"
#include "chat_helpers/bettergram_tabbed_selector.h"

namespace ChatHelpers {

BettergramTabbedMemento::BettergramTabbedMemento(
	object_ptr<BettergramTabbedSelector> selector,
	Fn<void(object_ptr<BettergramTabbedSelector>)> returnMethod)
: _selector(std::move(selector))
, _returnMethod(std::move(returnMethod)) {
}

object_ptr<Window::SectionWidget> BettergramTabbedMemento::createWidget(
		QWidget *parent,
		not_null<Window::Controller*> controller,
		Window::Column column,
		const QRect &geometry) {
	auto result = object_ptr<BettergramTabbedSection>(
		parent,
		controller,
		std::move(_selector),
		std::move(_returnMethod));
	result->setGeometry(geometry);
	return std::move(result);
}

BettergramTabbedMemento::~BettergramTabbedMemento() {
	if (_returnMethod && _selector) {
		_returnMethod(std::move(_selector));
	}
}

BettergramTabbedSection::BettergramTabbedSection(
	QWidget *parent,
	not_null<Window::Controller*> controller)
: BettergramTabbedSection(
	parent,
	controller,
	object_ptr<BettergramTabbedSelector>(this, controller),
	Fn<void(object_ptr<BettergramTabbedSelector>)>()) {
}

BettergramTabbedSection::BettergramTabbedSection(
	QWidget *parent,
	not_null<Window::Controller*> controller,
	object_ptr<BettergramTabbedSelector> selector,
	Fn<void(object_ptr<BettergramTabbedSelector>)> returnMethod)
: Window::SectionWidget(parent, controller)
, _selector(std::move(selector))
, _returnMethod(std::move(returnMethod)) {
	_selector->setParent(this);
	_selector->setRoundRadius(0);
	_selector->setGeometry(rect());
	_selector->showStarted();
	_selector->show();
	connect(_selector, &BettergramTabbedSelector::cancelled, this, [this] {
		if (_cancelledCallback) {
			_cancelledCallback();
		}
	});
	_selector->setAfterShownCallback(Fn<void(BettergramSelectorTab)>());
	_selector->setBeforeHidingCallback(Fn<void(BettergramSelectorTab)>());

	setAttribute(Qt::WA_OpaquePaintEvent, true);
}

void BettergramTabbedSection::beforeHiding() {
	_selector->beforeHiding();
}

void BettergramTabbedSection::afterShown() {
	_selector->afterShown();
}

void BettergramTabbedSection::resizeEvent(QResizeEvent *e) {
	_selector->setGeometry(rect());
}

object_ptr<BettergramTabbedSelector> BettergramTabbedSection::takeSelector() {
	_selector->beforeHiding();
	return std::move(_selector);
}

QPointer<BettergramTabbedSelector> BettergramTabbedSection::getSelector() const {
	return _selector.data();
}
bool BettergramTabbedSection::showInternal(
		not_null<Window::SectionMemento*> memento,
		const Window::SectionShow &params) {
	return false;
}

bool BettergramTabbedSection::wheelEventFromFloatPlayer(QEvent *e) {
	return _selector->wheelEventFromFloatPlayer(e);
}

QRect BettergramTabbedSection::rectForFloatPlayer() const {
	return _selector->rectForFloatPlayer();
}

BettergramTabbedSection::~BettergramTabbedSection() {
	beforeHiding();
	if (_returnMethod) {
		_returnMethod(takeSelector());
	}
}

} // namespace ChatHelpers
