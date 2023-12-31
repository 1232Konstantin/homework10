cmake_minimum_required(VERSION 3.10)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")  
set(PROJECT_VESRION 0.0.${PATCH_VERSION})    

#for VS CODE LOCAL  in my PC
#set (BOOST_ROOT "G:/boost_1_83_0/boost_1_83_0/") # Путь к библиотеке Boost
#set (Boost_NO_SYSTEM_PATHS ON)
#set (Boost_USE_STATIC_LIBS ON)
#set (Boost_USE_STATIC_RUNTIME OFF)
#set (BOOST_ALL_DYN_LINK OFF)



project(bulk_server VERSION ${PROJECT_VESRION})

find_package(Boost 1.74 REQUIRED)

add_executable(bulk_server main.cpp)


set_target_properties(bulk_server  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

#for GITHUB
set_target_properties(bulk_server  PROPERTIES
    COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

#for VS CODE LOCAL  in my PC
#set_target_properties(bulk_server  PROPERTIES
#    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
#)


target_link_libraries(bulk_server
    ${Boost_LIBRARIES}
)


if(WIN32)
  target_link_libraries(bulk_server wsock32 ws2_32)
endif()




install(TARGETS bulk_server RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)

