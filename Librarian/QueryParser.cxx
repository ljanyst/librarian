//------------------------------------------------------------------------------
// Copyright (c) 2014 by Lukasz Janyst <ljanyst@buggybrain.net>
//------------------------------------------------------------------------------
// This file is part of the Librarian software suite.
//
// Librarian is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Librarian is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Librarian.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <memory>
#include <Librarian/QueryParser.hh>

namespace Librarian
{
  //----------------------------------------------------------------------------
  // Retrieve a character from the source query
  //----------------------------------------------------------------------------
  QueryScanner::Character QueryScanner::getCharacter()
  {
    if( pPosition >= pQuery.length() )
      return Character(0, pLine, pColumn, pPosition);

    char ch = pQuery[pPosition];
    Character toRet(ch, pLine, pColumn, pPosition);
    ++pPosition;
    ++pColumn;
    if( ch == '\n' )
    {
      ++pLine;
      pColumn = 1;
    }
    return toRet;
  }

  //----------------------------------------------------------------------------
  // Retrieve a token from the source query
  //----------------------------------------------------------------------------
  QueryLexer::Token QueryLexer::getToken()
  {
    QueryScanner::Character ch;

    //--------------------------------------------------------------------------
    // Return pending token if we had any
    //--------------------------------------------------------------------------
    if( pPending.getType() != End )
    {
      Token t = pPending;
      pPending = Token();
      return t;
    }

    //--------------------------------------------------------------------------
    // Skip all the spaces
    //--------------------------------------------------------------------------
    while(1)
    {
      ch = pScanner.getCharacter();
      if( ch.getValue() == 0 )
        return Token("", End, ch.getLine(), ch.getColumn(), ch.getPosition());
      if( ch.getValue() != ' ' )
        break;
    }

    //--------------------------------------------------------------------------
    // Brackets
    //--------------------------------------------------------------------------
    if( ch.getValue() == '(' || ch.getValue() == '(' )
      return Token( std::string(1, ch.getValue()), Symbol, ch.getLine(),
                    ch.getColumn(), ch.getPosition() );

    //--------------------------------------------------------------------------
    // Retrieve a word
    //--------------------------------------------------------------------------
    std::string tok;
    int line = ch.getLine(), column = ch.getColumn();
    int position = ch.getPosition();
    tok += ch.getValue();
    while(1)
    {
      ch = pScanner.getCharacter();
      if( ch.getValue() == 0 || ch.getValue() == ' ' ||
          ch.getValue() == ')' || ch.getValue() == '(' )
        break;
      tok += ch.getValue();
    }

    if( ch.getValue() == ')' || ch.getValue() == '(' )
    {
      pPending = Token( std::string(1, ch.getValue()), Symbol,
                        ch.getLine(), ch.getColumn(), ch.getPosition() );
    }

    TokenType t = Term;
    if( tok == "OR" || tok == "AND" )
      t = BinaryOp;
    else if( tok == "NOT" )
      t = UnaryOp;
    return Token( tok, t, line, column, position );
  }

  //----------------------------------------------------------------------------
  // Parse the query
  //----------------------------------------------------------------------------
  Status QueryParser::parse( Node *&parseTree )
  {
    return query(parseTree);
  }

  //----------------------------------------------------------------------------
  // Format the error message
  //----------------------------------------------------------------------------
  std::string QueryParser::tokenError( const std::string &prefix )
  {
    std::string msg = prefix + " at (";
    msg += std::to_string(pToken.getLine()) + ", ";
    msg += std::to_string(pToken.getColumn()) + ")";
    return msg;
  }

  //----------------------------------------------------------------------------
  // Accept the current token if matching the criteria and get the next one
  //----------------------------------------------------------------------------
  bool QueryParser::accept( QueryLexer::TokenType t )
  {
    if( pToken.getType() == t )
    {
      getNextToken();
      return true;
    }
    return false;
  }

  //----------------------------------------------------------------------------
  // Accept the current token if matching the criteria and get the next one
  //----------------------------------------------------------------------------
  bool QueryParser::accept( QueryLexer::TokenType t, const std::string &value )
  {
    if( pToken.getValue() == value )
      return accept(t);
    return false;
  }

  //----------------------------------------------------------------------------
  // Parse a query
  //
  // query = block1 "End" .
  //----------------------------------------------------------------------------
  Status QueryParser::query( Node *&parseTree )
  {
    getNextToken();
    Status st;
    if( !(st = block1(parseTree)).isOK() )
      return st;

    if( !accept(QueryLexer::End) )
    {
      delete parseTree; parseTree = 0;
      return Status( Status::errSyntax, tokenError("Syntax error") );
    }
    return Status();
  }

  //----------------------------------------------------------------------------
  // Parse a block1
  //
  // block1 = block2 { "OR" block2 } .
  //----------------------------------------------------------------------------
  Status QueryParser::block1(Node *&parseTree)
  {
    Status  st;
    std::unique_ptr<Node> n(new Node(QueryLexer::BinaryOp, "OR")) ;
    Node *tmp = 0;

    if( !(st = block2(tmp)).isOK() )
      return st;
    n->addChild(tmp);

    while( accept(QueryLexer::BinaryOp, "OR") )
    {
      if( !(st = block2(tmp)).isOK() )
        return st;
      n->addChild(tmp);
    }

    if( n->getChildren().size() == 1 )
    {
      tmp = n->getChildren()[0];
      n->clearChildren();
      parseTree = tmp;
    }
    else
      parseTree = n.release();

    return Status();
  }

  //----------------------------------------------------------------------------
  // Parse block2
  //
  // block2 = block3 { "AND" block3 } .
  //----------------------------------------------------------------------------
  Status QueryParser::block2(Node *&parseTree)
  {
    Status st;
    std::unique_ptr<Node> n(new Node(QueryLexer::BinaryOp, "AND")) ;
    Node *tmp = 0;

    if( !(st = block3(tmp)).isOK() )
      return st;
    n->addChild(tmp);

    while( accept(QueryLexer::BinaryOp, "AND") )
    {
      if( !(st = block3(tmp)).isOK() )
        return st;
      n->addChild(tmp);
    }

    if( n->getChildren().size() == 1 )
    {
      tmp = n->getChildren()[0];
      n->clearChildren();
      parseTree = tmp;
    }
    else
      parseTree = n.release();

    return Status();
  }

  //----------------------------------------------------------------------------
  // Parse block3
  //
  // block3 = searchTerm
  //          | "NOT" block3
  //          | "(" block1 ")"
  //----------------------------------------------------------------------------
  Status QueryParser::block3(Node *&parseTree)
  {
    if( pToken.getType() == QueryLexer::Term )
    {
      parseTree = new Node(QueryLexer::Term, pToken.getValue());
      getNextToken();
      return Status();
    }

    Status st;
    if( accept(QueryLexer::UnaryOp, "NOT") )
    {
      parseTree = new Node(QueryLexer::UnaryOp, "NOT");
      Node *tmp = 0;
      if( !(st = block3(tmp)).isOK() )
      {
        delete parseTree; parseTree = 0;
        return st;
      }
      parseTree->addChild(tmp);
      return Status();
    }

    if( accept(QueryLexer::Symbol, "(") )
    {
      if( !(st = block1(parseTree)).isOK() )
        return st;
      if( !accept(QueryLexer::Symbol, ")") )
      {
        delete parseTree; parseTree = 0;
        return Status(Status::errSyntax,
                      tokenError("Syntax error") );
      }
      return Status();
    }

    return Status( Status::errSyntax, tokenError("Syntax error") );
  }
};
