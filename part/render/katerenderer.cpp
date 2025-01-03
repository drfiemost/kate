/* This file is part of the KDE libraries
   Copyright (C) 2007 Mirko Stocker <me@misto.ch>
   Copyright (C) 2003-2005 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>
   Copyright (C) 2013 Andrey Matveyakin <a.matveyakin@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "katerenderer.h"

#include "katedocument.h"
#include "kateconfig.h"
#include "katehighlight.h"
#include "kateview.h"
#include "katerenderrange.h"
#include "katetextlayout.h"
#include "katebuffer.h"

#include <limits.h>

#include <kdebug.h>

#include <QtGui/QPainter>
#include <QtGui/QTextLine>
#include <QtCore/QStack>
#include <QtGui/QBrush>

#include <ktexteditor/highlightinterface.h>

#include <algorithm>
#include <cmath>

static const QChar tabChar('\t');
static const QChar spaceChar(' ');
static const QChar nbSpaceChar(0xa0); // non-breaking space

KateRenderer::KateRenderer(KateDocument* doc, Kate::TextFolding &folding, KateView *view)
  : m_doc(doc)
  , m_folding (folding)
    , m_view (view)
    , m_tabWidth(m_doc->config()->tabWidth())
    , m_indentWidth(m_doc->config()->indentationWidth())
    , m_caretStyle(KateRenderer::Line)
    , m_drawCaret(true)
    , m_showSelections(true)
    , m_showTabs(true)
    , m_showSpaces(true)
    , m_printerFriendly(false)
    , m_config(new KateRendererConfig(this))
{
  updateAttributes ();

  // initialize with a sane font height
  updateFontHeight ();
}

KateRenderer::~KateRenderer()
{
  delete m_config;
}

void KateRenderer::updateAttributes ()
{
  m_attributes = m_doc->highlight()->attributes (config()->schema ());
}

KTextEditor::Attribute::Ptr KateRenderer::attribute(uint pos) const
{
  if (pos < (uint)m_attributes.count())
    return m_attributes[pos];

  return m_attributes[0];
}

KTextEditor::Attribute::Ptr KateRenderer::specificAttribute( int context ) const
{
  if (context >= 0 && context < m_attributes.count())
    return m_attributes[context];

  return m_attributes[0];
}

void KateRenderer::setDrawCaret(bool drawCaret)
{
  m_drawCaret = drawCaret;
}

void KateRenderer::setCaretStyle(KateRenderer::caretStyles style)
{
  m_caretStyle = style;
}

void KateRenderer::setShowTabs(bool showTabs)
{
  m_showTabs = showTabs;
}

void KateRenderer::setShowTrailingSpaces(bool showSpaces)
{
  m_showSpaces = showSpaces;
}

void KateRenderer::setTabWidth(int tabWidth)
{
  m_tabWidth = tabWidth;
}

bool KateRenderer::showIndentLines() const
{
  return m_config->showIndentationLines();
}

void KateRenderer::setShowIndentLines(bool showIndentLines)
{
  m_config->setShowIndentationLines(showIndentLines);
}

void KateRenderer::setIndentWidth(int indentWidth)
{
  m_indentWidth = indentWidth;
}

void KateRenderer::setShowSelections(bool showSelections)
{
  m_showSelections = showSelections;
}

void KateRenderer::increaseFontSizes()
{
  QFont f ( config()->font () );
  f.setPointSize (f.pointSize ()+1);

  config()->setFont (f);
}

void KateRenderer::decreaseFontSizes()
{
  QFont f ( config()->font () );

  if ((f.pointSize ()-1) > 0)
    f.setPointSize (f.pointSize ()-1);

  config()->setFont (f);
}

bool KateRenderer::isPrinterFriendly() const
{
  return m_printerFriendly;
}

void KateRenderer::setPrinterFriendly(bool printerFriendly)
{
  m_printerFriendly = printerFriendly;
  setShowTabs(false);
  setShowTrailingSpaces(false);
  setShowSelections(false);
  setDrawCaret(false);
}

void KateRenderer::paintTextLineBackground(QPainter& paint, KateLineLayoutPtr layout, int currentViewLine, int xStart, int xEnd)
{
  if (isPrinterFriendly())
    return;

  // Normal background color
  QColor backgroundColor( config()->backgroundColor() );

  // paint the current line background if we're on the current line
  QColor currentLineColor = config()->highlightedLineColor();

  // Check for mark background
  int markRed = 0, markGreen = 0, markBlue = 0, markCount = 0;

  // Retrieve marks for this line
  uint mrk = m_doc->mark( layout->line() );
  if (mrk)
  {
    for (uint bit = 0; bit < 32; bit++)
    {
      KTextEditor::MarkInterface::MarkTypes markType = (KTextEditor::MarkInterface::MarkTypes)(1<<bit);
      if (mrk & markType)
      {
        QColor markColor = config()->lineMarkerColor(markType);

        if (markColor.isValid()) {
          markCount++;
          markRed += markColor.red();
          markGreen += markColor.green();
          markBlue += markColor.blue();
        }
      }
    } // for
  } // Marks

  if (markCount) {
    markRed /= markCount;
    markGreen /= markCount;
    markBlue /= markCount;
    backgroundColor.setRgb(
      int((backgroundColor.red() * 0.9) + (markRed * 0.1)),
      int((backgroundColor.green() * 0.9) + (markGreen * 0.1)),
      int((backgroundColor.blue() * 0.9) + (markBlue * 0.1)),
      backgroundColor.alpha()
    );
  }

  // Draw line background
  paint.fillRect(0, 0, xEnd - xStart, lineHeight() * layout->viewLineCount(), backgroundColor);

  // paint the current line background if we're on the current line
  const bool currentLineHasSelection = m_view && m_view->selection() && m_view->selectionRange().overlapsLine(layout->line());
    if (currentViewLine != -1 && !currentLineHasSelection) {
    if (markCount) {
      markRed /= markCount;
      markGreen /= markCount;
      markBlue /= markCount;
      currentLineColor.setRgb(
        int((currentLineColor.red() * 0.9) + (markRed * 0.1)),
        int((currentLineColor.green() * 0.9) + (markGreen * 0.1)),
        int((currentLineColor.blue() * 0.9) + (markBlue * 0.1)),
        currentLineColor.alpha()
      );
    }

    paint.fillRect(0, lineHeight() * currentViewLine, xEnd - xStart, lineHeight(), currentLineColor);
  }
}

void KateRenderer::paintTabstop(QPainter &paint, qreal x, qreal y)
{
  QPen penBackup( paint.pen() );
  QPen pen( config()->tabMarkerColor() );
  pen.setWidthF(std::max(1.0, spaceWidth() / 10.0));
  paint.setPen( pen );
  paint.setRenderHint(QPainter::Antialiasing, false);

  int dist = spaceWidth() * 0.3;
  QPoint points[8];
  points[0] = QPoint(x - dist, y - dist);
  points[1] = QPoint(x, y);
  points[2] = QPoint(x, y);
  points[3] = QPoint(x - dist, y + dist);
  x += spaceWidth() / 3.0;
  points[4] = QPoint(x - dist, y - dist);
  points[5] = QPoint(x, y);
  points[6] = QPoint(x, y);
  points[7] = QPoint(x - dist, y + dist);
  paint.drawLines(points, 4);
  paint.setPen( penBackup );
}

void KateRenderer::paintTrailingSpace(QPainter &paint, qreal x, qreal y)
{
  QPen penBackup( paint.pen() );
  QPen pen( config()->tabMarkerColor() );
  pen.setWidthF(spaceWidth() / 3.5);
  pen.setCapStyle(Qt::RoundCap);
  paint.setPen( pen );
  paint.setRenderHint(QPainter::Antialiasing, true);

  paint.drawPoint( QPointF(x, y) );
  paint.setPen( penBackup );
}

void KateRenderer::paintNonBreakSpace(QPainter &paint, qreal x, qreal y)
{
  QPen penBackup( paint.pen() );
  QPen pen( config()->tabMarkerColor() );
  pen.setWidthF(std::max(1.0, spaceWidth() / 10.0));
  paint.setPen( pen );
  paint.setRenderHint(QPainter::Antialiasing, false);

  const int height = fontHeight();
  const int width = spaceWidth();

  QPoint points[6];
  points[0] = QPoint(x+width/10, y+height/4);
  points[1] = QPoint(x+width/10, y+height/3);
  points[2] = QPoint(x+width/10, y+height/3);
  points[3] = QPoint(x+width-width/10, y+height/3);
  points[4] = QPoint(x+width-width/10, y+height/3);
  points[5] = QPoint(x+width-width/10, y+height/4);
  paint.drawLines(points, 3);
  paint.setPen( penBackup );
}

void KateRenderer::paintIndentMarker(QPainter &paint, uint x, uint /*row*/)
{
  QPen penBackup( paint.pen() );
  QPen myPen(config()->indentationLineColor());
  static const QVector<qreal> dashPattern = QVector<qreal>() << 1 << 1;
  myPen.setDashPattern(dashPattern);
  paint.setPen(myPen);

  QPainter::RenderHints renderHints = paint.renderHints();
  paint.setRenderHints(renderHints, false);

  paint.drawLine(x + 2, 0, x + 2, lineHeight());

  paint.setRenderHints(renderHints, true);

  paint.setPen( penBackup );
}

