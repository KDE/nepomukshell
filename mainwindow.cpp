/*
   Copyright (C) 2008-2010 by Sebastian Trueg <trueg at kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "pimo.h"
#include "resourcepropertymodel.h"
#include "resourceview.h"
#include "resourcebrowserwidget.h"
#include "resourceeditorwidget.h"
#include "resourcequerywidget.h"
#include "nepomukshellsettings.h"
#include "resourcebrowsersettingspage.h"

#include <QtGui/QStackedWidget>
#include <QtGui/QTextBrowser>

#include <nepomuk/class.h>
#include <nepomuk/property.h>
#include <nepomuk/resourcemanager.h>
#include <nepomuk/resourcemodel.h>

#include <KIcon>
#include <KDebug>
#include <KAction>
#include <KStandardAction>
#include <KActionCollection>
#include <KApplication>
#include <KToggleAction>
#include <KMessageBox>
#include <KConfigDialog>
#include <KConfig>
#include <KGlobal>
#include <KFileDialog>

#include <Soprano/Vocabulary/RDFS>
#include <Soprano/StatementIterator>
#include <Soprano/Model>
#include <Soprano/Serializer>
#include <Soprano/PluginManager>
#define USING_SOPRANO_NRLMODEL_UNSTABLE_API
#include <Soprano/NRLModel>


MainWindow* MainWindow::s_self = 0;

MainWindow::MainWindow()
    : KXmlGuiWindow( 0 )
{
    s_self = this;

    setCaption( i18n("NepSaK - The Nepomuk Shell") );

    setCentralWidget( new QWidget( this ) );
    setupUi( centralWidget() );

    m_resourceBrowser = new ResourceBrowserWidget( m_mainStack );
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
    m_mainStack->setCurrentWidget( m_resourceBrowser );
    m_actionModeBrowse->setChecked( true );
    m_resourceActionGroup->setEnabled( false );
    slotResourcesSelected( QList<Nepomuk::Resource>() );

    readSettings();
}


MainWindow::~MainWindow()
{
}


void MainWindow::openResource( const Nepomuk::Resource& res )
{
    m_resourceEditor->setResource( res );
    m_actionModeEdit->setChecked( true );
    m_mainStack->setCurrentWidget( m_resourceEditor );
    slotResourcesSelected( selectedResources() );
}


void MainWindow::openResource( const KUrl& url )
{
    openResource( Nepomuk::Resource( url ) );
}


bool MainWindow::queryClose()
{
    saveSettings();
    return KXmlGuiWindow::queryClose();
}


QList<Nepomuk::Resource> MainWindow::selectedResources() const
{
    if( m_mainStack->currentWidget() == m_resourceEditor ) {
        QList<Nepomuk::Resource> rl;
        Nepomuk::Resource res = m_resourceEditor->resource();
        if( res.exists() )
            rl << res;
        return rl;
    }
    else {
        return m_resourceBrowser->selectedResources();
    }
}


void MainWindow::setupActions()
{
    // ontology handling actions
    // =============================================
    m_actionNewSubClass = new KAction( KIcon( QLatin1String( "document-new" ) ), i18nc( "@action:button create a new subclass to an existing RDF class", "Create Subclass" ), actionCollection() );
    m_actionNewProperty = new KAction( KIcon( QLatin1String( "document-new" ) ), i18nc( "@action:button create a new subproperty to an existing RDF property", "Create Property" ), actionCollection() );
    m_actionNewResource = new KAction( KIcon( QLatin1String( "document-new" ) ), i18nc( "@action:button create a new Nepomuk resource of any type", "Create Resource" ), actionCollection() );

    connect( m_actionNewSubClass, SIGNAL(triggered(bool)), m_resourceBrowser, SLOT(createClass()) );
    connect( m_actionNewProperty, SIGNAL(triggered(bool)), m_resourceBrowser, SLOT(createProperty()) );
    connect( m_actionNewResource, SIGNAL(triggered(bool)), m_resourceBrowser, SLOT(createResource()) );

    actionCollection()->addAction( QLatin1String( "create_class" ), m_actionNewSubClass );
    actionCollection()->addAction( QLatin1String( "create_property" ), m_actionNewProperty );
    actionCollection()->addAction( QLatin1String( "create_resource" ), m_actionNewResource );


    // mode actions
    // =============================================
    m_actionModeBrowse = new KToggleAction( actionCollection() );
    m_actionModeBrowse->setText( i18nc("@action:button", "Browse Resources") );
    m_actionModeBrowse->setIcon( KIcon( QLatin1String( "view-list-tree" ) ) );
    m_actionModeBrowse->setShortcut( Qt::ALT + Qt::Key_B );
    actionCollection()->addAction( QLatin1String( "mode_browse" ), m_actionModeBrowse );
    connect( m_actionModeBrowse, SIGNAL(triggered()), this, SLOT(slotModeBrowse()) );

    m_actionModeQuery = new KToggleAction( actionCollection() );
    m_actionModeQuery->setText( i18nc("@action:button", "Query Resources") );
    m_actionModeQuery->setIcon( KIcon( QLatin1String( "edit-find" ) ) );
    m_actionModeQuery->setShortcut( Qt::ALT + Qt::Key_Q );
    actionCollection()->addAction( QLatin1String( "mode_query" ), m_actionModeQuery );
    connect( m_actionModeQuery, SIGNAL(triggered()), this, SLOT(slotModeQuery()) );

    m_actionModeEdit = new KToggleAction( actionCollection() );
    m_actionModeEdit->setText( i18nc("@action:button", "Edit Resource") );
    m_actionModeEdit->setIcon( KIcon( QLatin1String( "edit-rename" ) ) );
    m_actionModeEdit->setShortcut( Qt::ALT + Qt::Key_E );
    actionCollection()->addAction( QLatin1String( "mode_edit" ), m_actionModeEdit );
    connect( m_actionModeEdit, SIGNAL(triggered()), this, SLOT(slotModeEdit()) );

    QActionGroup* modeGroup = new QActionGroup( this );
    modeGroup->setExclusive( true );
    modeGroup->addAction( m_actionModeBrowse );
    modeGroup->addAction( m_actionModeQuery );
    modeGroup->addAction( m_actionModeEdit );


    // resource actions
    // =============================================
    m_resourceActionGroup = new QActionGroup( actionCollection() );
    m_actionDelete = new KAction( actionCollection() );
    m_actionDelete->setText( i18nc("@action:button", "Delete Resource") );
    m_actionDelete->setIcon( KIcon( QLatin1String( "edit-delete" ) ) );
    actionCollection()->addAction( QLatin1String( "resource_delete" ), m_actionDelete );
    connect( m_actionDelete, SIGNAL(triggered()), this, SLOT(slotDeleteResource()) );
    m_resourceActionGroup->addAction( m_actionDelete );

    m_actionShowSource = new KAction( actionCollection() );
    m_actionShowSource->setText( i18nc("@action:button", "Show Serialized Resource") );
    actionCollection()->addAction( QLatin1String( "resource_show_source" ), m_actionShowSource );
    connect( m_actionShowSource, SIGNAL(triggered()), this, SLOT(slotShowSource()) );
    m_resourceActionGroup->addAction( m_actionShowSource );


    // misc actions
    // =============================================
    KStandardAction::preferences( this, SLOT( slotSettings() ), actionCollection() );
    KStandardAction::open( this, SLOT( slotOpen() ), actionCollection() );

    m_actionAutoIndentQuery= new KAction( i18nc( "@action:auto indents a query", "Auto Indent Query" ), actionCollection() );
    m_actionAutoIndentQuery->setShortcut(Qt::ALT + Qt::Key_A);
    connect( m_actionAutoIndentQuery, SIGNAL(triggered()), this, SLOT(slotAutoIndentQuery()) );
    actionCollection()->addAction( QLatin1String( "auto_indent" ), m_actionAutoIndentQuery );

    KStandardAction::quit( kapp, SLOT( quit() ), actionCollection() );

    setupGUI( Keys|Save|Create );
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
    m_resourceActionGroup->setEnabled( false );
}


void MainWindow::slotModeEdit()
{
    kDebug();
    if( !m_resourceEditor->resource().isValid() &&
        selectedResources().count() == 1 ) {
        m_resourceEditor->setResource( selectedResources().first() );
    }

    m_mainStack->setCurrentWidget( m_resourceEditor );
}


void MainWindow::slotResourcesSelected( const QList<Nepomuk::Resource>& res )
{
    m_actionModeEdit->setEnabled( res.count() == 1 || m_resourceEditor->resource().isValid() );
    m_resourceActionGroup->setEnabled( !res.isEmpty() );
}


void MainWindow::slotResourceActivated( const Nepomuk::Resource& res )
{
    kDebug() << res.resourceUri();
    openResource( res );
}


void MainWindow::slotDeleteResource()
{
    QList<Nepomuk::Resource> rl = selectedResources();
    if( rl.isEmpty() ) {
        KMessageBox::sorry( this, i18n("No Resource to delete selected.") );
    }
    else {
        QStringList resNames;
        Q_FOREACH( Nepomuk::Resource res, rl ) {
            resNames << res.genericLabel();
        }
        if( KMessageBox::questionYesNoList( this,
                                            i18n("Do you really want to delete these resources?"),
                                            resNames,
                                            i18n("Deleting Resources") ) == KMessageBox::Yes ) {
            Q_FOREACH( Nepomuk::Resource res, rl ) {
                res.remove();
            }
        }
    }
}


void MainWindow::slotSettings()
{
    if( KConfigDialog::showDialog( QLatin1String( "settings") ) )
        return;

    KConfigDialog* dialog = new KConfigDialog( this, QLatin1String( "settings" ), Settings::self() );
    dialog->setFaceType( KPageDialog::List );
    dialog->addPage( new ResourceBrowserSettingsPage(),
                     i18n("Resource Browser"),
                     QLatin1String("nepomuk"),
                     i18n("Configure the Resource Browser") );
    dialog->show();
}


void MainWindow::readSettings()
{
    kDebug();
    m_resourceQueryWidget->readSettings( KGlobal::config()->group("Query") );
}


void MainWindow::saveSettings()
{
    kDebug();
    KConfigGroup grp = KGlobal::config()->group("Query");
    m_resourceQueryWidget->saveSettings( grp );
}


void MainWindow::slotOpen()
{
    KUrl url = KFileDialog::getOpenUrl( KUrl(), QString(), this, i18n( "Open a file or Nepomuk resource to edit" ) );
    if ( !url.isEmpty() ) {
        openResource( url );
    }
}

void MainWindow::slotAutoIndentQuery()
{
    m_resourceQueryWidget->autoIndentQuery();
}

void MainWindow::slotShowSource()
{
    // TODO: create a dedicated dialog with buttons to change serialization and enable/disable bnames
    //       for the latter put the NRLModel trick from the QueryModel into a helper class
    Soprano::StatementIterator it = Nepomuk::ResourceManager::instance()->mainModel()->listStatements( selectedResources().first().resourceUri(),
                                                                                                       Soprano::Node(),
                                                                                                       Soprano::Node() );
    const Soprano::Serializer* serializer = Soprano::PluginManager::instance()->discoverSerializerForSerialization( Soprano::SerializationTurtle );
    if( !serializer ) {
        KMessageBox::error( this, i18n("Could not find a useful Soprano serializer plugin."));
        return;
    }

    // add query prefixes
    // TODO: put this into a helper class
    Soprano::NRLModel nrlModel( Nepomuk::ResourceManager::instance()->mainModel() );
    nrlModel.setEnableQueryPrefixExpansion( true );
    QHash<QString, QUrl> queryPrefixes = nrlModel.queryPrefixes();
    for( QHash<QString, QUrl>::const_iterator it = queryPrefixes.constBegin();
         it != queryPrefixes.constEnd(); ++it ) {
        serializer->addPrefix( it.key(), it.value() );
    }

    QString s;
    QTextStream stream( &s );
    serializer->serialize( it, stream, Soprano::SerializationTurtle );

    KDialog dlg(this);
    dlg.setButtons(KDialog::Ok);
    dlg.setCaption(i18n("Show Resource Source"));
    QTextBrowser* browser = new QTextBrowser(&dlg);
    browser->setPlainText(s);
    dlg.setMainWidget(browser);
    dlg.exec();
}


// static
MainWindow* MainWindow::nepomukShellMain()
{
    return s_self;
}

#include "mainwindow.moc"
