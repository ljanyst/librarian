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

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <cstring>
#include <libgen.h>

#include <Librarian/Index.hh>
#include <Librarian/Tokenizer.hh>
#include <Librarian/Normalizer.hh>

//------------------------------------------------------------------------------
// Procedure to perform
//------------------------------------------------------------------------------
namespace Param
{
  enum Params
  {
    Help    = 0,
    Create  = 1,
    Add     = 2,
    Invalid = 3
  };
}

//------------------------------------------------------------------------------
// Commandline parser
//------------------------------------------------------------------------------
Param::Params processCommandLine( std::vector<std::string>  &params,
                                  int                        argc,
                                  char                     **argv )
{
  if( argc == 1 )
    return Param::Invalid;

  std::string command = argv[1];
  if( command == "help" )
    return Param::Help;

  if( command == "create" )
  {
    if( argc != 3 )
      return Param::Invalid;
    params.push_back( argv[2] );
    return Param::Create;
  }

  if( command == "add" )
  {
    if( argc != 4 )
      return Param::Invalid;
    params.push_back( argv[2] );
    params.push_back( argv[3] );
    return Param::Add;
  }
  return Param::Invalid;
}

//------------------------------------------------------------------------------
// Print help
//------------------------------------------------------------------------------
int help( const std::vector<std::string> &params )
{
  std::cerr << "Usage:" << std::endl;
  std::cerr << "   help                print this help message" << std::endl;
  std::cerr << "   create filename     create a new index file" << std::endl;
  std::cerr << "   add index filename  add a new file to index" << std::endl;
  return 0;
}

//------------------------------------------------------------------------------
// Create a new index
//------------------------------------------------------------------------------
int create( const std::vector<std::string> &params )
{
  std::ofstream o(params[0]);
  if( !o.good() )
  {
    std::cerr << "Unable to open: " << params[0] << std::endl;
    return 1;
  }
  o << 0 << std::endl;
  o << 0 << std::endl;
  return 0;
}

//------------------------------------------------------------------------------
// Add new item to the index
//------------------------------------------------------------------------------
int add( const std::vector<std::string> &params )
{
  using namespace Librarian;

  //----------------------------------------------------------------------------
  // Load the index
  //----------------------------------------------------------------------------
  std::cerr << "Loading the index... " << std::flush;
  Index index;
  Status st = index.load( params[0] );
  if( !st.isOK() )
  {
    std::cerr << "Unable to load index from " << params[0] << ": ";
    std::cerr << st.toString() << std::endl;
    return 2;
  }
  std::cerr << "Done." << std::endl;

  //----------------------------------------------------------------------------
  // Open the input
  //----------------------------------------------------------------------------
  std::cerr << "Processing " << params[1] << "... " << std::flush;
  FileTokenizer t;
  st = t.open( params[1] );
  if( !st.isOK() )
  {
    std::cerr << st.toString() << std::endl;
    return 3;
  }

  char *nameBuf = new char[params[1].size()+1];
  nameBuf[params[1].size()] = 0;
  memcpy( nameBuf, params[1].c_str(), params[1].size() );
  char *name = basename( nameBuf );
  docid_t docId = index.registerDocument( name );
  delete [] nameBuf;

  EnglishNormalizer norm;
  std::unordered_set<std::string> tokens;
  int count = 0;
  while( t.loadNextToken() )
  {
    std::string token = norm.normalize(t.getToken());
    if( token.empty() )
      continue;
    tokens.insert( token );
    ++count;
  }

  for( auto it = tokens.begin(); it != tokens.end(); ++it )
    index.addPosting( *it, docId );

  std::cerr << "Processed " << count << " tokens, unique: " << tokens.size();
  std::cerr << "." << std::endl;

  //----------------------------------------------------------------------------
  // Clean up
  //----------------------------------------------------------------------------
  std::cerr << "Storing the index to " << params[0] << "... " << std::flush;
  t.close();
  st = index.dump( params[0] );
  if( !st.isOK() )
  {
    std::cerr << st.toString() << std::endl;
    return 5;
  }
  std::cerr << "Done." << std::endl;
  return 0;
}

//------------------------------------------------------------------------------
// The main show
//------------------------------------------------------------------------------
int main( int argc, char **argv )
{
  Param::Params p;
  std::vector<std::string> params;
  if( (p = processCommandLine( params, argc, argv )) == Param::Invalid )
  {
    std::cerr << "Invalid invocation. Type: '" << argv[0] << " help'";
    std::cerr << "for details" << std::endl;
    return 1;
  }

  std::vector<std::function<int(const std::vector<std::string>&)>> commands;
  commands.push_back( help );
  commands.push_back( create );
  commands.push_back( add  );

  if( p >= commands.size() )
  {
    std::cerr << "Umapped command" << std::endl;
    return 1;
  }

  return commands[p](params);
}
