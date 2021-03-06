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

#ifndef _NEPOMUK_QUERY_MODEL_H_
#define _NEPOMUK_QUERY_MODEL_H_

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>

#include <Soprano/Node>
#include <Soprano/Error/ErrorCode>
#include <Soprano/Util/AsyncQuery>

namespace Nepomuk2 {

    class QueryModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        QueryModel( QObject* parent = 0 );
        ~QueryModel();

        int columnCount( const QModelIndex& parent = QModelIndex() ) const;
        int rowCount( const QModelIndex& parent = QModelIndex() ) const;
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        QModelIndex parent( const QModelIndex& index ) const;
        Qt::ItemFlags flags( const QModelIndex& index ) const;
        QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

        Soprano::Node nodeForIndex( const QModelIndex& index ) const;

        int queryTime() const;
    Q_SIGNALS:
        void queryError( const Soprano::Error::Error & error ); 
        void queryFinished();

    public Q_SLOTS:
        void setQuery( const QString& query );
        void stopQuery();

    private Q_SLOTS:
        void slotNextResultReady( Soprano::Util::AsyncQuery* query );
        void slotQueryFinished( Soprano::Util::AsyncQuery* );
        
    private:
        class Private;
        Private* const d;
    };
}

#endif
