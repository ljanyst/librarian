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
#include <functional>
#include <vector>
#include <string>

#include <Librarian/Index.hh>
#include <Librarian/QueryExecutor.hh>

//------------------------------------------------------------------------------
// Procedure to perform
//------------------------------------------------------------------------------
namespace Param
{
  enum Params
  {
    Help    = 0,
    Run     = 1,
    Invalid = 2
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

  if( command == "run" )
  {
    if( argc != 4 )
      return Param::Invalid;
    params.push_back( argv[2] );
    params.push_back( argv[3] );
    return Param::Run;
  }

  return Param::Invalid;
}

//------------------------------------------------------------------------------
// Print help
//------------------------------------------------------------------------------
int help( const std::vector<std::string> &params )
{
  std::cerr << "Usage:" << std::endl;
  std::cerr << "   help                 print this help message" << std::endl;
  std::cerr << "   run index \"query\"    run a boolean query" << std::endl;
  return 0;
}

//------------------------------------------------------------------------------
// Run a query
//------------------------------------------------------------------------------
int run( const std::vector<std::string> &params )
{
  Librarian::Index         index;
  Librarian::QueryExecutor executor(&index);
  std::deque<std::string>  results;

  Librarian::Status st = index.load( params[0] );
  if( !st.isOK() )
  {
    std::cerr << "Unable to load index from " << params[0] << ": ";
    std::cerr << st.toString() << std::endl;
    return 2;
  }

  st = executor.runQuery( results, params[1] );
  if( !st.isOK() )
  {
    std::cerr << "Unable to process query \"" << params[1] << "\": ";
    std::cerr << st.toString() << std::endl;
    return 2;
  }
  std::cout << "Found " << results.size() << " documents:" << std::endl;
  for( auto &res: results )
    std::cout << res << std::endl;
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
  commands.push_back( run );

  if( p >= commands.size() )
  {
    std::cerr << "Umapped command" << std::endl;
    return 1;
  }

  return commands[p](params);
}
