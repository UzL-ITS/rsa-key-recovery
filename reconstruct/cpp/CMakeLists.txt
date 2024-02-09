cmake_minimum_required(VERSION 3.15)
project(reconstruct)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})

add_compile_options(-std=c++11)

find_package(Boost 1.7 REQUIRED)
find_package(GMP REQUIRED)
find_package(Threads)

add_subdirectory(jsoncpp)

add_executable(reconstruct
        main.cpp
        src/Candidate.cpp src/Candidate.h
        src/CandidateGenerator.cpp src/CandidateGenerator.h
        src/CandidateChecker.cpp src/CandidateChecker.h
        src/Constants.h
        src/ParameterTerminationHandler.h
        src/MathUtils.cpp src/MathUtils.h
        src/BinaryHelper.cpp src/BinaryHelper.h
        src/SimpleExampleTerminationHandler.cpp src/SimpleExampleTerminationHandler.h
        src/TerminationHandlerFactory.cpp src/TerminationHandlerFactory.h
        src/ValidationValues.cpp src/ValidationValues.h
        src/LengthValues.cpp src/LengthValues.h
        src/ValidationValuesFactory.cpp src/ValidationValuesFactory.h
        src/Config.cpp src/Config.h src/CandidateTransmitter.cpp src/CandidateTransmitter.h src/Observation.cpp src/Observation.h)

target_link_libraries(reconstruct jsoncpp_lib Boost::boost gmp ${CMAKE_THREAD_LIBS_INIT})

add_custom_command(TARGET reconstruct POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/bin/reconstruct ${CMAKE_SOURCE_DIR})