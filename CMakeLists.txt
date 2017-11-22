cmake_minimum_required(VERSION 3.8)
project(ComputersystemerA3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        inc/alarm.h
        inc/ecg.h
        inc/errors.h
        inc/radio.h
        inc/simsec.h
        src/alarm.c
        src/radio.c
        src/radio_test.c
        src/simsec.c
        src/simsec_test.c
        makefile)

add_executable(ComputersystemerA3 ${SOURCE_FILES})