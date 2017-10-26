#include "LightMonitor.h"

// for debug
#include <iostream>
#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>

using namespace std;


int main(int argc, char** argv)
{
    LightMonitor mon;
    mon.run();

    return 0;
}
