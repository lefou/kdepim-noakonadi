// -*- tab-width: 2 -*-

/***************************************************************************
                          knviewheader.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdebug.h>

#include "utilities.h"
#include "knviewheader.h"


//=============================================================================================================


// some standard headers
static const char *predef[] = { "Approved","Content-Transfer-Encoding","Content-Type","Control","Date","Distribution",
                                "Expires","Followup-To","From","Lines","Message-ID","Mime-Version","NNTP-Posting-Host",
                                "Newsgroups","Organization","Path","References","Reply-To","Sender","Subject","Supersedes",
                                "To", "User-Agent","X-Mailer","X-Newsreader","X-No-Archive","XRef",0 };

// default display names KNode uses
static const char *disp[] = { "Groups", 0 };

void dummyHeader()
{
  i18n("it's not very important to translate this","Approved");
  i18n("it's not very important to translate this","Content-Transfer-Encoding");
  i18n("it's not very important to translate this","Content-Type");
  i18n("it's not very important to translate this","Control");
  i18n("it's not very important to translate this","Date");
  i18n("it's not very important to translate this","Distribution");
  i18n("it's not very important to translate this","Expires");
  i18n("it's not very important to translate this","Followup-To");
  i18n("it's not very important to translate this","From");
  i18n("it's not very important to translate this","Lines");
  i18n("it's not very important to translate this","Message-ID");
  i18n("it's not very important to translate this","Mime-Version");
  i18n("it's not very important to translate this","NNTP-Posting-Host");
  i18n("it's not very important to translate this","Newsgroups");
  i18n("it's not very important to translate this","Organization");
  i18n("it's not very important to translate this","Path");
  i18n("it's not very important to translate this","References");
  i18n("it's not very important to translate this","Reply-To");
  i18n("it's not very important to translate this","Sender");
  i18n("it's not very important to translate this","Subject");
  i18n("it's not very important to translate this","Supersedes");
 // i18n("it's not very important to translate this","To");    I forgot one header, uncomment after message freeze !!!!!!!!
  i18n("it's not very important to translate this","User-Agent");
  i18n("it's not very important to translate this","X-Mailer");
  i18n("it's not very important to translate this","X-Newsreader");
  i18n("it's not very important to translate this","X-No-Archive");
  i18n("it's not very important to translate this","XRef");

  i18n("it's not very important to translate this","Groups");
}


//=============================================================================================================


QList<KNViewHeader> KNViewHeader::instances;

KNViewHeader::KNViewHeader()
 : translateName(true)
{
  flags.fill(false, 8);
  flags[1] = true;   // header name bold by default
}



KNViewHeader::~KNViewHeader()
{
}


void KNViewHeader::loadAll()
{
  QString fname(KGlobal::dirs()->findResource("appdata","headers.rc"));
  if (fname == QString::null)
    return;
  KSimpleConfig headerConf(fname,true);
  QStringList headers = headerConf.groupList();
  headers.remove("<default>");
  
  KNViewHeader *h;
  QValueList<int> flags;
  
  QStringList::Iterator it;
  for( it = headers.begin(); it != headers.end(); ++it ) {
    h=newItem();
    headerConf.setGroup((*it));
    h->n_ame = headerConf.readEntry("Name");
    h->translateName = headerConf.readBoolEntry("Translate_Name",true);
    h->h_eader = headerConf.readEntry("Header");
    flags = headerConf.readIntListEntry("Flags");
    if (h->n_ame.isNull()||h->h_eader.isNull()||(flags.count()!=8)) {
      kdWarning(5003) << "KNViewHeader::loadAll() : ignoring invalid/incomplete Header" << endl;
      remove(h);
    } else {
      for (int i=0; i<8; i++)
        h->flags.setBit(i,(flags[i]>0));
      h->createTags();
    }
  }
}



void KNViewHeader::saveAll()
{
  QString dir(KGlobal::dirs()->saveLocation("appdata"));
  if (dir==QString::null) {
    displayInternalFileError();
    return;
  }
  KSimpleConfig headerConf(dir+"headers.rc");
  QStringList oldHeaders = headerConf.groupList();
  
  QStringList::Iterator oldIt=oldHeaders.begin();
  for( ;oldIt != oldHeaders.end(); ++oldIt )      // remove all old groups
    headerConf.deleteGroup((*oldIt));             // find a better way to do it?
  
  QValueList<int> flags;
  int idx=0;
  
  for( QListIterator<KNViewHeader> it(instances); it.current(); ++it ) {
    headerConf.setGroup(QString::number(idx++));
    headerConf.writeEntry("Name",(*it)->n_ame);
    headerConf.writeEntry("Translate_Name",(*it)->translateName);
    headerConf.writeEntry("Header",(*it)->h_eader);
    flags.clear();
    for (int i=0; i<8; i++) {
      if ((*it)->flags[i])
        flags << 1;
      else
        flags << 0;
    }
    headerConf.writeEntry("Flags",flags);
  } 
}



KNViewHeader* KNViewHeader::newItem()
{
  KNViewHeader *h=new KNViewHeader();
  instances.append(h);
  // kdDebug(5003) << "KNViewHeader::newItem() : instance added" << endl;
  return h;
}


void KNViewHeader::remove(KNViewHeader *h)
{
  instances.setAutoDelete(true);
  if (!instances.remove(h))
    kdDebug(5003) << "KNViewHeader::remove() : cannot find pointer in list !!" << endl;
  // else kdDebug(5003) << "KNViewHeader::remove() : instance removed" << endl;
  
  instances.setAutoDelete(false);
}



void KNViewHeader::clear()
{
  instances.setAutoDelete(true);
  instances.clear();
  instances.setAutoDelete(false);
}



void KNViewHeader::up(KNViewHeader *h)
{
  int idx=instances.findRef(h);
  if(idx!=-1) {
    instances.take(idx);
    instances.insert(idx-1, h);
    kdDebug(5003) << "KNViewHeader::up() : item moved up" << endl;
  }
  else kdDebug(5003) << "KNViewHeader::up() : item not found in list" << endl;
}



void KNViewHeader::down(KNViewHeader *h)
{
  int idx=instances.findRef(h);
  if(idx!=-1) {
    instances.take(idx);
    instances.insert(idx+1, h);
    kdDebug(5003) << "KNViewHeader::up() : item moved down" << endl;
  }
  else kdDebug(5003) << "KNViewHeader::up() : item not found in list" << endl;
}



// some common headers
const char** KNViewHeader::predefs()
{
  return predef;
}



// *trys* to translate the name
QString KNViewHeader::translatedName()
{
  if (translateName) {
    // major hack alert !!!
    if (!n_ame.isEmpty()) {
      if (i18n("it's not very important to translate this",n_ame.local8Bit())!=n_ame.local8Bit().data())    // try to guess if this english or not
        return i18n("it's not very important to translate this",n_ame.local8Bit());
      else
        return n_ame;
    } else
      return QString::null;
  } else
    return n_ame;
}



// *trys* to retranslate the name to english
void KNViewHeader::setTranslatedName(const QString &s)
{
  bool retranslated = false;
  for (const char **c=predef;(*c)!=0;c++) {  // ok, first the standard header names
    if (s==i18n("it's not very important to translate this",*c)) {
      n_ame = QString::fromLatin1(*c);
      retranslated = true;
      break;
    }
  }

  if (!retranslated) {
    for (const char **c=disp;(*c)!=0;c++)   // now our standard display names
      if (s==i18n("it's not very important to translate this",*c)) {
        n_ame = QString::fromLatin1(*c);
        retranslated = true;
        break;
      }
  }

  if (!retranslated) {      // ok, we give up and store the maybe non-english string
    n_ame = s;
    translateName = false;  // and don't try to translate it, so a german user *can* use the original english name
  } else
    translateName = true;
}



void  KNViewHeader::createTags()
{
  const char *tokens[] = {  "<large>","</large>","<b>","</b>",
                            "<i>","</i>","<u>","</u>" };
  
  for(int i=0; i<4; i++) tags[i]=QString::null;
  
  if(flags.at(0)) {    // <font>
    tags[0]=tokens[0];
    tags[1]=tokens[1];
  }
  if(flags.at(4)) {
    tags[2]=tokens[0];
    tags[3]=tokens[1];
  }

  if(flags.at(1)) {     // <b>
    tags[0]+=(tokens[2]);
    tags[1].prepend(tokens[3]);
  }
  if(flags.at(5)) {
    tags[2]+=tokens[2];
    tags[3].prepend(tokens[3]);
  } 
  
  if(flags.at(2)) {     // <i>
    tags[0]+=tokens[4];
    tags[1].prepend(tokens[5]);
  }
  if(flags.at(6)) {
    tags[2]+=tokens[4];
    tags[3].prepend(tokens[5]);
  }
  
  if(flags.at(3)) {    // <u>
    tags[0]+=tokens[6];
    tags[1].prepend(tokens[7]);
  }
  if(flags.at(7)) {
    tags[2]+=tokens[6];
    tags[3].prepend(tokens[7]);
  }
}