static bool rangeLessThanForRenderer (const Kate::TextRange *a, const Kate::TextRange *b)
{
  // compare Z-Depth first
  // smaller Z-Depths should win!
  if (a->zDepth() > b->zDepth())
    return true;
  else if (a->zDepth() < b->zDepth())
    return false;

  // end of a > end of b?
  if (a->end().toCursor() > b->end().toCursor())
    return true;

  // if ends are equal, start of a < start of b?
  if (a->end().toCursor() == b->end().toCursor())
    return a->start().toCursor() < b->start().toCursor();

  return false;
}

QList<QTextLayout::FormatRange> KateRenderer::decorationsForLine( const Kate::TextLine& textLine, int line, bool selectionsOnly, KateRenderRange* completionHighlight, bool completionSelected ) const
{
  QList<QTextLayout::FormatRange> newHighlight;

  // Don't compute the highlighting if there isn't going to be any highlighting
  QList<Kate::TextRange *> rangesWithAttributes = m_doc->buffer().rangesForLine (line, m_printerFriendly ? 0 : m_view, true);
  if (selectionsOnly || textLine->attributesList().count() || rangesWithAttributes.count()) {
    RenderRangeList renderRanges;

    // Add the inbuilt highlighting to the list
    NormalRenderRange* inbuiltHighlight = new NormalRenderRange();
    const QVector<Kate::TextLineData::Attribute> &al = textLine->attributesList();
    for (int i = 0; i < al.count(); ++i)
      if (al[i].length > 0 && al[i].attributeValue > 0)
        inbuiltHighlight->addRange(new KTextEditor::Range(KTextEditor::Cursor(line, al[i].offset), al[i].length), specificAttribute(al[i].attributeValue));
    renderRanges.append(inbuiltHighlight);

    if (!completionHighlight) {
      // check for dynamic hl stuff
      const QSet<Kate::TextRange *> *rangesMouseIn = m_view ? m_view->rangesMouseIn () : 0;
      const QSet<Kate::TextRange *> *rangesCaretIn = m_view ? m_view->rangesCaretIn () : 0;
      bool anyDynamicHlsActive = m_view && (!rangesMouseIn->empty() || !rangesCaretIn->empty());

      // sort all ranges, we want that the most specific ranges win during rendering, multiple equal ranges are kind of random, still better than old smart rangs behavior ;)
      std::sort (rangesWithAttributes.begin(), rangesWithAttributes.end(), rangeLessThanForRenderer);

      // loop over all ranges
      for (int i = 0; i < rangesWithAttributes.size(); ++i) {
        // real range
        Kate::TextRange *kateRange = rangesWithAttributes[i];

        // calculate attribute, default: normal attribute
        KTextEditor::Attribute::Ptr attribute = kateRange->attribute();
        if (anyDynamicHlsActive) {
          // check mouse in
          if (KTextEditor::Attribute::Ptr attributeMouseIn = attribute->dynamicAttribute (KTextEditor::Attribute::ActivateMouseIn)) {
            if (rangesMouseIn->contains (kateRange))
              attribute = attributeMouseIn;
          }

          // check caret in
          if (KTextEditor::Attribute::Ptr attributeCaretIn = attribute->dynamicAttribute (KTextEditor::Attribute::ActivateCaretIn)) {
            if (rangesCaretIn->contains (kateRange))
              attribute = attributeCaretIn;
          }
        }

        // span range
        NormalRenderRange *additionaHl = new NormalRenderRange();
        additionaHl->addRange(new KTextEditor::Range (*kateRange), attribute);
        renderRanges.append(additionaHl);
      }
    } else {
      // Add the code completion arbitrary highlight to the list
      renderRanges.append(completionHighlight);
    }

    // Add selection highlighting if we're creating the selection decorations
    if ((selectionsOnly && showSelections() && m_view->selection()) || (completionHighlight && completionSelected) || m_view->blockSelection()) {
      NormalRenderRange* selectionHighlight = new NormalRenderRange();

      // Set up the selection background attribute TODO: move this elsewhere, eg. into the config?
      static KTextEditor::Attribute::Ptr backgroundAttribute;
      if (!backgroundAttribute)
        backgroundAttribute = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());

      backgroundAttribute->setBackground(config()->selectionColor());
      backgroundAttribute->setForeground(attribute(KTextEditor::HighlightInterface::dsNormal)->selectedForeground().color());

      // Create a range for the current selection
      if (completionHighlight && completionSelected)
        selectionHighlight->addRange(new KTextEditor::Range(line, 0, line + 1, 0), backgroundAttribute);
      else
        if(m_view->blockSelection() && m_view->selectionRange().overlapsLine(line))
          selectionHighlight->addRange(new KTextEditor::Range(m_doc->rangeOnLine(m_view->selectionRange(), line)), backgroundAttribute);
        else {
          selectionHighlight->addRange(new KTextEditor::Range(m_view->selectionRange()), backgroundAttribute);
        }

      renderRanges.append(selectionHighlight);
    // highlighting for the vi visual modes
    }

    KTextEditor::Cursor currentPosition, endPosition;

    // Calculate the range which we need to iterate in order to get the highlighting for just this line
    if (selectionsOnly) {
      if(m_view->blockSelection()) {
        KTextEditor::Range subRange = m_doc->rangeOnLine(m_view->selectionRange(), line);
        currentPosition = subRange.start();
        endPosition = subRange.end();
      } else {
        KTextEditor::Range rangeNeeded = m_view->selectionRange() & KTextEditor::Range(line, 0, line + 1, 0);

        currentPosition = std::max(KTextEditor::Cursor(line, 0), rangeNeeded.start());
        endPosition = std::min(KTextEditor::Cursor(line + 1, 0), rangeNeeded.end());
      }
    } else {
      currentPosition = KTextEditor::Cursor(line, 0);
      endPosition = KTextEditor::Cursor(line + 1, 0);
    }

    // Main iterative loop.  This walks through each set of highlighting ranges, and stops each
    // time the highlighting changes.  It then creates the corresponding QTextLayout::FormatRanges.
    while (currentPosition < endPosition) {
      renderRanges.advanceTo(currentPosition);

      if (!renderRanges.hasAttribute()) {
        // No attribute, don't need to create a FormatRange for this text range
        currentPosition = renderRanges.nextBoundary();
        continue;
      }

      KTextEditor::Cursor nextPosition = renderRanges.nextBoundary();

      // Create the format range and populate with the correct start, length and format info
      QTextLayout::FormatRange fr;
      fr.start = currentPosition.column();

      if (nextPosition < endPosition || endPosition.line() <= line) {
        fr.length = nextPosition.column() - currentPosition.column();

      } else {
        // +1 to force background drawing at the end of the line when it's warranted
        fr.length = textLine->length() - currentPosition.column() + 1;
      }

      KTextEditor::Attribute::Ptr a = renderRanges.generateAttribute();
      if (a) {
        fr.format = *a;

        if(selectionsOnly) {
              assignSelectionBrushesFromAttribute(fr, *a);
        }
      }

      newHighlight.append(fr);

      currentPosition = nextPosition;
    }

    if (completionHighlight)
      // Don't delete external completion render range
      renderRanges.removeAll(completionHighlight);

    qDeleteAll(renderRanges);
  }

  return newHighlight;
}

