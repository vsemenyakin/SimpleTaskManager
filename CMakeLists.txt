cmake_minimum_required(VERSION 3.2)
project(SimpleTaskManager)

set(CMAKE_CXX_STANDARD 14)

add_executable(SimpleTaskManager)

target_sources(SimpleTaskManager
	PRIVATE
        main.cpp
		TasksManager.h
		TasksManager.inl
		utils/Future.h
		utils/Future.inl
)
