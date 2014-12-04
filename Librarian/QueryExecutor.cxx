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

#include <cstdint>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <cstring>
#include <memory>

#include <Librarian/QueryExecutor.hh>
#include <Librarian/QueryParser.hh>
#include <Librarian/Index.hh>
#include <Librarian/Status.hh>

using namespace Librarian;

namespace
{
  //----------------------------------------------------------------------------
  //! Abstract node
  //----------------------------------------------------------------------------
  class Node
  {
    public:
      virtual ~Node() {}
      virtual void prepare( const Index *index ) = 0;
      virtual docid_t getResult() const = 0;
      virtual bool loadResult() = 0;

      uint64_t getCount() const
      {
        return pCount;
      }
    protected:
      uint64_t pCount = 0;
  };

  //----------------------------------------------------------------------------
  //! Load data from the postings list
  //----------------------------------------------------------------------------
  class DataLoader
  {
    public:
      DataLoader(const TermData::Postings *postings): pPostings(postings)
      { pCurrent = pPostings->begin(); }
      virtual docid_t getResult() const { return pDoc; }

      virtual bool loadResult()
      {
        if( pCurrent == pPostings->end() )
        {
          pDoc = (docid_t)-1;
          return false;
        }
        pDoc = *pCurrent;
        ++pCurrent;
        return true;
      }

    private:
      const TermData::Postings           *pPostings = 0;
      TermData::Postings::const_iterator  pCurrent;
      docid_t                             pDoc      = (docid_t)-1;
  };

  //----------------------------------------------------------------------------
  //! Intersection
  //----------------------------------------------------------------------------
  class Intersection
  {
    public:
      bool check(docid_t docId)
      {
        for(auto n: pNodes)
        {
          while(n->getResult() < docId && n->loadResult());
          if(n->getResult() == docId)
            continue;
          return false;
        }
        return true;
      }
      void addNode(Node *n) { pNodes.push_back(n); }
      void loadNodes() { for(auto n: pNodes) n->loadResult(); }
    private:
      std::vector<Node*> pNodes;
  };

  //----------------------------------------------------------------------------
  //! Sum
  //----------------------------------------------------------------------------
  class Sum
  {
    public:
      bool check(docid_t docId)
      {
        for(auto n: pNodes)
        {
          while(n->getResult() < docId && n->loadResult());
          if(n->getResult() == docId)
            return true;
        }
        return false;
      }
      void addNode(Node *n) { pNodes.push_back(n); }
      void loadNodes() { for(auto n: pNodes) n->loadResult(); }
    private:
      std::vector<Node*> pNodes;
  };

  //----------------------------------------------------------------------------
  //! Term node
  //----------------------------------------------------------------------------
  class TermNode: public Node
  {
    public:
      TermNode( const std::string &term )
      {
        pTerm.resize(term.size());
        std::transform(term.begin(), term.end(), pTerm.begin(), tolower);
      }

      virtual void prepare( const Index *index )
      {
        auto it = index->find( pTerm );
        if( it != index->termsEnd() )
        {
          pCount    =  it->second.numPostings();
          pDataLoader.reset( new DataLoader(&it->second.getPostings()) );
        }
      }

      virtual docid_t getResult() const { return pDataLoader->getResult(); }
      virtual bool loadResult() { return pDataLoader->loadResult(); };

    protected:
      std::string                 pTerm;
      std::unique_ptr<DataLoader> pDataLoader;
  };

  //----------------------------------------------------------------------------
  //! Negate node
  //----------------------------------------------------------------------------
  class NotNode: public Node
  {
    public:
      void setChild( Node *n ) { pChild.reset(n); };
      Node *getChild() { return pChild.get(); };
      virtual void prepare( const Index *index )
      {
        pChild->prepare( index );
        pCount = index->numDocuments() - pChild->getCount();
        pIndex = index;
        pCurrent = pIndex->documentsBegin();
        ++pCurrent; // skip the dummy index
        pSum.addNode(pChild.get());
        pSum.loadNodes();
       }

      virtual docid_t getResult() const
      {
        return pDoc;
      }

      virtual bool loadResult()
      {
        while(pCurrent != pIndex->documentsEnd())
        {
          pDoc = pCurrent->first;
          ++pCurrent;
          if(pSum.check(pDoc))
            continue;
          return true;
        }

        return false;
      }

    protected:
      std::unique_ptr<Node>         pChild;
      docid_t                       pDoc     = (docid_t)-1;
      Index::DocMap::const_iterator pCurrent;
      const Index                  *pIndex   = 0;
      Sum                           pSum;
  };

  //----------------------------------------------------------------------------
  //! Composite node
  //----------------------------------------------------------------------------
  class CompositeNode: public Node
  {
    public:
      void addChild( Node *child ) { pNodes.emplace_back(child); }
    protected:
      std::vector<std::unique_ptr<Node>> pNodes;
  };