void KateRenderer::assignSelectionBrushesFromAttribute(QTextLayout::FormatRange& target, const KTextEditor::Attribute& attribute) const
{
  if(attribute.hasProperty(KTextEditor::Attribute::SelectedForeground)) {
    target.format.setForeground(attribute.selectedForeground());
  }
  if(attribute.hasProperty(KTextEditor::Attribute::SelectedBackground)) {
    target.format.setBackground(attribute.selectedBackground());
  }
}

/*
The ultimate line painting function.
Currently missing features:
- draw indent lines
*/
void KateRenderer::paintTextLine(QPainter& paint, KateLineLayoutPtr range, int xStart, int xEnd, const KTextEditor::Cursor* cursor)
{
  Q_ASSERT(range->isValid());

//   kDebug( 13033 )<<"KateRenderer::paintTextLine";

  // font data
  const QFontMetricsF &fm = config()->fontMetrics();

  int currentViewLine = -1;
  if (cursor && cursor->line() == range->line())
    currentViewLine = range->viewLineForColumn(cursor->column());

  paintTextLineBackground(paint, range, currentViewLine, xStart, xEnd);

  if (range->layout()) {
    bool drawSelection = m_view->selection() && showSelections() && m_view->selectionRange().overlapsLine(range->line());
    // Draw selection in block selecton mode. We need 2 kinds of selections that QTextLayout::draw can't render:
    //   - past-end-of-line selection and
    //   - 0-column-wide selection (used to indicate where text will be typed)
    if (drawSelection && m_view->blockSelection()) {
      int selectionStartColumn = m_doc->fromVirtualColumn(range->line(), m_doc->toVirtualColumn(m_view->selectionRange().start()));
      int selectionEndColumn   = m_doc->fromVirtualColumn(range->line(), m_doc->toVirtualColumn(m_view->selectionRange().end()));
      QBrush selectionBrush = config()->selectionColor();
      if (selectionStartColumn != selectionEndColumn) {
        KateTextLayout lastLine = range->viewLine(range->viewLineCount() - 1);
        if (selectionEndColumn > lastLine.startCol()) {
          int selectionStartX = (selectionStartColumn > lastLine.startCol()) ? cursorToX(lastLine, selectionStartColumn, true) : 0;
          int selectionEndX = cursorToX(lastLine, selectionEndColumn, true);
          paint.fillRect(QRect(selectionStartX - xStart, (int)lastLine.lineLayout().y(), selectionEndX - selectionStartX, lineHeight()), selectionBrush);
        }
      } else {
        const int selectStickWidth = 2;
        KateTextLayout selectionLine = range->viewLine(range->viewLineForColumn(selectionStartColumn));
        int selectionX = cursorToX(selectionLine, selectionStartColumn, true);
        paint.fillRect(QRect(selectionX - xStart, (int)selectionLine.lineLayout().y(), selectStickWidth, lineHeight()), selectionBrush);
      }
    }

    QVector<QTextLayout::FormatRange> additionalFormats;
    if (range->length() > 0) {
      // We may have changed the pen, be absolutely sure it gets set back to
      // normal foreground color before drawing text for text that does not
      // set the pen color
      paint.setPen(attribute(KTextEditor::HighlightInterface::dsNormal)->foreground().color());
      // Draw the text :)
      if (drawSelection) {
        // FIXME toVector() may be a performance issue
        additionalFormats = decorationsForLine(range->textLine(), range->line(), true).toVector();
        range->layout()->draw(&paint, QPoint(-xStart,0), additionalFormats);

      } else {
        range->layout()->draw(&paint, QPoint(-xStart,0));
      }
    }

    QBrush backgroundBrush;
    bool backgroundBrushSet = false;

    // Loop each individual line for additional text decoration etc.
    QListIterator<QTextLayout::FormatRange> it = range->layout()->additionalFormats();
    QVectorIterator<QTextLayout::FormatRange> it2 = additionalFormats;
    for (int i = 0; i < range->viewLineCount(); ++i) {
      KateTextLayout line = range->viewLine(i);

      bool haveBackground = false;
      // Determine the background to use, if any, for the end of this view line
      backgroundBrushSet = false;
      while (it2.hasNext()) {
        const QTextLayout::FormatRange& fr = it2.peekNext();
        if (fr.start > line.endCol())
          break;

        if (fr.start + fr.length > line.endCol()) {
          if (fr.format.hasProperty(QTextFormat::BackgroundBrush)) {
            backgroundBrushSet = true;
            backgroundBrush = fr.format.background();
          }

          haveBackground = true;
          break;
        }

        it2.next();
      }

      while (!haveBackground && it.hasNext()) {
        const QTextLayout::FormatRange& fr = it.peekNext();
        if (fr.start > line.endCol())
          break;

        if (fr.start + fr.length > line.endCol()) {
          if (fr.format.hasProperty(QTextFormat::BackgroundBrush)) {
            backgroundBrushSet = true;
            backgroundBrush = fr.format.background();
          }

          break;
        }

        it.next();
      }

      // Draw selection or background color outside of areas where text is rendered
      if (!m_printerFriendly ) {
        bool draw = false;
        QBrush drawBrush;
        if (m_view->selection() && !m_view->blockSelection() && m_view->lineEndSelected(line.end(true))) {
          draw = true;
          drawBrush = config()->selectionColor();
        } else if (backgroundBrushSet && !m_view->blockSelection()) {
          draw = true;
          drawBrush = backgroundBrush;
        }

        if (draw) {
          int fillStartX = line.endX() - line.startX() + line.xOffset() - xStart;
          int fillStartY = lineHeight() * i;
          int width= xEnd - xStart - fillStartX;
          int height= lineHeight();

          // reverse X for right-aligned lines
          if (range->layout()->textOption().alignment() == Qt::AlignRight)
            fillStartX = 0;

          if (width > 0) {
            QRect area(fillStartX, fillStartY, width, height);
            paint.fillRect(area, drawBrush);
          }
        }
      }
      // Draw indent lines
      if (showIndentLines() && i == 0)
      {
        const qreal w = spaceWidth();
        const int lastIndentColumn = range->textLine()->indentDepth(m_tabWidth);

        for (int x = m_indentWidth; x < lastIndentColumn; x += m_indentWidth)
        {
          paintIndentMarker(paint, x * w + 1 - xStart, range->line());
        }
      }

      // draw an open box to mark non-breaking spaces
      const QString& text = range->textLine()->string();
      int y = lineHeight() * i + fm.ascent() - fm.strikeOutPos();
      int nbSpaceIndex = text.indexOf(nbSpaceChar, line.lineLayout().xToCursor(xStart));

      while (nbSpaceIndex != -1 && nbSpaceIndex < line.endCol()) {
        int x = line.lineLayout().cursorToX(nbSpaceIndex);
        if (x > xEnd)
          break;
        paintNonBreakSpace(paint, x - xStart, y);
        nbSpaceIndex = text.indexOf(nbSpaceChar, nbSpaceIndex + 1);
      }

      // draw tab stop indicators
      if (showTabs()) {
        int tabIndex = text.indexOf(tabChar, line.lineLayout().xToCursor(xStart));
        while (tabIndex != -1 && tabIndex < line.endCol()) {
          int x = line.lineLayout().cursorToX(tabIndex);
          if (x > xEnd)
            break;
          paintTabstop(paint, x - xStart + spaceWidth()/2.0, y);
          tabIndex = text.indexOf(tabChar, tabIndex + 1);
        }
      }

      // draw trailing spaces
      if (showTrailingSpaces()) {
        int spaceIndex = line.endCol() - 1;
        int trailingPos = range->textLine()->lastChar();
        if (trailingPos < 0)
          trailingPos = 0;
        if (spaceIndex >= trailingPos) {
          while (spaceIndex >= line.startCol() && text.at(spaceIndex).isSpace()) {
            if (text.at(spaceIndex) != '\t' || !showTabs())
              paintTrailingSpace(paint, line.lineLayout().cursorToX(spaceIndex) - xStart + spaceWidth()/2.0, y);
            --spaceIndex;
          }
        }
      }

      // draw word-wrap-honor-indent filling
      if ((i > 0) && range->shiftX() && (range->shiftX() > xStart)) {
          // fill background first with selection if we had selection from the previous line
          if (drawSelection && !m_view->blockSelection() && m_view->selectionRange().start() < line.start()
              && m_view->selectionRange().end() >= line.start()) {
              paint.fillRect(0, lineHeight() * i, range->shiftX() - xStart, lineHeight(), QBrush(config()->selectionColor()));
          }

          // paint the normal filling for the word wrap markers
          paint.fillRect(0, lineHeight() * i, range->shiftX() - xStart, lineHeight(), QBrush(config()->wordWrapMarkerColor(), Qt::Dense4Pattern));
      }
    }

    // Draw caret
    if (drawCaret() && cursor && range->includesCursor(*cursor)) {
      int caretWidth, lineWidth = 2;
      QColor color;
      QTextLine line = range->layout()->lineForTextPosition(std::min(cursor->column(), range->length()));

      // Determine the caret's style
      caretStyles style = caretStyle();

      // Make the caret the desired width
      if (style == Line) {
        caretWidth = lineWidth;
      } else if (line.isValid() && cursor->column() < range->length()) {
        caretWidth = int(line.cursorToX(cursor->column() + 1) - line.cursorToX(cursor->column()));
        if (caretWidth < 0) {
          caretWidth = -caretWidth;
        }
      } else {
        caretWidth = spaceWidth();
      }

      // Determine the color
      if (m_caretOverrideColor.isValid()) {
        // Could actually use the real highlighting system for this...
        // would be slower, but more accurate for corner cases
        color = m_caretOverrideColor;
      } else {
        // search for the FormatRange that includes the cursor
        foreach (const QTextLayout::FormatRange &r, range->layout()->additionalFormats()) {
          if ((r.start <= cursor->column() ) && ( (r.start + r.length)  > cursor->column())) {
            // check for Qt::NoBrush, as the returned color is black() and no invalid QColor
            QBrush foregroundBrush = r.format.foreground();
            if (foregroundBrush != Qt::NoBrush) {
              color = r.format.foreground().color();
            }
            break;
          }
        }
        // still no color found, fall back to default style
        if (!color.isValid())
          color = attribute(KTextEditor::HighlightInterface::dsNormal)->foreground().color();
      }

      // Clip the caret - Qt's caret has a habit of intruding onto other lines.
      paint.save();
      paint.setClipRect(0, line.lineNumber() * lineHeight(), xEnd - xStart, lineHeight());
      switch(style) {
      case Line :
        paint.setPen(QPen(color, caretWidth));
        break;
      case Block :
        // use a gray caret so it's possible to see the character
        color.setAlpha(128);
        paint.setPen(QPen(color, caretWidth));
        break;
      case Underline :
        paint.setClipRect(0, lineHeight() - lineWidth, xEnd - xStart, lineWidth);
        break;
      case Half :
        color.setAlpha(128);
        paint.setPen(QPen(color, caretWidth));
        paint.setClipRect(0, lineHeight() / 2, xEnd - xStart, lineHeight() / 2);
        break;
      }

      if (cursor->column() <= range->length()) {
        range->layout()->drawCursor(&paint, QPoint(-xStart,0), cursor->column(), caretWidth);
      } else {
        // Off the end of the line... must be block mode. Draw the caret ourselves.
        const KateTextLayout& lastLine = range->viewLine(range->viewLineCount() - 1);
        int x = cursorToX(lastLine, KTextEditor::Cursor(range->line(), cursor->column()), true);
        if ((x >= xStart) && (x <= xEnd)) {
          paint.fillRect(x - xStart, (int)lastLine.lineLayout().y(), caretWidth, lineHeight(), color);
        }
      }

      paint.restore();
    }
  }

  // Draws the dashed underline at the start of a folded block of text.
  if (range->startsInvisibleBlock()) {
    const QPainter::RenderHints backupRenderHints = paint.renderHints();
    paint.setRenderHint(QPainter::Antialiasing, false);
    QPen pen(config()->wordWrapMarkerColor());
    pen.setCosmetic(true);
    pen.setStyle(Qt::DashLine);
    pen.setDashOffset(xStart);
    paint.setPen(pen);
    paint.drawLine(0, (lineHeight() * range->viewLineCount()) - 1, xEnd - xStart, (lineHeight() * range->viewLineCount()) - 1);
    paint.setRenderHints(backupRenderHints);
  }

  // show word wrap marker if desirable
  if ((!isPrinterFriendly()) && config()->wordWrapMarker() && QFontInfo(config()->font()).fixedPitch())
  {
    const QPainter::RenderHints backupRenderHints = paint.renderHints();
    paint.setRenderHint(QPainter::Antialiasing, false);
    paint.setPen( config()->wordWrapMarkerColor() );
    int _x = qreal(m_doc->config()->wordWrapAt()) * fm.width('x') - xStart;
    paint.drawLine( _x,0,_x,lineHeight() );
    paint.setRenderHints(backupRenderHints);
  }
}

