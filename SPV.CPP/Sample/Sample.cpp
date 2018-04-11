#include <stdio.h>
#include <iostream>
#include "SPVSDK.h"

int main(int argc, char *argv[])
{
	SPVSDK SDK;
	std::string version = SDK.version();
	std::cout << version << std::endl;

    return 0;
}
