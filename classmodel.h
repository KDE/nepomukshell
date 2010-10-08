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

#ifndef _NEPOMUK_CLASS_MODEL_H_
#define _NEPOMUK_CLASS_MODEL_H_

#include <QtCore/QAbstractItemModel>

namespace Nepomuk {
    namespace Types {
        class Class;
    }

    // FIXME: use canFetchMore for faster listing
    class ClassModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        ClassModel( QObject* parent = 0 );
        ~ClassModel();

        // FIXME: add method
        // /**
        //  * If set to \p true "ugly" types such as ProcessConcept are not shown
        //  */
        // void setFilterCorePimoTypes( bool );

        enum Roles {
            /**
             * FIXME: implement me!
             * A pretty name for the class. The base class Thing for example
             * would have a name like "All things".
             */
            PrettyName = 797897,
            TypeRole = 7777
        };

        /**
         * Set the parent class that will become the root of the
         * type hierarchy.
         */
        void setParentClass( const Types::Class& type );

        /**
         * \sa setParentClass
         */
        Nepomuk::Types::Class parentClass() const;

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

        /**
         * Method for getting the QModelIndex corresponding to the node
         * with the specified Nepomuk::Types::Class.
         * This will expand the type tree until it gets the node; if that
         * node does not exist, the return value will be an empty index.
         *
         * @param cls a class of which to get the node
         * @return the corresponding QModelIndex, empty if the class type is not found
         */
        QModelIndex indexForClass( const Types::Class& cls ) const;

        /**
         * This function returns a list of QModelIndexes, in which each
         * element is a parent node of the one containing the passed class
         * type.
         * This list may be empty if the root node of the tree has the
         * parameter itself or if the class type is not found.
         *
         * @param cls the class type to be searched for
         * @return a list of all indexes for parent nodes of cls
         */
        QModelIndexList parentIndexList( const Types::Class& cls );

    private:
        class Private;
        Private* const d;
    };
}

#endif