  //----------------------------------------------------------------------------
  //! And node
  //----------------------------------------------------------------------------
  class AndNode: public CompositeNode
  {
    public:
      //------------------------------------------------------------------------
      // Prepare the query for optimal execution
      //------------------------------------------------------------------------
      virtual void prepare( const Index *index )
      {
        for( auto &n: pNodes )
          n->prepare( index );
        std::sort(pNodes.begin(), pNodes.end(),
                  [](auto &n1, auto &n2)
                    { return n1->getCount() < n2->getCount(); } );
        pCount = pNodes[0]->getCount();

        //----------------------------------------------------------------------
        // Find first node that is not a NotNode to use as a first source
        // of documents, if there is no such node just use the first node
        //----------------------------------------------------------------------
        for( auto &n: pNodes )
        {
          NotNode *notNode = dynamic_cast<NotNode*>(n.get());
          if(!notNode)
          {
            pFirst = n.get();
            break;
          }
        }
        if(!pFirst)
          pFirst = pNodes[0].get();

        //----------------------------------------------------------------------
        // Use not nodes as negators and normal nodes as intersectors
        //----------------------------------------------------------------------
        for( auto &n: pNodes )
        {
          Node *node = n.get();
          if( node == pFirst )
            continue;

          NotNode *notNode = dynamic_cast<NotNode*>(node);
          if(notNode)
            pNegators.addNode(notNode->getChild());
          else
            pIntersectors.addNode(node);
        }

        //----------------------------------------------------------------------
        // Load data to intersectors but not no negators, because these have
        // been loaded by the not-node's prepare already
        //----------------------------------------------------------------------
        pIntersectors.loadNodes();
      }

      virtual docid_t getResult() const
      {
        return pDoc;
      }

      virtual bool loadResult()
      {
        while(pFirst->loadResult())
        {
          pDoc = pFirst->getResult();
          if(pIntersectors.check(pDoc))
          {
            if(pNegators.check(pDoc))
              continue;
            return true;
          }
        }
        return false;
      }
    private:
      docid_t       pDoc;
      Node         *pFirst;
      Sum           pNegators;
      Intersection  pIntersectors;
  };

  //----------------------------------------------------------------------------
  //! Or node
  //----------------------------------------------------------------------------
  class OrNode: public CompositeNode
  {
    public:
      virtual void prepare( const Index *index )
      {
        for( auto &n: pNodes )
          n->prepare( index );
        pCount = 0;
        for( auto &n: pNodes )
        {
          pCount += n->getCount();
          n->loadResult();
        }
        std::sort(pNodes.begin(), pNodes.end(),
                  [](auto &n1, auto &n2)
                    {
                      NotNode *no1 = dynamic_cast<NotNode*>(n1.get());
                      NotNode *no2 = dynamic_cast<NotNode*>(n2.get());
                      if( no1 == no2 )
                        return n1->getCount() < n2->getCount();
                      return no1 < no2;
                    } );
      }

      virtual docid_t getResult() const
      {
        return pDoc;
      }

      //------------------------------------------------------------------------
      //! Load a result
      //------------------------------------------------------------------------
      virtual bool loadResult()
      {
        pDoc = (docid_t)-1;
        for( auto &n: pNodes )
        {
          if(n->getResult() == (docid_t)-1)
            continue;
          if(pDoc > n->getResult())
            pDoc = n->getResult();
        }

        if(pDoc == (docid_t)-1)
          return false;

        for( auto &n: pNodes )
          if(pDoc == n->getResult())
            n->loadResult();

        return true;
      }
    private:
      docid_t pDoc = (docid_t)-1;
  };

  //----------------------------------------------------------------------------
  // Translate the parse tree to the execution tree
  //----------------------------------------------------------------------------
  Node *translate( Librarian::QueryParser::Node *node )
  {
    using namespace Librarian;
    if(!node)
      return nullptr;

    switch(node->getType())
    {
      case(QueryLexer::Term):
        return new TermNode(node->getToken());
      case(QueryLexer::UnaryOp):
      {
        NotNode *n = new NotNode();
        n->setChild(translate(node->getChildren()[0]));
        return n;
      }
      case(QueryLexer::BinaryOp):
      {
        CompositeNode *n;
        if( node->getToken() == "OR" )
          n = new OrNode();
        else
          n = new AndNode();
        for( auto &ch: node->getChildren() )
          n->addChild(translate(ch));
        return n;
      }
     default:
       return nullptr;
    }
    return nullptr;
  }
}

namespace Librarian
{
  //----------------------------------------------------------------------------
  // Execute a boolean query
  //----------------------------------------------------------------------------
  Status QueryExecutor::runQuery( std::deque<std::string> &result,
                                  const std::string       &query )
  {
    QueryParser parser( query.c_str() );
    QueryParser::Node *parseTree = 0;
    Status st = parser.parse(parseTree);
    if( !st.isOK() )
      return st;
    Node *execTree = translate(parseTree);
    delete parseTree;
    execTree->prepare(pIndex);
    result.clear();
    while(execTree->loadResult())
      result.push_back(pIndex->getDocumentName(execTree->getResult()));
    delete execTree;
    return Status();
  }
};
