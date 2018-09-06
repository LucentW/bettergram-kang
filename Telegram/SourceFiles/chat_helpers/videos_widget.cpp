#include "videos_widget.h"

#include <bettergram/bettergramservice.h>

#include <styles/style_chat_helpers.h>

namespace ChatHelpers {

using namespace Bettergram;

VideosWidget::VideosWidget(QWidget* parent, not_null<Window::Controller*> controller)
	: RssWidget(parent,
				controller,
				BettergramService::instance()->videoChannelList(),
				st::videosPanNewsReadFg,
				st::videosPanNewsBodyFg,
				st::videosPanNewsHeaderFg,
				st::videosPanSiteNameFg,
				st::videosPanBg,
				st::videosPanHover,
				st::videosPanPadding,
				st::videosPanHeader,
				st::videosPanImageSize,
				st::videosPanRowVerticalPadding,
				st::videosPanRowHeight,
				st::videosPanChannelRowHeight,
				st::videosPanDateHeight)
{
}

} // namespace ChatHelpers
