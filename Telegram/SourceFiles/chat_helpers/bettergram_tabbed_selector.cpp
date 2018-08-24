/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/

#include "bettergram_tabbed_selector.h"

#include "prices_list_widget.h"
#include "rss_widget.h"
#include "resources_widget.h"
#include "styles/style_chat_helpers.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/shadow.h"
#include "ui/widgets/discrete_sliders.h"
#include "ui/widgets/scroll_area.h"
#include "storage/localstorage.h"
#include "lang/lang_keys.h"
#include "mainwindow.h"
#include "observer_peer.h"
#include "apiwrap.h"

namespace ChatHelpers {

BettergramTabbedSelector::Tab::Tab(BettergramSelectorTab type, object_ptr<TabbedSelector::Inner> widget)
	: _type(type)
	, _widget(std::move(widget))
	, _weak(_widget)
	, _footer(_widget->createFooter()) {
	_footer->setParent(_widget->parentWidget());
}

object_ptr<TabbedSelector::Inner> BettergramTabbedSelector::Tab::takeWidget() {
	return std::move(_widget);
}

void BettergramTabbedSelector::Tab::returnWidget(object_ptr<TabbedSelector::Inner> widget) {
	_widget = std::move(widget);
	Ensures(_widget == _weak);
}

void BettergramTabbedSelector::Tab::saveScrollTop() {
	_scrollTop = widget()->getVisibleTop();
}

BettergramTabbedSelector::BettergramTabbedSelector(QWidget *parent, not_null<Window::Controller*> controller) : RpWidget(parent)
	, _tabsSlider(this, st::emojiTabs)
	, _topShadow(this)
	, _bottomShadow(this)
	, _scroll(this, st::emojiScroll)
	, _tabs{ {
			Tab{ BettergramSelectorTab::Prices, object_ptr<PricesListWidget>(this, controller) },
			Tab{ BettergramSelectorTab::News, object_ptr<RssWidget>(this, controller) },
			//Tab{ BettergramSelectorTab::Icos, object_ptr<PricesListWidget>(this, controller) },
			Tab{ BettergramSelectorTab::Resources, object_ptr<ResourcesWidget>(this, controller) },
		} }
		, _currentTabType(Auth().settings().bettergramSelectorTab()) {
	resize(st::emojiPanWidth, st::emojiPanMaxHeight);

	for (auto &tab : _tabs) {
		tab.footer()->hide();
		tab.widget()->hide();
	}

	createTabsSlider();
	setWidgetToScrollArea();

	_bottomShadow->setGeometry(_tabsSlider->x(), _scroll->y() + _scroll->height() - st::lineWidth, _tabsSlider->width(), st::lineWidth);

	for (auto &tab : _tabs) {
		auto widget = tab.widget();
		connect(widget, &TabbedSelector::Inner::scrollToY, this, [this, tab = &tab](int y) {
			if (tab == currentTab()) {
				scrollToY(y);
			}
			else {
				tab->saveScrollTop(y);
			}
		});
		connect(widget, &TabbedSelector::Inner::disableScroll, this, [this, tab = &tab](bool disabled) {
			if (tab == currentTab()) {
				_scroll->disableScroll(disabled);
			}
		});
	}

	connect(_scroll, &Ui::ScrollArea::scrolled, this, &BettergramTabbedSelector::onScroll);

	_topShadow->raise();
	_bottomShadow->raise();
	_tabsSlider->raise();

	//	setAttribute(Qt::WA_AcceptTouchEvents);
	setAttribute(Qt::WA_OpaquePaintEvent, false);
	showAll();
}

void BettergramTabbedSelector::resizeEvent(QResizeEvent *e) {
	_tabsSlider->resizeToWidth(width());
	_tabsSlider->moveToLeft(0, 0);
	_topShadow->setGeometry(
		_tabsSlider->x(),
		_tabsSlider->bottomNoMargins() - st::lineWidth,
		_tabsSlider->width(),
		st::lineWidth);

	auto scrollWidth = width() - st::buttonRadius;
	auto scrollHeight = height() - marginTop() - marginBottom();
	auto inner = currentTab()->widget();
	auto innerWidth = scrollWidth - st::emojiScroll.width;
	auto updateScrollGeometry = [&] {
		_scroll->setGeometryToLeft(
			st::buttonRadius,
			marginTop(),
			scrollWidth,
			scrollHeight);
	};
	auto updateInnerGeometry = [&] {
		auto scrollTop = _scroll->scrollTop();
		auto scrollBottom = scrollTop + scrollHeight;
		inner->setMinimalHeight(innerWidth, scrollHeight);
		inner->setVisibleTopBottom(scrollTop, scrollBottom);
	};
	if (e->oldSize().height() > height()) {
		updateScrollGeometry();
		updateInnerGeometry();
	}
	else {
		updateInnerGeometry();
		updateScrollGeometry();
	}
	_bottomShadow->setGeometry(_tabsSlider->x(), _scroll->y() + _scroll->height() - st::lineWidth, _tabsSlider->width(), st::lineWidth);
	updateRestrictedLabelGeometry();

	_footerTop = height() - st::emojiFooterHeight;
	for (auto &tab : _tabs) {
		tab.footer()->resizeToWidth(width());
		tab.footer()->moveToLeft(0, _footerTop);
	}

	update();
}

void BettergramTabbedSelector::updateRestrictedLabelGeometry() {
	if (!_restrictedLabel) {
		return;
	}

	auto labelWidth = width() - st::stickerPanPadding * 2;
	_restrictedLabel->resizeToWidth(labelWidth);
	_restrictedLabel->moveToLeft(
		(width() - _restrictedLabel->width()) / 2,
		(height() / 3 - _restrictedLabel->height() / 2));
}

void BettergramTabbedSelector::paintEvent(QPaintEvent *e) {
	Painter p(this);

	auto ms = getms();

	auto switching = (_slideAnimation != nullptr);
	if (switching) {
		paintSlideFrame(p, ms);
		if (!_a_slide.animating()) {
			_slideAnimation.reset();
			afterShown();
			emit slideFinished();
		}
	}
	else {
		paintContent(p);
	}
}

void BettergramTabbedSelector::paintSlideFrame(Painter &p, TimeMs ms) {
	if (_roundRadius > 0) {
		auto topPart = QRect(0, 0, width(), _tabsSlider->height() + _roundRadius);
		App::roundRect(p, topPart, st::emojiPanBg, ImageRoundRadius::Small, RectPart::FullTop | RectPart::NoTopBottom);
	}
	else {
		p.fillRect(0, 0, width(), _tabsSlider->height(), st::emojiPanBg);
	}

	auto slideDt = _a_slide.current(ms, 1.);
	_slideAnimation->paintFrame(p, slideDt, 1.);
}

void BettergramTabbedSelector::paintContent(Painter &p) {
	auto &bottomBg = st::emojiPanBg;
	if (_roundRadius > 0) {
		auto topPart = QRect(0, 0, width(), _tabsSlider->height() + _roundRadius);
		App::roundRect(p, topPart, st::emojiPanBg, ImageRoundRadius::Small, RectPart::FullTop | RectPart::NoTopBottom);

		auto bottomPart = QRect(0, _footerTop - _roundRadius, width(), st::emojiFooterHeight + _roundRadius);
		auto bottomParts = RectPart::NoTopBottom | RectPart::FullBottom;
		App::roundRect(p, bottomPart, bottomBg, ImageRoundRadius::Small, bottomParts);
	}
	else {
		p.fillRect(0, 0, width(), _tabsSlider->height(), st::emojiPanBg);
		p.fillRect(0, _footerTop, width(), st::emojiFooterHeight, bottomBg);
	}

	auto sidesTop = marginTop();
	auto sidesHeight = height() - sidesTop - marginBottom();
	if (_restrictedLabel) {
		p.fillRect(0, sidesTop, width(), sidesHeight, st::emojiPanBg);
	}
	else {
		p.fillRect(myrtlrect(width() - st::emojiScroll.width, sidesTop, st::emojiScroll.width, sidesHeight), st::emojiPanBg);
		p.fillRect(myrtlrect(0, sidesTop, st::buttonRadius, sidesHeight), st::emojiPanBg);
	}
}

int BettergramTabbedSelector::marginTop() const {
	return _tabsSlider->height() - st::lineWidth;
}

int BettergramTabbedSelector::marginBottom() const {
	return st::emojiFooterHeight;
}

bool BettergramTabbedSelector::preventAutoHide() const {
	return false;
}

QImage BettergramTabbedSelector::grabForAnimation() {
	auto slideAnimationData = base::take(_slideAnimation);
	auto slideAnimation = base::take(_a_slide);

	showAll();
	_topShadow->hide();
	_tabsSlider->hide();
	Ui::SendPendingMoveResizeEvents(this);

	auto result = QImage(size() * cIntRetinaFactor(), QImage::Format_ARGB32_Premultiplied);
	result.setDevicePixelRatio(cRetinaFactor());
	result.fill(Qt::transparent);
	render(&result);

	_a_slide = base::take(slideAnimation);
	_slideAnimation = base::take(slideAnimationData);

	return result;
}

bool BettergramTabbedSelector::wheelEventFromFloatPlayer(QEvent *e) {
	return _scroll->viewportEvent(e);
}

QRect BettergramTabbedSelector::rectForFloatPlayer() const {
	return mapToGlobal(_scroll->geometry());
}

BettergramTabbedSelector::~BettergramTabbedSelector() = default;

void BettergramTabbedSelector::hideFinished() {
	for (auto &tab : _tabs) {
		tab.widget()->panelHideFinished();
	}
	_a_slide.finish();
	_slideAnimation.reset();
}

void BettergramTabbedSelector::showStarted() {
	Auth().api().updateStickers();
	currentTab()->widget()->refreshRecent();
	currentTab()->widget()->preloadImages();
	_a_slide.finish();
	_slideAnimation.reset();
	showAll();
}

void BettergramTabbedSelector::beforeHiding() {
	if (!_scroll->isHidden()) {
		currentTab()->widget()->beforeHiding();
		if (_beforeHidingCallback) {
			_beforeHidingCallback(_currentTabType);
		}
	}
}

void BettergramTabbedSelector::afterShown() {
	if (!_a_slide.animating()) {
		showAll();
		currentTab()->widget()->afterShown();
		if (_afterShownCallback) {
			_afterShownCallback(_currentTabType);
		}
	}
}

void BettergramTabbedSelector::showAll() {
	currentTab()->footer()->show();
	_scroll->show();
	_bottomShadow->setVisible(false);

	_topShadow->show();
	_tabsSlider->show();
}

void BettergramTabbedSelector::hideForSliding() {
	hideChildren();
	_tabsSlider->show();
	_topShadow->show();
	currentTab()->widget()->clearSelection();
}

void BettergramTabbedSelector::onScroll() {
	auto scrollTop = _scroll->scrollTop();
	auto scrollBottom = scrollTop + _scroll->height();
	currentTab()->widget()->setVisibleTopBottom(scrollTop, scrollBottom);
}

void BettergramTabbedSelector::setRoundRadius(int radius) {
	_roundRadius = radius;
	_tabsSlider->setRippleTopRoundRadius(_roundRadius);
}

void BettergramTabbedSelector::createTabsSlider() {
	auto sections = QStringList();
	sections.push_back(lang(lng_switch_prices).toUpper());
	sections.push_back(lang(lng_switch_news).toUpper());
	//sections.push_back(lang(lng_switch_icos).toUpper());
	sections.push_back(lang(lng_switch_resources).toUpper());
	_tabsSlider->setSections(sections);

	_tabsSlider->setActiveSectionFast(static_cast<int>(_currentTabType));
	_tabsSlider->sectionActivated(
	) | rpl::start_with_next(
		[this] { switchTab(); },
		lifetime());
}

void BettergramTabbedSelector::switchTab() {
	auto tab = _tabsSlider->activeSection();
	Assert(tab >= 0 && tab < Tab::kCount);
	auto newTabType = static_cast<BettergramSelectorTab>(tab);
	if (_currentTabType == newTabType) {
		return;
	}

	auto wasTab = _currentTabType;
	currentTab()->saveScrollTop();

	beforeHiding();

	auto wasCache = grabForAnimation();

	auto widget = _scroll->takeWidget<TabbedSelector::Inner>();
	widget->setParent(this);
	widget->hide();
	currentTab()->footer()->hide();
	currentTab()->returnWidget(std::move(widget));

	_currentTabType = newTabType;
	_restrictedLabel.destroy();

	currentTab()->widget()->refreshRecent();
	currentTab()->widget()->preloadImages();
	setWidgetToScrollArea();

	auto nowCache = grabForAnimation();

	auto direction = (wasTab > _currentTabType) ? TabbedSelector::SlideAnimation::Direction::LeftToRight : TabbedSelector::SlideAnimation::Direction::RightToLeft;
	if (direction == TabbedSelector::SlideAnimation::Direction::LeftToRight) {
		std::swap(wasCache, nowCache);
	}
	_slideAnimation = std::make_unique<TabbedSelector::SlideAnimation>();
	auto slidingRect = QRect(_tabsSlider->x() * cIntRetinaFactor(), _scroll->y() * cIntRetinaFactor(), _tabsSlider->width() * cIntRetinaFactor(), (height() - _scroll->y()) * cIntRetinaFactor());
	_slideAnimation->setFinalImages(direction, std::move(wasCache), std::move(nowCache), slidingRect, false);
	auto corners = App::cornersMask(ImageRoundRadius::Small);
	_slideAnimation->setCornerMasks(corners[0], corners[1], corners[2], corners[3]);
	_slideAnimation->start();

	hideForSliding();

	getTab(wasTab)->widget()->hideFinished();

	_a_slide.start([this] { update(); }, 0., 1., st::emojiPanSlideDuration, anim::linear);
	update();

	Auth().settings().setBettergramSelectorTab(_currentTabType);
	Auth().saveSettingsDelayed();
}

void BettergramTabbedSelector::setWidgetToScrollArea() {
	auto inner = _scroll->setOwnedWidget(currentTab()->takeWidget());
	auto innerWidth = _scroll->width() - st::emojiScroll.width;
	auto scrollHeight = _scroll->height();
	inner->setMinimalHeight(innerWidth, scrollHeight);
	inner->moveToLeft(0, 0);
	inner->show();

	_scroll->disableScroll(false);
	scrollToY(currentTab()->getScrollTop());
	onScroll();
}

void BettergramTabbedSelector::scrollToY(int y) {
	_scroll->scrollToY(y);

	// Qt render glitch workaround, shadow sometimes disappears if we just scroll to y.
	_topShadow->update();
}
} // namespace ChatHelpers
