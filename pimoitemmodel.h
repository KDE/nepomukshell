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

#ifndef _NEPOMUK_PIMO_ITEM_MODEL_H_
#define _NEPOMUK_PIMO_ITEM_MODEL_H_

#include <QtCore/QAbstractItemModel>
#include <nepomuk/nepomuk_export.h>

#include <nepomuk/class.h>

Q_DECLARE_METATYPE( Nepomuk::Types::Class )


namespace Nepomuk {
    namespace Types {
        class Class;
    }

    // FIXME: rename this class
    // FIXME: use canFetchMore for faster listing
    class NEPOMUK_EXPORT PIMOItemModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        PIMOItemModel( QObject* parent = 0 );
        ~PIMOItemModel();

        enum Roles {
            TypeRole = 7777
        };

        /**
         * Set the parent class that will become the root of the
         * type hierarchy.
         */
        void setParentClass( const Types::Class& type );

        /**
         * Nepomuk::Types does not support automatic updates. Thus,
         * we have to be informed about a change through this method.
         */
        void updateClass( Types::Class type );

        int columnCount( const QModelIndex& parent = QModelIndex() ) const; 
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
        QModelIndex parent( const QModelIndex& index ) const;
        int rowCount( const QModelIndex& parent = QModelIndex() ) const;
        Qt::ItemFlags flags( const QModelIndex& index ) const;

        Qt::DropActions supportedDropActions() const;
        QMimeData* mimeData( const QModelIndexList& indexes ) const;
        bool dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent );
        QStringList mimeTypes() const;

        Types::Class classForIndex( const QModelIndex& index ) const;

    private:
        class Private;
        Private* const d;
    };
}

#endif
