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

#pragma once

#include <string>
#include <vector>
#include <Librarian/Status.hh>

namespace Librarian
{
  //----------------------------------------------------------------------------
  //! Class for scanning the input query
  //----------------------------------------------------------------------------
  class QueryScanner
  {
    public:
      //------------------------------------------------------------------------
      //! Representation of a character
      //------------------------------------------------------------------------
      class Character
      {
        public:
          Character() {}
          Character( char value, int line, int column, int position ):
            pValue(value), pLine(line), pColumn(column), pPosition(position)
            {}
          char getValue()    const { return pValue; }
          int  getLine()     const { return pLine; }
          int  getColumn()   const { return pColumn; }
          int  getPosition() const { return pPosition; }
        private:
          char pValue    = 0;
          int  pLine     = 0;
          int  pColumn   = 0;
          int  pPosition = 0;
      };

      QueryScanner( const std::string &query ):
        pQuery(query) {}

      //------------------------------------------------------------------------
      //! Retrieve a character from the source query
      //!
      //! @return a zero-Character to denote end
      //------------------------------------------------------------------------
      Character getCharacter();

    private:
      std::string pQuery;
      int         pLine     = 1;
      int         pColumn   = 1;
      int         pPosition = 0;
  };

  //----------------------------------------------------------------------------
  //! Class for tokenizing the input
  //----------------------------------------------------------------------------
  class QueryLexer
  {
    public:
      enum TokenType
      {
        Unknown,   //<! Unknown tokey type
        Term,      //<! A search term
        Symbol,    //<! "(" or ")"
        BinaryOp,  //<! "AND" or "OR"
        UnaryOp,   //<! "NOT"
        End        //<! the end
      };

      //------------------------------------------------------------------------
      //! Representation of a token
      //------------------------------------------------------------------------
      class Token
      {
        public:
          Token() {}
          Token( const std::string &value, TokenType type, int line,
                 int column, int position ):
            pValue(value), pType(type), pLine(line), pColumn(column),
            pPosition(position) {}
          const std::string &getValue()    const { return pValue; }
          TokenType          getType()     const { return pType; }
          int                getLine()     const { return pLine; }
          int                getColumn()   const { return pColumn; }
          int                getPosition() const { return pPosition; }
        private:
          std::string pValue     = "";
          TokenType   pType      = End;
          int         pLine      = 0;
          int         pColumn    = 0;
          int         pPosition  = 0;
      };

      QueryLexer( QueryScanner &scanner ):
        pScanner(scanner) {}

      //------------------------------------------------------------------------
      //! Retrieve a token from the source query
      //!
      //! @return End token to terminate
      //------------------------------------------------------------------------
      Token getToken();

    private:
      QueryScanner &pScanner;
      Token         pPending;
  };

  //----------------------------------------------------------------------------
  //! Parse the query
  //----------------------------------------------------------------------------
  class QueryParser
  {
    public:
      class Node
      {
        public:
          Node( QueryLexer::TokenType type, const std::string token ):
            pType(type), pToken(token) {}
          ~Node()
          {
            for( auto c: pChildren )
              delete c;
          }

          auto &getChildren() const { return pChildren; }
          void addChild( Node *c ) { pChildren.push_back(c); };
          void clearChildren() { pChildren.clear(); }

          auto getType() const { return pType; }
          auto &getToken() const { return pToken; };
          auto childrenBegin() const { return pChildren.begin(); }
          auto childrenEnd() const { return pChildren.end(); }
        private:
          std::vector<Node*>    pChildren;
          QueryLexer::TokenType pType;
          std::string           pToken;
      };

      QueryParser( const std::string &query ):
        pQuery(query), pScanner(pQuery.c_str()), pLexer(pScanner) {}

      //------------------------------------------------------------------------
      //! Parse the query
      //!
      //! @param  parseTree return the resulting ast
      //! @return status of the operation
      //------------------------------------------------------------------------
      Status parse( Node *&parseTree );
    private:
      std::string tokenError( const std::string &prefix );
      bool accept( QueryLexer::TokenType t );
      bool accept( QueryLexer::TokenType t, const std::string &value );
      void getNextToken() { pToken = pLexer.getToken(); }

      Status query( Node *&parseTree );
      Status block1( Node *&parseTree );
      Status block2( Node *&parseTree );
      Status block3( Node *&parseTree );

      std::string       pQuery;
      QueryScanner      pScanner;
      QueryLexer        pLexer;
      QueryLexer::Token pToken;
  };
}
