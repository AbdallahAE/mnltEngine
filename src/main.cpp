#include "test_app.hpp"
#include "gravity_app.hpp"
#include "particle_life.hpp"

#include <iostream>

int main()
{
    /* PartcleLife p{}; */
    /* GravityApp g{}; */
    TestApp t{};

    try
    {

        t.run();
    } 
    catch (const std::exception &e)
    {
        std::cerr << e.what() <<'\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
