#include "test_app.hpp"

#include <iostream>


int main()
{
    TestApp testApp{};

    try
    {
        testApp.run();
    } 
    catch (const std::exception &e)
    {
        std::cerr << e.what() <<'\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}