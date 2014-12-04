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

#include <fstream>
#include <cerrno>
#include <cstring>

#include <Librarian/Index.hh>

namespace Librarian
{
  //----------------------------------------------------------------------------
  // Dump the index to a file
  //----------------------------------------------------------------------------
  Status Index::dump( const std::string &filename ) const
  {
    std::ofstream out( filename.c_str() );
    if( !out.is_open() )
      return Status( Status::errIO, strerror(errno ) );

    //--------------------------------------------------------------------------
    // Dump document ids, all but the dummy one
    //--------------------------------------------------------------------------
    out << pDocuments.size()-1 << std::endl;
    for( auto it = pDocuments.begin(); it != pDocuments.end(); ++it )
      if( it->first != 0 )
        out << it->first << " " << it->second << std::endl;

    //--------------------------------------------------------------------------
    // Dump the postings
    //--------------------------------------------------------------------------
    out << pIndex.size() << std::endl;
    for( auto it = pIndex.begin(); it != pIndex.end(); ++it )
    {
      out << it->first << " " << it->second.numPostings() << " ";
      auto postIt = it->second.postingsBegin();
      for( ; postIt != it->second.postingsEnd(); ++postIt )
        out << *postIt << " ";
      out << std::endl;
    }
    return Status();
  }

  //----------------------------------------------------------------------------
  // Load an index from a file
  //----------------------------------------------------------------------------
  Status Index::load( const std::string &filename )
  {
    //--------------------------------------------------------------------------
    // Open the file
    //--------------------------------------------------------------------------
    std::ifstream in( filename.c_str() );
    if( !in.is_open() )
      return Status( Status::errIO, strerror(errno ) );

    //--------------------------------------------------------------------------
    // Read the document index
    //--------------------------------------------------------------------------
    pFreeDocId = 0;
    size_t numDocs;
    in >> numDocs;
    if( !in.good() )
    {
      cleanUp();
      return Status( Status::errIO, "File corrupted" );
    }

    docid_t id;
    std::string doc;
    for( int i = 0; i < numDocs; ++i )
    {
      in >> id >> doc;
      if( !in.good() )
      {
        cleanUp();
        return Status( Status::errIO, "File corrupted" );
      }
      pDocuments[id] = doc;
      pFreeDocId = std::max( pFreeDocId, id );
    }
    ++pFreeDocId;

    //--------------------------------------------------------------------------
    // Read back the postings
    //--------------------------------------------------------------------------
    size_t numTerms;
    in >> numTerms;
    if( !in.good() )
    {
      cleanUp();
      return Status( Status::errIO, "File corrupted" );
    }

    std::string term;
    size_t numPostings;
    for( int i = 0; i < numTerms; ++i )
    {
      in >> term >> numPostings;
      if( !in.good() )
      {
        cleanUp();
        return Status( Status::errIO, "File corrupted" );
      }

      TermData &d = pIndex[term];
      for( int k = 0; k < numPostings; ++k )
      {
        in >> id;
        if( !in.good() )
        {
          cleanUp();
          return Status( Status::errIO, "File corrupted" );
        }
        d.addPosting( id );
      }
    }
    return Status();
  }
}
