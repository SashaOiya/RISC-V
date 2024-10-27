#include "processor.hpp"

int main ( int argc, char *argv[] )
{
    processor<int> proc{ argv[1] };
    proc.next_fetch();
    // end check

    return 0;
}