const QFont& KateRenderer::currentFont() const
{
  return config()->font();
}

const QFontMetricsF& KateRenderer::currentFontMetrics() const
{
  return config()->fontMetrics();
}

uint KateRenderer::fontHeight() const
{
  return m_fontHeight;
}

uint KateRenderer::documentHeight() const
{
  return m_doc->lines() * lineHeight();
}

int KateRenderer::lineHeight() const
{
  return fontHeight();
}

bool KateRenderer::getSelectionBounds(int line, int lineLength, int &start, int &end) const
{
  bool hasSel = false;

  if (m_view->selection() && !m_view->blockSelection())
  {
    if (m_view->lineIsSelection(line))
    {
      start = m_view->selectionRange().start().column();
      end = m_view->selectionRange().end().column();
      hasSel = true;
    }
    else if (line == m_view->selectionRange().start().line())
    {
      start = m_view->selectionRange().start().column();
      end = lineLength;
      hasSel = true;
    }
    else if (m_view->selectionRange().containsLine(line))
    {
      start = 0;
      end = lineLength;
      hasSel = true;
    }
    else if (line == m_view->selectionRange().end().line())
    {
      start = 0;
      end = m_view->selectionRange().end().column();
      hasSel = true;
    }
  }
  else if (m_view->lineHasSelected(line))
  {
    start = m_view->selectionRange().start().column();
    end = m_view->selectionRange().end().column();
    hasSel = true;
  }

  if (start > end) {
    int temp = end;
    end = start;
    start = temp;
  }

  return hasSel;
}

