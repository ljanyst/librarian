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
  //! Normalize an input string
  //----------------------------------------------------------------------------
  class Normalizer
  {
    public:
      //------------------------------------------------------------------------
      //! Remove the punctuation from the string
      //------------------------------------------------------------------------
      virtual std::string removePunctuation( const std::string &str ) = 0;

      //------------------------------------------------------------------------
      //! Normalize the input string by returning the base form of the string
      //------------------------------------------------------------------------
      virtual std::string normalize( const std::string &str ) = 0;
  };

  //----------------------------------------------------------------------------
  //! Normalize an input string in latin alphabet
  //----------------------------------------------------------------------------
  class LatinNormalizer: public Normalizer
  {
    public:
      //------------------------------------------------------------------------
      //! Remove the punctuation from the string
      //------------------------------------------------------------------------
      virtual std::string removePunctuation( const std::string &str );
  };

  //----------------------------------------------------------------------------
  //! Normalize an english input string
  //----------------------------------------------------------------------------
  class EnglishNormalizer: public LatinNormalizer
  {
    public:
      //------------------------------------------------------------------------
      //! Normalize the input string by returning the base form of the string
      //------------------------------------------------------------------------
      virtual std::string normalize( const std::string &str );
  };
}
