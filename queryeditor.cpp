/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2010 Vishesh Handa

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "queryeditor.h"
#include "sparqlsyntaxhighlighter.h"

#include <QtGui/QStringListModel>
#include <QtGui/QCursor>
#include <QtGui/QKeyEvent>
#include <QtGui/QScrollBar>

#include <KGlobalSettings>
#include <KDebug>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

#include <Nepomuk/ResourceManager>

QueryEditor::QueryEditor(QWidget* parent)
    : QTextEdit(parent)
{
    setFont( KGlobalSettings::fixedFont() );
    setAcceptRichText( false );
    setFocus();

    m_highlighter = new Nepomuk::SparqlSyntaxHighlighter( document() );

    //
    // Completion
    //
    QString query = QString::fromLatin1("select ?pre ?r where { graph ?g {?r a ?t.} ?g nao:hasDefaultNamespaceAbbreviation ?pre. }");

    Soprano::Model * model = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery( query, Soprano::Query::QueryLanguageSparql );

    QStringList candidates;
    while( it.next() ) {
        QString prefix = it["pre"].toString();
        QString res = it["r"].uri().toString();
        res = res.mid( res.lastIndexOf('#') + 1 );

        candidates << QString( prefix + ':' + res );
    }

    m_completer = new QCompleter( this );
    m_completer->setModel( new QStringListModel( candidates, m_completer ) );
    m_completer->setModelSorting( QCompleter::CaseSensitivelySortedModel );
    m_completer->setCaseSensitivity( Qt::CaseInsensitive );

    m_completer->setWidget( this );
    m_completer->setCompletionMode( QCompleter::PopupCompletion );

    connect( m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)) );
}

void QueryEditor::keyPressEvent(QKeyEvent* e)
{
    if( m_completer->popup()->isVisible() ) {
        switch (e->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                e->ignore();
                return; // let the completer do default behavior
            default:
                break;
        }
    }
    QTextEdit::keyPressEvent(e);

    QString text = wordUnderCursor();
    if( text.length() < 2 ) // min 2 char for completion
        return;

    m_completer->setCompletionPrefix( text );

    QRect cr = cursorRect();
    cr.setWidth( m_completer->popup()->sizeHintForColumn(0)
                 + m_completer->popup()->verticalScrollBar()->sizeHint().width() );
    m_completer->complete( cr );
}


QString QueryEditor::wordUnderCursor()
{
    static QString eow("~!@#$%^&*()+{}|\"<>,./;'[]\\-= "); // everything without ':', '?' and '_'
    QTextCursor tc = textCursor();

    tc.anchor();
    while( 1 ) {
        // vHanda: I don't understand why the cursor seems to give a pos 1 past the last char instead
        // of just the last char.
        int pos = tc.position() - 1;
        if( pos < 0 || eow.contains( document()->characterAt(pos) ) )
            break;
        tc.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor );
    }
    return tc.selectedText();
}

void QueryEditor::insertCompletion(const QString& completion)
{
    QTextCursor tc = textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}
