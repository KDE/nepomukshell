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

#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>

#include <Soprano/Node>

#include <Nepomuk/Resource>


ResourceQueryWidget::ResourceQueryWidget( QWidget* parent )
    : QWidget( parent )
{
    setupUi( this );
    m_queryModel = new Nepomuk::QueryModel( this );
    m_queryView->setModel( m_queryModel );

    connect(m_queryButton, SIGNAL(clicked()),
            this, SLOT(slotQueryButtonClicked()) );
    connect( m_queryView, SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(slotNodeActivated(QModelIndex)) );
}


ResourceQueryWidget::~ResourceQueryWidget()
{
}


void ResourceQueryWidget::slotQueryButtonClicked()
{
    m_queryModel->setQuery( m_queryEdit->toPlainText() );
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

#include "resourcequerywidget.moc"
