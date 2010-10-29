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

#include "nepomukextras_export.h"

namespace Nepomuk {
    namespace Types {
        class Class;
    }

    namespace Utils {
        /**
         * \class ClassModel classmodel.h Nepomuk/Utils/ClassModel
         *
         * \brief A model which shows a %Nepomuk type hirarchy in a tree.
         *
         * Creating a %ClassModel is as simple as instanciating it and
         * calling addRootClass() to populate it.
         *
         * In addition to listing the type hierarchy ClassModel also supports
         * drag'n'drop allowing to create new rdfs:subclass relationsships
         * for user created types.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class NEPOMUKEXTRAS_EXPORT ClassModel : public QAbstractItemModel
        {
            Q_OBJECT

        public:
            /**
             * Constructor. Creates an empty model.
             */
            ClassModel( QObject* parent = 0 );

            /**
             * Destructor.
             */
            ~ClassModel();

            /**
             * Custom roles supported by this model in addition
             * to the standard Qt::DataItemRole types.
             */
            enum CustomRoles {
                /**
                 * The type the node represents, provided as a
                 * Nepomuk::Types::Class instance.
                 */
                TypeRole = 7777
            };

            /**
             * \return The list of root classes as added via addRootClass() or
             * setRootClasses().
             */
            QList<Nepomuk::Types::Class> rootClasses() const;

            /**
             * Matches a model index to the corresponding resource type.
             * \return The type represented by \p index or an invalid Types::Class
             * instance if \p index is invalid.
             */
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

            int columnCount( const QModelIndex& parent = QModelIndex() ) const;
            QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
            QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
            QModelIndex parent( const QModelIndex& index ) const;

            /**
             * Reimplemented to support lazy loading of subclasses.
             */
            bool hasChildren( const QModelIndex& parent = QModelIndex() ) const;

            /**
             * \returns The number of subclasses of the type corresponding to the cell
             * identified by \p index.
             */
            int rowCount( const QModelIndex& parent = QModelIndex() ) const;

            /**
             * Reimplemented to support drag'n'drop of types in order to create new rdfs:subClassOf
             * relations.
             */
            Qt::ItemFlags flags( const QModelIndex& index ) const;

            /**
             * Reimplemented to support drag'n'drop of types in order to create new rdfs:subClassOf
             * relations.
             */
            Qt::DropActions supportedDropActions() const;

            /**
             * Reimplemented to support drag'n'drop of types in order to create new rdfs:subClassOf
             * relations.
             */
            QMimeData* mimeData( const QModelIndexList& indexes ) const;

            /**
             * Reimplemented to support drag'n'drop of types in order to create new rdfs:subClassOf
             * relations.
             */
            bool dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent );

            /**
             * Reimplemented to support drag'n'drop of types in order to create new rdfs:subClassOf
             * relations.
             */
            QStringList mimeTypes() const;

        public Q_SLOTS:
            /**
             * Add a root class to the model. This will add a new branch
             * which contains the full type hirarchy under \p type.
             *
             * \sa setRootClasses(), clear()
             */
            void addRootClass( const Types::Class& type );

            /**
             * Set a root class to the model. This will add a new branch
             * which contains the full type hirarchy under \p type.
             *
             * This is a convinience method which makes for simpler code
             * in case one only uses a single branch.
             *
             * \sa addRootClass(), setRootClasses(), clear()
             */
            void setRootClass( const Types::Class& type );

            /**
             * Set the root classes. Each class will add a new branch to
             * the model.
             *
             * \sa addRootClass(), clear()
             */
            void setRootClasses( const QList<Types::Class>& types );

            /**
             * Clear the model, remove all root types.
             */
            void clear();

            /**
             * For performance reasons Nepomuk::Types does not support automatic updates. Thus,
             * we have to be informed about a change through this method.
             */
            void updateClass( const Types::Class& type );

        private:
            bool canFetchMore( const QModelIndex& parent ) const;
            void fetchMore( const QModelIndex& parent );

            class ClassNode;
            class Private;
            Private* const d;
        };
    }
}

#endif
