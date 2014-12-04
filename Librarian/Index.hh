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

#include <cstdint>
#include <map>
#include <unordered_map>
#include <string>
#include <list>

#include <Librarian/Status.hh>

namespace Librarian
{
  typedef uint64_t docid_t;

  //----------------------------------------------------------------------------
  //! Term data representation
  //----------------------------------------------------------------------------
  class TermData
  {
    public:
      typedef std::list<docid_t> Postings;

      //------------------------------------------------------------------------
      //! Number of postings
      //------------------------------------------------------------------------
      uint64_t numPostings() const
      {
        return pCount;
      }

      //------------------------------------------------------------------------
      //! Beginning iterator
      //------------------------------------------------------------------------
      Postings::const_iterator postingsBegin() const
      {
        return pPostings.begin();
      }

      //------------------------------------------------------------------------
      //! End iterator
      //------------------------------------------------------------------------
      Postings::const_iterator postingsEnd() const
      {
        return pPostings.end();
      }

      //------------------------------------------------------------------------
      //! Get postings
      //------------------------------------------------------------------------
      const Postings &getPostings() const
      {
        return pPostings;
      }

      //------------------------------------------------------------------------
      //! Add posting
      //------------------------------------------------------------------------
      void addPosting( docid_t id )
      {
        if( pPostings.empty() || pPostings.back() < id )
        {
          pPostings.push_back(id);
          ++pCount;
          return;
        }

        auto it = pPostings.begin();
        for( ; it != pPostings.end() && (*it) < id; ++it );
        if( *it == id )
          return;
        pPostings.push_back(id);
        ++pCount;
      }

      //------------------------------------------------------------------------
      //! Remove posting
      //------------------------------------------------------------------------
      void removePosting( docid_t id )
      {
        auto it = pPostings.begin();
        for( ; it != pPostings.end() && *it != id; ++it );
        if( it == pPostings.end() )
          return;
        pPostings.erase(it);
        --pCount;
      }

    private:
      uint64_t           pCount;
      std::list<docid_t> pPostings;
  };

  //----------------------------------------------------------------------------
  //! Represenation of the search index
  //----------------------------------------------------------------------------
  class Index
  {
    public:
      typedef std::unordered_map<std::string, TermData> Dict;
      typedef std::map<docid_t, std::string>            DocMap;

      //------------------------------------------------------------------------
      //! Constructor
      //------------------------------------------------------------------------
      Index()
      {
        pDocuments[0] = "";
      }

      //------------------------------------------------------------------------
      //! Dump the index to a file
      //------------------------------------------------------------------------
      Status dump( const std::string &filename ) const;

      //------------------------------------------------------------------------
      //! Load an index from a file
      //------------------------------------------------------------------------
      Status load( const std::string &filename );

      //------------------------------------------------------------------------
      //! Add posting
      //------------------------------------------------------------------------
      void addPosting( const std::string &term, docid_t posting )
      {
        auto it = pIndex.find(term);
        if( it == pIndex.end() )
          pIndex[term].addPosting( posting );
        else
          it->second.addPosting( posting );
      }

      //------------------------------------------------------------------------
      //! Get document name for the given id
      //------------------------------------------------------------------------
      const std::string &getDocumentName( docid_t id ) const
      {
        auto it = pDocuments.find( id );
        if( it == pDocuments.end() )
          return pDocuments.find(0)->second;
        return it->second;
      }

      //------------------------------------------------------------------------
      //! Register new document in the index
      //------------------------------------------------------------------------
      docid_t registerDocument( const std::string &name )
      {
        pDocuments[pFreeDocId] = name;
        return pFreeDocId++;
      }

      //------------------------------------------------------------------------
      //! Get number of documents
      //------------------------------------------------------------------------
      docid_t numDocuments() const
      {
        return pDocuments.size();
      }

      //------------------------------------------------------------------------
      //! Return the document map
      //------------------------------------------------------------------------
      const DocMap &getDocuments() const
      {
        return pDocuments;
      }

      //------------------------------------------------------------------------
      //! Begin documents
      //------------------------------------------------------------------------
      DocMap::const_iterator documentsBegin() const
      {
        return pDocuments.begin();
      }

      //------------------------------------------------------------------------
      //!
      //------------------------------------------------------------------------
      DocMap::const_iterator documentsEnd() const
      {
        return pDocuments.end();
      }

      //------------------------------------------------------------------------
      //! Begin terms
      //------------------------------------------------------------------------
      Dict::const_iterator termsBegin() const
      {
        return pIndex.begin();
      }

      //------------------------------------------------------------------------
      // End terms
      //------------------------------------------------------------------------
      Dict::const_iterator termsEnd() const
      {
        return pIndex.end();
      }

      //------------------------------------------------------------------------
      //! Find term
      //------------------------------------------------------------------------
      Dict::const_iterator find( const std::string &term ) const
      {
        return pIndex.find( term );
      }

    private:
      void cleanUp()
      {
        pIndex.clear();
        pDocuments.clear();
        pDocuments[0] = "";
        pFreeDocId    = 1;
      }
      docid_t pFreeDocId = 1;
      Dict    pIndex;
      DocMap  pDocuments;
  };
}
