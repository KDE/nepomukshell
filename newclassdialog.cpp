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

#include "newclassdialog.h"
#include "pimomodel.h"

#include <nepomuk2/class.h>
#include <nepomuk2/property.h>
#include <nepomuk2/resource.h>
#include <nepomuk2/resourcemanager.h>

#include <KDebug>
#include <KLocale>

#include <QtGui/QTreeView>
#include <QtGui/QHeaderView>

#include <Soprano/Vocabulary/RDFS>
#include <Nepomuk2/Vocabulary/PIMO>


NewClassDialog::NewClassDialog( QWidget* parent )
    : KDialog( parent )
{
    setCaption( i18n( "Create new Type" ) );
    setButtons( Ok|Cancel );
    enableButtonOk( false );

    setupUi( mainWidget() );

    connect( m_editClassLabel, SIGNAL(textChanged(QString)),
             this, SLOT(slotLabelChanged(QString)) );

    m_editClassLabel->setFocus();
}


NewClassDialog::~NewClassDialog()
{
}


void NewClassDialog::slotLabelChanged( const QString& text )
{
    enableButtonOk( !text.isEmpty() );
}


Nepomuk2::Types::Class NewClassDialog::createClass( Nepomuk2::Types::Class parentClass, QWidget* parent )
{
    NewClassDialog dlg( parent );
    dlg.m_labelTitle->setText( i18n( "Create New Type" ) );
    dlg.m_labelTitle->setComment( i18nc( "Continuation of the previous message. %1 = name of class",
                                         "based on <i>%1</i>", parentClass.label() )  );
    dlg.m_rangeWidget->hide();
    QIcon icon = parentClass.icon();
    if ( icon.isNull() ) {
        icon = KIcon( QLatin1String( "nepomuk" ) );
    }
    dlg.m_labelTitle->setPixmap( icon.pixmap( 32, 32 ) );

    if ( dlg.exec() ) {
        QString name = dlg.m_editClassLabel->text();
        QString comment = dlg.m_editClassComment->text();
        QString icon = dlg.m_buttonClassIcon->icon();

        Nepomuk2::PimoModel pimoModel( Nepomuk2::ResourceManager::instance()->mainModel() );
        return pimoModel.createClass( parentClass.uri(),
                                      name,
                                      comment,
                                      icon != QLatin1String( "unknown" ) ? icon : QString() );
    }
    else {
        return Nepomuk2::Types::Class();
    }
}


Nepomuk2::Types::Property NewClassDialog::createProperty( Nepomuk2::Types::Class parentClass, QWidget* parent )
{
    NewClassDialog dlg( parent );
    dlg.m_labelTitle->setText( i18n( "Create New Property" ) );
    dlg.m_labelTitle->setComment( i18nc( "Continuation of the previous message. %1 = name of class",
                                         "for <i>%1</i>", parentClass.label() )  );
    QIcon icon = parentClass.icon();
    if ( icon.isNull() ) {
        icon = KIcon( QLatin1String( "nepomuk" ) );
    }
    dlg.m_labelTitle->setPixmap( icon.pixmap( 32, 32 ) );

    // FIXME: hopefully at some point QComboBox will support QTreeView
    // we can then use the PIMOItemModel.
    Nepomuk2::Types::Class base( Nepomuk2::Vocabulary::PIMO::Thing() );
    dlg.m_propertyRangeCombo->addItem( base.label(), base.uri() );
    foreach( Nepomuk2::Types::Class c, base.allSubClasses() ) {
        dlg.m_propertyRangeCombo->addItem( c.label(), c.uri() );
    }
    // FIXME: add literal ranges: string, int, double, datetime

    if ( dlg.exec() ) {
        QString name = dlg.m_editClassLabel->text();
        QString comment = dlg.m_editClassComment->text();
        QString icon = dlg.m_buttonClassIcon->icon();

        Nepomuk2::PimoModel pimoModel( Nepomuk2::ResourceManager::instance()->mainModel() );
        return pimoModel.createProperty( parentClass.uri(),
                                         dlg.m_propertyRangeCombo->itemData( dlg.m_propertyRangeCombo->currentIndex() ).toUrl(),
                                         name,
                                         comment,
                                         icon != QLatin1String( "unknown" ) ? icon : QString() );
    }
    else {
        return Nepomuk2::Types::Property();
    }
}


Nepomuk2::Resource NewClassDialog::createResource( Nepomuk2::Types::Class type, QWidget* parent )
{
    NewClassDialog dlg( parent );
    dlg.m_labelTitle->setText( i18n( "Create New Resource" ) );
    dlg.m_labelTitle->setComment( i18nc( "Continuation of the previous message", "of type <i>%1</i>", type.label() )  );
    dlg.m_rangeWidget->hide();
    QIcon icon = type.icon();
    if ( icon.isNull() ) {
        icon = KIcon( QLatin1String( "nepomuk" ) );
    }
    dlg.m_labelTitle->setPixmap( icon.pixmap( 32, 32 ) );

    if ( dlg.exec() ) {
        const QString name = dlg.m_editClassLabel->text();
        const QString comment = dlg.m_editClassComment->text();
        const QString icon = dlg.m_buttonClassIcon->icon();

        Nepomuk2::Resource newResource( QUrl(), type.uri() );
        newResource.setLabel( name );
        if ( !comment.isEmpty() ) {
            newResource.setDescription( comment );
        }
        if ( !icon.isEmpty() ) {
            // FIXME: create a proper Symbol object, if possible maybe a subclass DesktopIcon if its a standard icon
            newResource.addSymbol( icon );
        }
        return newResource;
    }
    else {
        return Nepomuk2::Resource();
    }
}

#include "newclassdialog.moc"
