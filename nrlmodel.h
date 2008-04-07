/*
 * This file is part of Soprano Project.
 *
 * Copyright (C) 2007-2008 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _SOPRANO_NRL_MODEL_H_
#define _SOPRANO_NRL_MODEL_H_

#include <soprano/rdfschemamodel.h>
//#include "soprano_export.h"

namespace Soprano {
    /**
     * \class NRLModel nrlmodel.h Soprano/NRLModel
     *
     * \brief Model filter that enforces NRL cardinality rules.
     *
     * The NRLModel enforces NRL cardinality rules. That means predicates with a cardinality
     * of maximum 1 are always udpated while statements that define predicates with a maximum
     * cardinality bigger than 1 are rejected once the maximum is reached (future versions 
     * might remove an earlier defined statement based on the time the old statements were
     * added).
     *
     * Thus, at the moment NRLModel is mostly usable for handling properties with a maximum
     * cardinality of 1.
     *
     * \author Sebastian Trueg <trueg@kde.org>
     *
     * \sa Vocabulary::NRL
     */
    class SOPRANO_EXPORT NRLModel : public RdfSchemaModel
    {
    public:
        /**
         * Create a new %NRLModel
         */
        NRLModel( Model* parent = 0 );
        
        /**
         * Destructor
         */
        ~NRLModel();

        /**
         * When enforcing the NRL cardinality rules NRLModel can either
         * ignore the context of statements or treat different contexts
         * as separate sets, each resetting the cardinality.
         *
         * \param b If \p true (the default) NRLModel does ignore the context
         * when enforcing rules. If \p false the NRL rules can be violated
         * across contexts.
         *
         * \sa ignoreContext()
         */
        void setIgnoreContext( bool b );

        /**
         * \return \p true if contexts should be ignored when enforcing NRL
         * rules.
         *
         * \sa setIgnoreContext()
         */
        bool ignoreContext() const;

        /**
         * Add a statement.
         *
         * \param s The statement containing the property to be set.
         * If the predicate has NRL cardinality restrictions existing
         * statements will be updated. Otherwise this method has the
         * same effect as Model::addStatement().
         *
         * Adding a statement that defines a predicate with a maximum
         * cardinality bigger than 1 which has already been reached 
         * fails with an error.
         *
         * \return Error::ErrorNone on success.
         */
//        Error::ErrorCode addStatement( const Statement& s );

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
                          const QString label, 
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
        QUrl createResource( const QUrl& type,
                             const QString& label, 
                             const QString& comment = QString(), 
                             const QString& icon = QString() );

        /**
         * Adds a set of statements to a new named graph and assigns it the specified role.
         *
         * \param statements The statements to add. The context of each statement is rewritten
         *                   to the newly created one.
         * \param role The named graph role. In most cases this will be one of Vocabulary::NRL::Ontology
         *             or Vocabulary::NRL::InstanceBase.
         *
         * \return The newly created graph on success or an invalid uri if an error occurs. In case
         * of an error see ErrorCache::lastError() for details.
         */
        QUrl addStatements( const QList<Statement>& statements, const QUrl& graphRole );

        /**
         * \return The namespace used for new graph URIs in newGraphUri.
         * \sa setBaseNamespace
         */
        QUrl baseNamespace() const;

        /**
         * Set the namespace used for new URIs in newGraphUri and newClassUri.
         * \sa baseNamespace
         */
        void setBaseNamespace( const QUrl& uri );

        /**
         * Create a new graph URI. This is used internally by methods such as
         * addStatements(const QList<Statement>&, const QUrl&)
         * to create a new graph. Can be overridden to create custom URIs.
         * The default implementation uses the baseNamespace in combination
         * with a random string URI fragment.
         *
         * \return A new unique URI to be used as a new named graph.
         */
        virtual QUrl newGraphUri();

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
         * Create a new resource URI. This is used internally by methods such as
         * createResource. Can be overridden to create custom URIs.
         * The default implementation uses the baseNamespace in combination
         * with a random string URI fragment.
         *
         * \param name The name of the resource, i.e. the rdf:label. This can
         *             be used to make the URI more user readable.
         *
         * \return A new unique URI to be used as a new named class.
         */
        virtual QUrl newResourceUri( const QString& name = QString() );

    private:
        class Private;
        Private* const d;
    };
}

#endif
