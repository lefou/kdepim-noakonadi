/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>

#include <konnectormanager.h>
#include <configwidget.h>

#include "konnectorwizard.h"

#include "konnectorprofilewizardintro.h"
#include "konnectorwizardoutro.h"

using namespace KSync;

namespace {

const int AREA = 5210;

void setCurrent( const QString& str, QComboBox* box, bool insert = true )
{
    if ( str.isEmpty() ) return;
    uint b = box->count();
    for ( uint i = 0; i < b; i++ ) {
        if ( box->text(i) == str ) {
            box->setCurrentItem( i );
            return;
        }
    }
    if ( !insert ) return;

    box->insertItem( str );
    box->setCurrentItem( b );
}

}


KonnectorWizard::KonnectorWizard( KonnectorManager *manager )
    : KWizard( 0, "wizard", true ), m_manager( manager )
{
    m_isEdit = false;
    m_conf = 0;
    initUI();
}

KonnectorWizard::KonnectorWizard( KonnectorManager* manager,
                                  const KonnectorProfile &prof )
    : KWizard( 0, "wizard", true ), m_manager( manager)
{
    m_isEdit = true;
    m_kaps = prof.kapabilities();
    m_conf = 0;
    initUI();

    /* init the ui */
    m_outro->lneName->setText( prof.name() );
    kdDebug(AREA) << "Current Identify is " << prof.device().name() << " " << prof.device().identify() << endl;
    setCurrent( prof.device().name(), m_intro->cmbDevice, false );
    kdDebug(AREA) << "Current entry is now  " << m_intro->cmbDevice->currentText() << endl;
    slotKonChanged( m_intro->cmbDevice->currentText() );
}

KonnectorWizard::~KonnectorWizard()
{
}

KonnectorProfile KonnectorWizard::profile() const
{
    KonnectorProfile prof;

    if ( m_conf ) {
        prof.setKapabilities( m_conf->capabilities() );
    }

    prof.setDevice( byString( m_intro->cmbDevice->currentText() ) );
    prof.setName( m_outro->lneName->text() );

    return prof;
}

/*
 * let's add some pages
 * and make the Configure widget be kewl
 * basicly we need to recreate it on
 *
 */
void KonnectorWizard::initUI()
{
    m_conf = 0;
    m_intro = new KonnectorProfileWizardIntro();
    m_outro = new KonnectorWizardOutro();
    addPage( m_intro, "Profile");
    addPage( m_outro, "Outro");
    setFinishEnabled( m_outro, true );

    Device::ValueList list = m_manager->query();
    Device::ValueList::Iterator it;
    m_current =i18n("Please choose a Konnector");
    m_intro->cmbDevice->insertItem( m_current  );
    for (it = list.begin(); it != list.end(); ++it ) {
	kdDebug(AREA) << "Inserting into Combo " << (*it).name() << " " << (*it).identify() << endl;
        m_intro->cmbDevice->insertItem( (*it).name() );
        m_devices.insert( (*it).name(), (*it) );
    }
    // connect to textchanges
    connect(m_intro->cmbDevice, SIGNAL(activated(const QString&) ),
            SLOT(slotKonChanged(const QString&) ) );
}

/*
 * If the Device Combobox changed we need to update the
 * Configuration Tab
 * First check tif the selection changed
 * Then check if the selection changed to the Kapabilities
 * giving as parameter
 * then recreate the widget
 */
void KonnectorWizard::slotKonChanged( const QString& str)
{
    if ( str == m_current )  // the selection was not changed
        return;
    if ( str == i18n("Please choose a Konnector") ) {
        kdDebug(5210) << "Connector " << endl;
        delete m_conf;
        m_conf =0;
        return;
    }
    m_current = str;

    delete m_conf;
    m_conf = 0;

    Device dev = byString( str );

    // load the Konnector for getting a ConfigureWidget
    Konnector *k = m_manager->load( dev );
    if ( !k ) return;

    if( !m_isEdit )
        m_conf = m_manager->configWidget( k, this, "config" ); // never 0
    else
        m_conf = m_manager->configWidget( k, m_kaps, this, "config" );

    insertPage(m_conf, i18n("Configure"), 1 );
    m_manager->unload( k );
}

Device KonnectorWizard::byString( const QString& str ) const
{
    return m_devices[str];
}


#include "konnectorwizard.moc"
