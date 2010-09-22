/*
   Copyright (c) 2010 Sebastian Trueg <trueg@kde.org>

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

#include "resourcequerywidget.h"
#include "querymodel.h"
#include "sparqlsyntaxhighlighter.h"

#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QFont>

#include <KIcon>
#include <KConfigGroup>
#include <KGlobalSettings>

#include <Soprano/Node>

#include <Nepomuk/Resource>


ResourceQueryWidget::ResourceQueryWidget( QWidget* parent )
    : QWidget( parent ),
      m_queryHistoryIndex( 0 )
{
    setupUi( this );

    m_buttonBack->setIcon( KIcon("go-previous") );
    m_buttonForward->setIcon( KIcon("go-next") );

    m_queryEdit->setFont( KGlobalSettings::fixedFont() );
    m_queryEdit->setAcceptRichText( true );
    // we install an event filter to catch Ctrl+Return for quick query execution
    m_queryEdit->installEventFilter( this );

    m_queryModel = new Nepomuk::QueryModel( this );
    m_queryView->setModel( m_queryModel );

    connect(m_queryButton, SIGNAL(clicked()),
            this, SLOT(slotQueryButtonClicked()) );
    connect(m_buttonBack, SIGNAL(clicked()),
            this, SLOT(slotQueryHistoryPrevious()) );
    connect(m_buttonForward, SIGNAL(clicked()),
            this, SLOT(slotQueryHistoryNext()) );
    connect( m_queryView, SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(slotNodeActivated(QModelIndex)) );

    m_buttonForward->setEnabled( false );
    m_buttonBack->setEnabled( false );

    // the empty string representing the current query
    m_queryHistory << QString();

    m_highlighter = new Nepomuk::SparqlSyntaxHighlighter( m_queryEdit );
}


ResourceQueryWidget::~ResourceQueryWidget()
{
}


QStringList ResourceQueryWidget::queryHistory() const
{
    // do not return the non-executed query, ie. the last element in the list
    return m_queryHistory.mid( 0, m_queryHistory.count()-1 );
}


void ResourceQueryWidget::readSettings( const KConfigGroup& cfg )
{
    m_queryHistory = cfg.readEntry( "query history", QStringList() );

    // the empty string representing the current query
    m_queryHistory << QString();
    m_queryHistoryIndex = m_queryHistory.count()-1;

    updateHistoryButtonStates();
}


void ResourceQueryWidget::saveSettings( KConfigGroup& cfg ) const
{
    // save the last 20 queries
    QStringList history = queryHistory();
    if( history.count() > 20 )
        history = history.mid( history.count()-20 );

    cfg.writeEntry( "query history", history );
}


void ResourceQueryWidget::slotQueryButtonClicked()
{
    const QString query = m_queryEdit->toPlainText();
    if( m_queryHistory.count() == 1 || m_queryHistory[m_queryHistory.count()-2] != query ) {
        m_queryHistoryIndex = m_queryHistory.count();
        m_queryHistory.insert(m_queryHistory.count()-1, m_queryEdit->toPlainText() );
    }
    m_queryModel->setQuery( query );
    updateHistoryButtonStates();
}


void ResourceQueryWidget::slotNodeActivated( const QModelIndex& index )
{
    Soprano::Node node = m_queryModel->nodeForIndex( index );
    if ( node.isValid() && node.isResource() ) {
        Nepomuk::Resource res( node.uri() );
        if( res.exists() ) {
            emit resourceActivated( res );
        }
    }
}


void ResourceQueryWidget::slotQueryHistoryPrevious()
{
    if( m_queryHistoryIndex > 0 ) {
        if( m_queryHistoryIndex == m_queryHistory.count()-1 ) {
            m_queryHistory[m_queryHistoryIndex] = m_queryEdit->toPlainText();
        }

        --m_queryHistoryIndex;
        m_queryEdit->setPlainText( m_queryHistory[m_queryHistoryIndex] );

        updateHistoryButtonStates();
    }
}


void ResourceQueryWidget::slotQueryHistoryNext()
{
    if( m_queryHistoryIndex+1 < m_queryHistory.count() ) {
        ++m_queryHistoryIndex;
        m_queryEdit->setPlainText( m_queryHistory[m_queryHistoryIndex] );
        updateHistoryButtonStates();
    }
}


void ResourceQueryWidget::updateHistoryButtonStates()
{
    m_buttonForward->setEnabled( m_queryHistoryIndex < m_queryHistory.count()-1 );
    m_buttonBack->setEnabled( m_queryHistoryIndex > 0 );
}


bool ResourceQueryWidget::eventFilter( QObject* watched, QEvent* event )
{
    if( watched == m_queryEdit &&
        event->type() == QEvent::KeyPress ) {
        QKeyEvent* kev = static_cast<QKeyEvent*>(event);
        if( kev->key() == Qt::Key_Return &&
            kev->modifiers() == Qt::ControlModifier ) {
            slotQueryButtonClicked();
            return true;
        }
    }

    return QWidget::eventFilter( watched, event );
}

#include "resourcequerywidget.moc"
