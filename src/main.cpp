#include "test_app.hpp"
#include "particle_life.hpp"

#include <iostream>


int main()
{
    PartcleLife particleLife{};

    try
    {
        particleLife.run();
    } 
    catch (const std::exception &e)
    {
        std::cerr << e.what() <<'\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}