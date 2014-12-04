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

namespace Librarian
{
  //----------------------------------------------------------------------------
  //! Encapsulate the status of an operation
  //----------------------------------------------------------------------------
  class Status
  {
    public:
      static const uint16_t success     = 0x0000; //!< All went well
      static const uint16_t errIO       = 0x0001; //!< An IO error has occurred
      static const uint16_t errSyntax   = 0x0002; //!< Syntax error

      //------------------------------------------------------------------------
      //! Constructor
      //------------------------------------------------------------------------
      Status( uint16_t code = success, const std::string &msg = "" ):
        pCode(code),
        pMessage(msg) {}

      //------------------------------------------------------------------------
      //! All went well?
      //------------------------------------------------------------------------
      bool isOK() const
      {
        return pCode == success;
      }

      //------------------------------------------------------------------------
      //! Represent the status as string so that it could easity be printed
      //------------------------------------------------------------------------
      std::string toString() const;

      //------------------------------------------------------------------------
      //! Get the status code
      //------------------------------------------------------------------------
      uint16_t getCode() const
      {
        return pCode;
      }

    private:
      uint16_t    pCode;
      std::string pMessage;
  };
}
