#include "processor.hpp"

int main ( int argc, char *argv[] )
{
    processor_c<int> proc{ argv[1] };

    return 0;
}