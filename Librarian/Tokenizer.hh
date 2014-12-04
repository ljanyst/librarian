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

#include <fstream>
#include <string>

#include <Librarian/Status.hh>

namespace Librarian
{
  //----------------------------------------------------------------------------
  //! Tokenize a resource
  //----------------------------------------------------------------------------
  class Tokenizer
  {
    public:
      //------------------------------------------------------------------------
      //! Open a resource for tokenization
      //------------------------------------------------------------------------
      virtual Status open( const std::string &uri ) = 0;

      //------------------------------------------------------------------------
      //! Close the resource
      //------------------------------------------------------------------------
      virtual void close() = 0;

      //------------------------------------------------------------------------
      //! Load next token
      //------------------------------------------------------------------------
      virtual bool loadNextToken() = 0;

      //------------------------------------------------------------------------
      //! Get the current token
      //------------------------------------------------------------------------
      virtual const std::string &getToken() const = 0;
  };

  //----------------------------------------------------------------------------
  //! Tokenize a file
  //----------------------------------------------------------------------------
  class FileTokenizer: public Tokenizer
  {
    public:
      //------------------------------------------------------------------------
      //! Open a resource for tokenization
      //------------------------------------------------------------------------
      virtual Status open( const std::string &uri );

      //------------------------------------------------------------------------
      //! Close the resource
      //------------------------------------------------------------------------
      virtual void close();

      //------------------------------------------------------------------------
      //! Load next token
      //------------------------------------------------------------------------
      virtual bool loadNextToken()
      {
        if(!pStream)
          return false;
        (*pStream) >> pToken;
        if( !pStream->good() )
          return false;
        return true;
      }

      //------------------------------------------------------------------------
      //! Get the current token
      //------------------------------------------------------------------------
      virtual const std::string &getToken() const
      {
        return pToken;
      }
    private:
      std::ifstream *pStream = 0;
      std::string    pToken;
  };

}
