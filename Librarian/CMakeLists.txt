cmake_minimum_required( VERSION 3.0 )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )

include_directories( ${CMAKE_SOURCE_DIR} )

add_library(
  Librarian
  SHARED
  Status.cxx           Status.hh
  Index.cxx            Index.hh
  Tokenizer.cxx        Tokenizer.hh
  Normalizer.cxx       Normalizer.hh
  QueryExecutor.cxx    QueryExecutor.hh
  QueryParser.cxx      QueryParser.hh
  )
