cmake_minimum_required(VERSION 2.8)
find_package( Freetype )

add_executable( example1 example1.cpp truetype_to_svg.hpp )
add_executable( example2 example2.cpp truetype_to_svg.hpp )
add_executable( example3 example3.cpp truetype_to_svg.hpp )

include_directories( ${FREETYPE_INCLUDE_DIRS} )
target_link_libraries( example1 ${FREETYPE_LIBRARIES} )
target_link_libraries( example2 ${FREETYPE_LIBRARIES} )
target_link_libraries( example3 ${FREETYPE_LIBRARIES} )

