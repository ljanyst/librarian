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
#include <deque>

#include <Librarian/Status.hh>

namespace Librarian
{
  class Index;

  class QueryExecutor
  {
    public:
      //------------------------------------------------------------------------
      //! Constructor
      //------------------------------------------------------------------------
      QueryExecutor( const Index *index ):
        pIndex( index ) {}

      //------------------------------------------------------------------------
      //! Execute a boolean query
      //------------------------------------------------------------------------
      Status runQuery( std::deque<std::string> &result,
                       const std::string       &query );

    private:
      const Index *pIndex;
  };
}
