#pragma once

/**
 * @brief TextHelper is a static helper class to draw texts.
 * Please note that it is used only by Bettergram code
 */
class TextHelper
{
public:
	TextHelper() = delete;

	static void drawElidedText(QPainter &painter, const QRect &rect, const QString &text);
};
