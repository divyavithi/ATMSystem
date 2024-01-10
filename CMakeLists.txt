cmake_minimum_required(VERSION 3.26)
project(PartB C)

set(CMAKE_C_STANDARD 11)

add_executable(PartB
        atm.c
        db_editor.c
        db_server.c
        semaphore.c
        semaphore.h
        structs.h
        msg_queue.c
        msg_queue.h
        shared_mem.c
        shared_mem.h)
