#include "text_helper.h"

void TextHelper::drawElidedText(QPainter &painter, const QRect &rect, const QString &text)
{
	QFontMetrics fontMetrics = painter.fontMetrics();

	int lineSpacing = fontMetrics.lineSpacing();
	int y = rect.top();

	QTextLayout textLayout(text, painter.font());
	textLayout.beginLayout();

	while (true) {
		QTextLine line = textLayout.createLine();

		if (!line.isValid()) {
			break;
		}

		line.setLineWidth(rect.width());
		int nextLineY = y + lineSpacing;

		if (nextLineY + lineSpacing <= rect.bottom()) {
			line.draw(&painter, QPoint(rect.left(), y));
			y = nextLineY;
		} else {
			QString lastLine = text.mid(line.textStart());
			QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, rect.width());
			painter.drawText(QPoint(rect.left(), y + fontMetrics.ascent()), elidedLastLine);
			line = textLayout.createLine();

			break;
		}
	}

	textLayout.endLayout();
}