/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NEPOMUKSHELLCONFIG_H_CMAKE
#define NEPOMUKSHELLCONFIG_H_CMAKE

/// @brief Soprano version as string at compile time.
#define NEPOMUKSHELL_VERSION_STRING "${CMAKE_NEPOMUKSHELL_VERSION_STRING}"

/// @brief The major Soprano version number at compile time
#define NEPOMUKSHELL_VERSION_MAJOR ${CMAKE_NEPOMUKSHELL_VERSION_MAJOR}

/// @brief The minor Soprano version number at compile time
#define NEPOMUKSHELL_VERSION_MINOR ${CMAKE_NEPOMUKSHELL_VERSION_MINOR}

/// @brief The Soprano release version number at compile time
#define NEPOMUKSHELL_VERSION_RELEASE ${CMAKE_NEPOMUKSHELL_VERSION_RELEASE}

#endif // NEPOMUKSHELLCONFIG_H_CMAKE
