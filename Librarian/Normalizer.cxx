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

#include <cctype>
#include <algorithm>

#include <Librarian/Normalizer.hh>

namespace Librarian
{
  //----------------------------------------------------------------------------
  // Remove the punctuation from the string
  //----------------------------------------------------------------------------
  std::string LatinNormalizer::removePunctuation( const std::string &str )
  {
    int s = 0, e = 0;
    for( ; s < str.length() && !isalnum(str[s]); ++s );
    for( ; s+e < str.length() && isalnum(str[s+e]); ++e );
    return str.substr( s, e );
  }

  //----------------------------------------------------------------------------
  // Normalize the input string by returning the base form of the string
  //----------------------------------------------------------------------------
  std::string EnglishNormalizer::normalize( const std::string &str )
  {
    std::string out; out.reserve( str.size() );
    int s = 0, e = 0;
    for( ; s < str.length() && !isalnum(str[s]); ++s );
    for( ; s+e < str.length() && isalnum(str[s+e]); ++e )
      out += tolower(str[s+e]);
    return out;
  }
}
