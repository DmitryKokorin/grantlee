/*
  This file is part of the Grantlee template system.

  Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 3 only, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License version 3 for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "block.h"

#include <QStringList>

#include "parser.h"
#include "template.h"
#include "grantlee.h"
#include <util_p.h>


const char * __loadedBlocks = "__loadedBlocks";

BlockNodeFactory::BlockNodeFactory( QObject *parent ) : AbstractNodeFactory( parent )
{

}

Node* BlockNodeFactory::getNode( const QString &tagContent, Parser *p ) const
{
  QStringList expr = smartSplit( tagContent );

  if ( expr.size() != 2 ) {
    setError( TagSyntaxError, "block tag takes one argument" );
    return 0;
  }

  QString blockName = expr.at( 1 );

  QVariant loadedBlocksVariant = p->property( __loadedBlocks );
  QVariantList blockVariantList;

  if ( loadedBlocksVariant.isValid() && loadedBlocksVariant.type() == QVariant::List ) {
    blockVariantList = loadedBlocksVariant.toList();
    QListIterator<QVariant> it( blockVariantList );
    while ( it.hasNext() ) {
      QString blockNodeName = it.next().toString();

      if ( blockNodeName == blockName ) {
        setError( TagSyntaxError, QString( "%1 appears more than once." ).arg( blockName ) );
        return 0;
      }
    }
  }
  // Block not already in list.
  blockVariantList.append( blockName );
  loadedBlocksVariant = QVariant( blockVariantList );

  p->setProperty( __loadedBlocks, loadedBlocksVariant );

  BlockNode *n = new BlockNode( blockName );
  NodeList list = p->parse( n, QStringList() << "endblock" << "endblock " + blockName );

  n->setNodeList( list );
  p->deleteNextToken();

  return n;
}

BlockNode::BlockNode( const QString &name, QObject *parent )
    : Node( parent ), m_nodeParent( 0 )
{
//   m_filterExpression = FilterExpression(name);
  m_name = name;

  qRegisterMetaType<Grantlee::SafeString>( "Grantlee::SafeString" );
}

BlockNode::~BlockNode()
{
  delete m_nodeParent;
}

void BlockNode::setNodeList( NodeList list )
{
  m_list = list;
}

QString BlockNode::render( Context *c )
{
  c->push();
  m_context = c;
  c->insert( "block", QVariant::fromValue( static_cast<QObject *>( this ) ) );
  QString result = m_list.render( c );
  c->pop();
  return result;
}

SafeString BlockNode::getSuper() const
{
  return Util::markSafe( m_nodeParent->render( m_context ) );
}

QString BlockNode::blockName()
{
  return m_name;
//   return m_filterExpression.variable().toString();
}

BlockNode* BlockNode::nodeParent() const
{
  return m_nodeParent;
}

void BlockNode::addParent( NodeList nodeList )
{
  if ( m_nodeParent )
    m_nodeParent->addParent( nodeList );
  else {
    BlockNode *n = new BlockNode( m_name );
    n->setNodeList( nodeList );
    m_nodeParent = n;
  }
}

void BlockNode::setNodeParent( BlockNode* node )
{
  m_nodeParent = node;
}

NodeList BlockNode::nodeList()
{
  return m_list;
}

QString BlockNode::name()
{
  return m_name;
}

