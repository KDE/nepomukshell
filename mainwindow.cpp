/*
   Copyright (C) 2008-2009 by Sebastian Trueg <trueg at kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mainwindow.h"
#include "pimoitemmodel.h"
#include "pimo.h"
#include "resourcepropertymodel.h"
#include "resourcepropertydelegate.h"
#include "nepomukcontext.h"
#include "resourceeditor.h"
#include "resourceview.h"
#include "resourcemodel.h"
#include "resourcebrowserwidget.h"
#include "resourceeditorwidget.h"
#include "resourcequerywidget.h"

#include <QtGui/QStackedWidget>

#include <nepomuk/class.h>
#include <nepomuk/property.h>
#include <nepomuk/resourcemanager.h>

#include <KIcon>
#include <KDebug>
#include <KAction>
#include <KStandardAction>
#include <KActionCollection>
#include <KApplication>
#include <KToggleAction>
#include <KMessageBox>

#include <Soprano/Vocabulary/RDFS>


MainWindow::MainWindow()
    : KXmlGuiWindow( 0 )
{
    m_mainStack = new QStackedWidget( this );
    setCentralWidget( m_mainStack );

    m_resourceBrowser = new ResourceBrowserWidget( actionCollection(), m_mainStack );
    m_mainStack->addWidget( m_resourceBrowser );

    m_resourceQueryWidget = new ResourceQueryWidget( m_mainStack );
    m_mainStack->addWidget( m_resourceQueryWidget );

    m_resourceEditor = new ResourceEditorWidget( m_mainStack );
    m_mainStack->addWidget( m_resourceEditor );

    setupActions();

    connect( m_resourceBrowser, SIGNAL(resourcesSelected(QList<Nepomuk::Resource>)),
             this, SLOT(slotResourcesSelected(QList<Nepomuk::Resource>)) );

    connect( m_resourceBrowser, SIGNAL(resourceActivated(Nepomuk::Resource)),
             this, SLOT(slotResourceActivated(Nepomuk::Resource)) );
    connect( m_resourceQueryWidget, SIGNAL(resourceActivated(Nepomuk::Resource)),
             this, SLOT(slotResourceActivated(Nepomuk::Resource)) );
    connect( m_resourceEditor, SIGNAL(resourceActivated(Nepomuk::Resource)),
             this, SLOT(slotResourceActivated(Nepomuk::Resource)) );

    // init
    m_actionModeBrowse->setChecked( true );
    m_actionDelete->setEnabled( false );
    slotResourcesSelected( QList<Nepomuk::Resource>() );
}


MainWindow::~MainWindow()
{
}


QList<Nepomuk::Resource> MainWindow::selectedResources() const
{
    if( m_mainStack->currentWidget() == m_resourceBrowser )
        return m_resourceBrowser->selectedResources();
    else if( m_mainStack->currentWidget() == m_resourceEditor )
        return QList<Nepomuk::Resource>() << m_resourceEditor->resource();
    else
        return QList<Nepomuk::Resource>();
}


void MainWindow::setupActions()
{
    // ontology handling actions
    // =============================================
    m_actionNewSubClass = new KAction( KIcon( "document-new" ), i18n( "Create Subclass" ), actionCollection() );
    m_actionNewProperty = new KAction( KIcon( "document-new" ), i18n( "Create Property" ), actionCollection() );
    m_actionNewResource = new KAction( KIcon( "document-new" ), i18n( "Create Resource" ), actionCollection() );

    connect( m_actionNewSubClass, SIGNAL(triggered(bool)), m_resourceBrowser, SLOT(createClass()) );
    connect( m_actionNewProperty, SIGNAL(triggered(bool)), m_resourceBrowser, SLOT(createProperty()) );
    connect( m_actionNewResource, SIGNAL(triggered(bool)), m_resourceBrowser, SLOT(createResource()) );

    actionCollection()->addAction( "create_class", m_actionNewSubClass );
    actionCollection()->addAction( "create_property", m_actionNewProperty );
    actionCollection()->addAction( "create_resource", m_actionNewResource );


    // mode actions
    // =============================================
    m_actionModeBrowse = new KToggleAction( actionCollection() );
    m_actionModeBrowse->setText( i18n("Browse Resources") );
    m_actionModeBrowse->setIcon( KIcon("FIXME") );
    actionCollection()->addAction( "mode_browse", m_actionModeBrowse );
    connect( m_actionModeBrowse, SIGNAL(triggered()), this, SLOT(slotModeBrowse()) );

    m_actionModeQuery = new KToggleAction( actionCollection() );
    m_actionModeQuery->setText( i18n("Query Resources") );
    m_actionModeQuery->setIcon( KIcon("FIXME") );
    actionCollection()->addAction( "mode_query", m_actionModeQuery );
    connect( m_actionModeQuery, SIGNAL(triggered()), this, SLOT(slotModeQuery()) );

    m_actionModeEdit = new KToggleAction( actionCollection() );
    m_actionModeEdit->setText( i18n("Edit Resource") );
    m_actionModeEdit->setIcon( KIcon("FIXME") );
    actionCollection()->addAction( "mode_edit", m_actionModeEdit );
    connect( m_actionModeEdit, SIGNAL(triggered()), this, SLOT(slotModeEdit()) );

    QActionGroup* modeGroup = new QActionGroup( this );
    modeGroup->setExclusive( true );
    modeGroup->addAction( m_actionModeBrowse );
    modeGroup->addAction( m_actionModeQuery );
    modeGroup->addAction( m_actionModeEdit );


    // resource actions
    // =============================================
    m_actionDelete = new KAction( actionCollection() );
    m_actionDelete->setText( i18n("Delete Resource") );
    m_actionDelete->setIcon( KIcon("edit-delete") );
    actionCollection()->addAction( "resource_delete", m_actionDelete );
    connect( m_actionDelete, SIGNAL(triggered()), this, SLOT(slotDeleteResource()) );

    // misc actions
    // =============================================
    KStandardAction::quit( kapp, SLOT( quit() ), actionCollection() );

    setupGUI();
}


void MainWindow::slotModeBrowse()
{
    kDebug();
    m_mainStack->setCurrentWidget( m_resourceBrowser );
    slotResourcesSelected( selectedResources() );
}


void MainWindow::slotModeQuery()
{
    kDebug();
    m_mainStack->setCurrentWidget( m_resourceQueryWidget );
    // in query mode we do not have a selected resource
    slotResourcesSelected( selectedResources() );
}


void MainWindow::slotModeEdit()
{
    kDebug();
    if( selectedResources().count() == 1 ) {
        m_resourceEditor->setResource( selectedResources().first() );
        m_mainStack->setCurrentWidget( m_resourceEditor );
    }
}


void MainWindow::slotResourcesSelected( const QList<Nepomuk::Resource>& res )
{
    m_actionModeEdit->setEnabled( res.count() == 1 );
    m_actionDelete->setEnabled( !res.isEmpty() );
}


void MainWindow::slotResourceActivated( const Nepomuk::Resource& res )
{
    kDebug() << res.resourceUri();
    m_resourceEditor->setResource( res );
    m_actionModeEdit->setChecked( true );
    m_mainStack->setCurrentWidget( m_resourceEditor );
    slotResourcesSelected( selectedResources() );
}


void MainWindow::slotDeleteResource()
{
    QList<Nepomuk::Resource> rl = selectedResources();
    if( rl.isEmpty() ) {
        KMessageBox::sorry( this, i18n("No Resource to delete selected.") );
    }
    else {
        Q_FOREACH( Nepomuk::Resource res, rl )
            res.remove();
    }
}

#include "mainwindow.moc"
