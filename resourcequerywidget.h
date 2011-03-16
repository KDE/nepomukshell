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

#ifndef _NEPOMUK_RESOURCE_QUERY_WIDGET_H_
#define _NEPOMUK_RESOURCE_QUERY_WIDGET_H_

#include <QtGui/QWidget>
#include <QtCore/QTime>

#include <Nepomuk/Resource>
#include <Nepomuk/Types/Class>

#include <Soprano/Error/Error>

#include "ui_resourcequerywidget.h"

class KConfigGroup;
class QEvent;
namespace Nepomuk {
    class QueryModel;
}

class ResourceQueryWidget : public QWidget, private Ui::ResourceQueryWidget
{
    Q_OBJECT

public:
    ResourceQueryWidget( QWidget* parent = 0 );
    ~ResourceQueryWidget();

    QStringList queryHistory() const;

    void readSettings( const KConfigGroup& cfg );
    void saveSettings( KConfigGroup& cfg ) const;

    bool eventFilter( QObject* watched, QEvent* event );

Q_SIGNALS:
    void resourceActivated( const Nepomuk::Resource& res );

private Q_SLOTS:
    void slotQueryButtonClicked();
    void slotQueryStopButtonClicked();
    void slotNodeActivated( const QModelIndex& index );
    void slotQueryHistoryPrevious();
    void slotQueryHistoryNext();
    void slotQueryError( const Soprano::Error::Error & error );
    void slotQueryFinished();
    void slotQueryShortenButtonClicked();

private:
    void updateHistoryButtonStates();

    Nepomuk::QueryModel* m_queryModel;
    
    QStringList m_queryHistory;
    int m_queryHistoryIndex;
};

#endif