void KateRenderer::updateConfig ()
{
  // update the attibute list pointer
  updateAttributes ();

  if (m_view)
    m_view->updateRendererConfig();
  
  // update font height
  updateFontHeight ();
}

void KateRenderer::updateFontHeight ()
{
  // first: get normal line spacing
  m_fontHeight = config()->fontMetrics().height();
  
  // Sometimes the height of italic fonts is larger than for the non-italic
  // font. Since all our lines are of same/fixed height, use the maximum of
  // both heights (bug #302748)
  QFont italicFont = config()->font();
  italicFont.setItalic(true);
  m_fontHeight = std::max (m_fontHeight, QFontMetrics(italicFont).height());
  
  // same for bold font
  QFont boldFont = config()->font();
  boldFont.setBold (true);
  m_fontHeight = std::max (m_fontHeight, QFontMetrics(boldFont).height());
}

qreal KateRenderer::spaceWidth() const
{
  return config()->fontMetrics().width(spaceChar);
}

void KateRenderer::layoutLine(KateLineLayoutPtr lineLayout, int maxwidth, bool cacheLayout) const
{
  // if maxwidth == -1 we have no wrap

  Kate::TextLine textLine = lineLayout->textLine();
  Q_ASSERT(textLine);

  QTextLayout* l = lineLayout->layout();
  if (!l) {
    l = new QTextLayout(textLine->string(), config()->font());
  } else {
    l->setText(textLine->string());
    l->setFont(config()->font());
  }

  l->setCacheEnabled(cacheLayout);

  // Initial setup of the QTextLayout.

  // Tab width
  QTextOption opt;
  opt.setFlags(QTextOption::IncludeTrailingSpaces);
  opt.setTabStop(m_tabWidth * config()->fontMetrics().width(spaceChar));
  opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

  // Find the first strong character in the string.
  // If it is an RTL character, set the base layout direction of the string to RTL.
  //
  // See http://www.unicode.org/reports/tr9/#The_Paragraph_Level (Sections P2 & P3).
  // Qt's text renderer ("scribe") version 4.2 assumes a "higher-level protocol"
  // (such as KatePart) will specify the paragraph level, so it does not apply P2 & P3
  // by itself. If this ever change in Qt, the next code block could be removed.
  if (isLineRightToLeft(lineLayout)) {
      opt.setAlignment( Qt::AlignRight );
      opt.setTextDirection( Qt::RightToLeft );
  }
  else {
      opt.setAlignment( Qt::AlignLeft );
      opt.setTextDirection( Qt::LeftToRight );
  }

  l->setTextOption(opt);

  // Syntax highlighting, inbuilt and arbitrary
  l->setAdditionalFormats(decorationsForLine(textLine, lineLayout->line()));

  // Begin layouting
  l->beginLayout();

  int height = 0;
  int shiftX = 0;

  bool needShiftX = (maxwidth != -1)
                 && (m_view->config()->dynWordWrapAlignIndent() > 0);

  forever {
    QTextLine line = l->createLine();
    if (!line.isValid())
      break;

    if (maxwidth > 0)
      line.setLineWidth(maxwidth);

    line.setPosition(QPoint(line.lineNumber() ? shiftX : 0, height));

    if (needShiftX && line.width() > 0) {
      needShiftX = false;
      // Determine x offset for subsequent-lines-of-paragraph indenting
      int pos = textLine->nextNonSpaceChar(0);

      if (pos > 0) {
        shiftX = (int)line.cursorToX(pos);
      }

      // check for too deep shift value and limit if necessary
      if (shiftX > ((double)maxwidth / 100 * m_view->config()->dynWordWrapAlignIndent()))
        shiftX = 0;

      // if shiftX > 0, the maxwidth has to adapted
      maxwidth -= shiftX;

      lineLayout->setShiftX(shiftX);
    }

    height += lineHeight();
  }

  l->endLayout();

  lineLayout->setLayout(l);
}


