/*
 * conversationdelegate.cpp
 *
 * copyright (c) Aron Bostrom <Aron.Bostrom at gmail.com>, 2006 
 *
 * this library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QStyle>
#include <QBrush>
#include <QSize>
#include <QDateTime>
#include <QtDebug>

#include "conversationdelegate.h"

ConversationDelegate::ConversationDelegate(DummyKonadiAdapter &adapter, QObject *parent) : QAbstractItemDelegate(parent)
{
  backend = adapter;
  lineWidth = 500;
  authorBaseWidth = 175;
  margin = 5;
}

ConversationDelegate::~ConversationDelegate()
{
}

void ConversationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  int lineHeight = option.fontMetrics.height() + 2;
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(Qt::NoPen);

  if (option.state & QStyle::State_Selected)
    painter->setBrush(option.palette.highlight());
  else
    painter->setBrush(Qt::white);
  painter->drawRect(option.rect);

  if (option.state & QStyle::State_Selected) {
    painter->setBrush(option.palette.highlightedText());
    painter->setPen(QPen(option.palette.highlightedText().color()));      
  } else {
    painter->setBrush(Qt::black);
    painter->setPen(Qt::black);
  }
  painter->setFont(option.font);
  DummyKonadiConversation c = backend.conversation(index.row());
  QString ctitle = c.conversationTitle();
  QString ctime = c.arrivalTimeInText();
  QString cauthors = c.author(0);
  int messageCount = c.count();
  for (int count = 1; count < messageCount; ++count) {
    cauthors.append(", ");
    cauthors.append(c.author(count));
  }

  QString messageCountText = QString("(%L1)").arg(messageCount);
  int messageCountWidth = option.fontMetrics.width(messageCountText);

  int authorWidth = authorBaseWidth - (messageCount > 1 ? messageCountWidth + margin : 0);
  int authorPos = margin;
  int linePos = index.row() * lineHeight;
  painter->drawText(authorPos, linePos, authorWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, cauthors);

  if (messageCount > 1) {
    int messageCountPos = 2*margin + authorWidth;
    painter->drawText(messageCountPos, linePos, messageCountWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, messageCountText);
  }

  int subjectPos = authorBaseWidth + 2*margin;
  int timeWidth = option.fontMetrics.width(ctime);
  int subjectWidth = lineWidth - subjectPos - timeWidth - 2*margin;
  painter->drawText(subjectPos, linePos, subjectWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, ctitle);

  int timePos = qMax(lineWidth - margin - timeWidth, authorBaseWidth + 2*margin);
  timeWidth = qMax(lineWidth - (timePos + margin), 0);
  painter->drawText(timePos, linePos, timeWidth, option.fontMetrics.height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, ctime);
}

QSize ConversationDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
  int lineHeight = option.fontMetrics.height() + 2;
  return QSize(0, lineHeight);
}

void ConversationDelegate::updateWidth(int pos, int /*nouse*/)
{
  lineWidth = pos;
}
