#include "TasksManager.h"

#include <string.h>
#include <chrono>
#include <iostream>
#include <sstream>

int main()
{
    auto TestIOSimulation1 = []
    {
        using namespace std::chrono_literals;

        constexpr auto Timeout = 2s;

        std::this_thread::sleep_for(Timeout);

        std::ostringstream sstream;
        sstream << "Loaded content 1 (after " << Timeout.count() << "s)";
        return std::string{ sstream.str() };
    };
    
    auto TestIOSimulation2 = []
    {
        using namespace std::chrono_literals;

        constexpr auto Timeout = 2s;

        std::this_thread::sleep_for(Timeout);

        std::ostringstream sstream;
        sstream << "Loaded content 2 (after " << Timeout.count() << "s)";
        return std::string{ sstream.str() };
    };
    
    // ========================================================================
    // ===================== Chaining example =================================

    TaskManager::Get().Run(TestIOSimulation1).Next(
        [&TestIOSimulation2](const std::string& Result1)
        {
            std::cout << Result1 << std::endl;
        
            TaskManager::Get().Run(TestIOSimulation2).Next(
                [](const std::string& Result2)
                {
                    std::cout << Result2 << std::endl;
                });
        });
    
    // ===================== Chaining example =================================    
    // ========================================================================    
    
    //Test update
    using namespace std::chrono_literals;
    
    std::chrono::steady_clock::duration TimeToWork = 5s;
    std::chrono::steady_clock::duration PassedTime = 0s;
    
    while (PassedTime <= TimeToWork)
    {
       std::chrono::steady_clock::time_point updateBeginTime = std::chrono::steady_clock::now();

       std::this_thread::sleep_for(200ms); //Simulate time for UI update

       PassedTime += std::chrono::steady_clock::now() - updateBeginTime;

       std::cout << "UI updated: " <<
          std::chrono::duration_cast<std::chrono::milliseconds>(PassedTime).count() <<
          std::endl;

       TaskManager::Get().ProcessFinishedTasksForCurrentThread();
    }
    
    return 0;
}