// 1) QString::isRightToLeft() sux
// 2) QString::isRightToLeft() is marked as internal (WTF?)
// 3) QString::isRightToLeft() does not seem to work on my setup
// 4) isStringRightToLeft() should behave much better than QString::isRightToLeft() therefore:
// 5) isStringRightToLeft() kicks ass
bool KateRenderer::isLineRightToLeft( KateLineLayoutPtr lineLayout ) const
{
  QString s = lineLayout->textLine()->string();
  int i = 0;

  // borrowed from QString::updateProperties()
  while( i != s.length() )
  {
    QChar c = s.at(i);

    switch(c.direction()) {
      case QChar::DirL:
      case QChar::DirLRO:
      case QChar::DirLRE:
          return false;

      case QChar::DirR:
      case QChar::DirAL:
      case QChar::DirRLO:
      case QChar::DirRLE:
          return true;

      default:
          break;
    }
    i ++;
  }

   return false;
#if 0
  // or should we use the direction of the widget?
  QWidget* display = qobject_cast<QWidget*>(view()->parent());
  if (!display)
    return false;
  return display->layoutDirection() == Qt::RightToLeft;
#endif
}

int KateRenderer::cursorToX(const KateTextLayout& range, int col, bool returnPastLine) const
{
  return cursorToX(range, KTextEditor::Cursor(range.line(), col), returnPastLine);
}

int KateRenderer::cursorToX(const KateTextLayout& range, const KTextEditor::Cursor & pos, bool returnPastLine) const
{
  Q_ASSERT(range.isValid());

  int x;
  if (range.lineLayout().width() > 0) {
    x = (int)range.lineLayout().cursorToX(pos.column());
  } else {
    x = 0;
  }

  int over = pos.column() - range.endCol();
  if (returnPastLine && over > 0)
    x += over * spaceWidth();

  return x;
}

KTextEditor::Cursor KateRenderer::xToCursor(const KateTextLayout & range, int x, bool returnPastLine ) const
{
  Q_ASSERT(range.isValid());
  KTextEditor::Cursor ret(range.line(), range.lineLayout().xToCursor(x));

  // TODO wrong for RTL lines?
  if (returnPastLine && range.endCol(true) == -1 && x > range.width() + range.xOffset())
    ret.setColumn(ret.column() + std::round((x - (range.width() + range.xOffset())) / spaceWidth()));

  return ret;
}

void KateRenderer::setCaretOverrideColor(const QColor& color)
{
  m_caretOverrideColor = color;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
