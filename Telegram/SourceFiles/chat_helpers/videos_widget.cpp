#include "videos_widget.h"

#include <bettergram/bettergramservice.h>

#include <styles/style_chat_helpers.h>
#include <lang/lang_keys.h>

namespace ChatHelpers {

using namespace Bettergram;

VideosWidget::VideosWidget(QWidget* parent, not_null<Window::Controller*> controller)
	: RssWidget(parent,
				controller,
				BettergramService::instance()->videoChannelList(),
				lang(lng_videos_show_only_unread_videos),
				lang(lng_videos_show_all_videos),
				lang(lng_menu_videos_mark_as_watched),
				lang(lng_menu_videos_mark_all_site_videos_as_watched),
				lang(lng_menu_videos_mark_all_videos_as_watched),
				st::videosPanNewsReadFg,
				st::videosPanNewsBodyFg,
				st::videosPanNewsHeaderFg,
				st::videosPanSiteNameFg,
				st::videosPanBg,
				st::videosPanHover,
				st::videosPanPadding,
				st::videosPanHeader,
				st::videosPanImageWidth,
				st::videosPanImageHeight,
				st::videosPanRowVerticalPadding,
				st::videosPanRowHeight,
				st::videosPanChannelRowHeight,
				st::videosPanDateHeight,
				true,
				false)
{
}

} // namespace ChatHelpers
