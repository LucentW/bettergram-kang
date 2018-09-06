#pragma once

#include "rss_widget.h"

namespace ChatHelpers {

/**
 * @brief The VideosWidget class shows Video feeds.
 */
class VideosWidget : public RssWidget
{
	Q_OBJECT

public:
	VideosWidget(QWidget* parent, not_null<Window::Controller*> controller);

protected:

private:

private slots:

};

} // namespace ChatHelpers
