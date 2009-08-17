/*
   Copyright (C) 2009 by Sebastian Trueg <trueg at kde.org>

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

#ifndef _ASYNC_LOADING_RESOURCE_MODEL_H_
#define _ASYNC_LOADING_RESOURCE_MODEL_H_

#include "simpleresourcemodel.h"

namespace Nepomuk {
    namespace Types {
        class Class;
    }

    class AsyncLoadingResourceModel : public SimpleResourceModel
    {
        Q_OBJECT

    public:
        AsyncLoadingResourceModel( QObject* parent = 0 );
        ~AsyncLoadingResourceModel();

    public Q_SLOTS:
        void loadResourcesOfType( const Nepomuk::Types::Class& type );

    Q_SIGNALS:
        void finishedLoading();

    private:
        class Private;
        Private* const d;

        Q_PRIVATE_SLOT( d, void _k_queryNextReady( Soprano::Util::AsyncQuery* ) )
        Q_PRIVATE_SLOT( d, void _k_queryFinished( Soprano::Util::AsyncQuery* ) )
    };
}

#endif
