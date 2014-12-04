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

#include <cerrno>
#include <cstring>

#include <Librarian/Tokenizer.hh>

namespace Librarian
{
  //----------------------------------------------------------------------------
  // Open a file for tokenization
  //----------------------------------------------------------------------------
  Status FileTokenizer::open( const std::string &uri )
  {
    pStream = new std::ifstream( uri.c_str() );
    if( !pStream->is_open() )
      return Status( Status::errIO, strerror( errno ) );
    return Status();
  }

  //----------------------------------------------------------------------------
  // Close the file
  //----------------------------------------------------------------------------
  void FileTokenizer::close()
  {
    delete pStream;
    pStream = 0;
  }
}
