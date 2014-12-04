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

#include <vector>
#include <string>
#include <Librarian/Status.hh>

namespace
{
  std::vector<std::string> gMessages = {"Success", "I/O Error", "Syntax Error"};
}

namespace Librarian
{
  //----------------------------------------------------------------------------
  // Represent the status as string so that it could easity be printed
  //----------------------------------------------------------------------------
  std::string Status::toString() const
  {
    if( pCode == success )
      return "[Success]";

    std::string ret = "[";
    if( pCode < gMessages.size() )
      ret += gMessages[pCode];
    else
      ret += "[Error";
    ret += "]";
    if( !pMessage.empty() )
    {
      ret += ": ";
      ret += pMessage;
    }
    return ret;
  }
};
