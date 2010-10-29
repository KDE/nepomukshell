/*
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _NEPOMUK_PIMO_MODEL_H_
#define _NEPOMUK_PIMO_MODEL_H_

#include <Soprano/RdfSchemaModel>

#include "nepomukextras_export.h"

namespace Nepomuk {
    /**
     * \class PimoModel pimomodel.h pimomodel.h
     *
     * \author Sebastian Trueg <trueg@kde.org>
     *
     * \sa Vocabulary::PIMO
     */
    class NEPOMUKEXTRAS_EXPORT PimoModel : public Soprano::RdfSchemaModel
    {
        Q_OBJECT

    public:
        /**
         * Create a new PIMO model.
         */
        PimoModel( Soprano::Model* parentModel = 0 );

        /**
         * Destructor
         */
        ~PimoModel();

//        Soprano::StatementIterator allThings( const QUrl& parent = QUrl() ) const;

        /**
         * The URI of the user's personal information model.
         * Refers to the context (named graph) that contains all the personal
         * information created by the user (FIXME: Well, there are also resources
         * that are created by the system without the user noticing, is that ok?)
         *
         * \return The URI of the context that contains all pimo information.
         * This context is used by all creation methods in this class.
         */
        QUrl pimoContext();

        /**
         * Create new new RDF class.
         * \param parentClass The parent class to use
         * \param label The new classes label
         * \param comment An optional comment for the class, ie. a more detailed
         *        description.
         * \icon An optional icon for the new class.
         *
         * \return The URI of the newly created class or an invalid URI on error.
         * In that case see ErrorCache::lastError() for details.
         */
        QUrl createClass( const QUrl& parentClass,
                          const QString& label,
                          const QString& comment = QString(),
                          const QString& icon = QString() );

        /**
         * Create new new RDF property.
         * \param domain The domain of the new property
         * \param range The range of the property
         * \param label The new property's label
         * \param comment An optional comment for the property, ie. a more detailed
         *        description.
         * \icon An optional icon for the new class.
         *
         * \return The URI of the newly created property or an invalid URI on error.
         * In that case see ErrorCache::lastError() for details.
         */
        QUrl createProperty ( const QUrl& domain,
                              const QUrl& range,
                              const QString label,
                              const QString& comment = QString(),
                              const QString& icon = QString() );

        /**
         * Create new new RDF resource.
         * \param type The type of the new resource
         * \param label The new resource's label
         * \param comment An optional comment for the resource, ie. a more detailed
         *        description.
         * \icon An optional icon for the new resource
         *
         * \return The URI of the newly created resource or an invalid URI on error.
         * In that case see ErrorCache::lastError() for details.
         */
        QUrl createThing( const QUrl& type,
                          const QString& label,
                          const QString& comment = QString(),
                          const QString& icon = QString() );

        /**
         * Create a rdfs:subClassOf relation.
         *
         * \param classUri The URI of the class that should get a new parent class.
         * \param newParentClassUri The URI of the class that should become the new parent of classUri.
         * \param singleParent If \p true any existing subClassOf relations are deleted, otherwise the
         * new relation is added.
         *
         * \return \p true if the relation could be created successfully, \p false otherwise.
         */
        bool createSubClassRelation( const QUrl& classUri, const QUrl& newParentClassUri, bool singleParent = false );

        /**
         * Create a new class URI. This is used internally by methods such as
         * createClass. Can be overridden to create custom URIs.
         * The default implementation uses the baseNamespace in combination
         * with a random string URI fragment.
         *
         * \param name The name of the class, i.e. the rdf:label. This can
         *             be used to make the URI more user readable.
         *
         * \return A new unique URI to be used as a new named class.
         */
        virtual QUrl newClassUri( const QString& name = QString() );

        /**
         * Create a new property URI. This is used internally by methods such as
         * createProperty. Can be overridden to create custom URIs.
         * The default implementation uses the baseNamespace in combination
         * with a random string URI fragment.
         *
         * \param name The name of the property, i.e. the rdf:label. This can
         *             be used to make the URI more user readable.
         *
         * \return A new unique URI to be used as a new named property.
         */
        virtual QUrl newPropertyUri( const QString& name = QString() );

        /**
         * Adds a list of statements to the pimo context. Used internally by
         * createClass, createProperty, and createThing.
         *
         * \sa pimoContext
         */
        virtual Soprano::Error::ErrorCode addPimoStatements( const QList<Soprano::Statement>& );

//        QUrl findOrCreateThing( const QString& label, const QUrl& type = QUrl() );

    private:
        class Private;
        Private* const d;
    };
}

#endif
