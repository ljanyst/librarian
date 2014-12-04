cmake_minimum_required(VERSION 3.0)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )

add_subdirectory( Librarian )

include_directories( ${CMAKE_SOURCE_DIR} )

add_executable(
  indexer
  Indexer.cxx
  )

target_link_libraries(
  indexer
  Librarian
  )

add_executable(
  query_processor
  QueryProcessor.cxx
  )

target_link_libraries(
  query_processor
  Librarian
  )
