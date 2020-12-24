project( order_book_gtest )

find_package( GTest MODULE REQUIRED )

file( GLOB GTEST_SRC "*.cpp" )

add_executable( ${PROJECT_NAME}
    ${GTEST_SRC} )

target_include_directories( ${PROJECT_NAME} PRIVATE
    ../inc )

target_link_libraries( ${PROJECT_NAME} PRIVATE
    order_book 
    GTest::GTest GTest::Main )

gtest_discover_tests(${PROJECT_NAME} )
