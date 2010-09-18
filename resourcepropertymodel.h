/*
   Copyright (C) 2008 by Sebastian Trueg <trueg at kde.org>

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

#ifndef _NEPOMUK_RESOURCE_PROPERTY_EDIT_MODEL_H_
#define _NEPOMUK_RESOURCE_PROPERTY_EDIT_MODEL_H_

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>

#include <Soprano/Statement>


class QUrl;

namespace Nepomuk {
    class Resource;
    namespace Types {
        class Class;
    }

    class ResourcePropertyEditModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        ResourcePropertyEditModel( QObject* parent = 0 );
        ~ResourcePropertyEditModel();

        /**
         * \return The configured resource or an invalid one if
         * no resource has been set.
         */
        Resource resource() const;

        int columnCount( const QModelIndex& parent = QModelIndex() ) const;
        int rowCount( const QModelIndex& parent = QModelIndex() ) const;
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        QModelIndex parent( const QModelIndex& index ) const;
        Qt::ItemFlags flags( const QModelIndex& index ) const;
        QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

        bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );
        bool setData( const QModelIndex& index, const QVariant& value, int role );

    public Q_SLOTS:
        /**
         * Se the resource to edit.
         */
        void setResource( const Resource& resource );

    private:
        class Private;
        Private* const d;
    };
}

#endif
