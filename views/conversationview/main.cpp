/*
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

#include <QApplication>
//#include <QSplitter>
#include <QListView>
#include <QItemSelectionModel>
#include <QObject>
#include <QList>
#include <QScrollArea>
#include <QAbstractItemDelegate>

#include "foldermodel.h"
#include "conversationdelegate.h"
// #include "mydisplaywidget.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
//	QSplitter *splitter = new QSplitter;

  DummyKonadiAdapter data;

  QAbstractItemModel *model = new FolderModel(data);

  QListView *conversationList = new QListView;
  conversationList->setModel(model);

  ConversationDelegate *delegate = new ConversationDelegate;
  conversationList->setItemDelegate(delegate);

//   ConversationDisplay *conversationDisplay = new ConversationDisplay(this, &data);
//   QScrollArea *scrollArea = new QScrollArea;
//   scrollArea->setWidget(conversationDisplay);
//   scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QItemSelectionModel *selection = new QItemSelectionModel(model);
  conversationList->setSelectionModel(selection);

//   QObject::connect(conversationList, SIGNAL(clicked(const QModelIndex&)),
//                    conversationDisplay, SLOT(setConversation(const QModelIndex)));
//   QObject::connect(conversationList, SIGNAL(activated(const QModelIndex&)),
//                    conversationDisplay, SLOT(setConversation(const QModelIndex)));

/*  splitter->setWindowTitle("Conversations for KMail");
  splitter->addWidget(conversationList);
  splitter->addWidget(conversationDisplay);
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);
  QList<int> sizes;
  sizes << 275 << 650;
  splitter->setSizes(sizes);

  splitter->show();*/
  conversationList->show();
  return app.exec();
}


