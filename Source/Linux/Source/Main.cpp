#include <iostream>
#include <GitVersion.hpp>

int main( int p_Argc, char **p_ppArgv )
{
	std::cout << "Voxel Demo" << std::endl;
	std::cout << "----------" << std::endl;
	std::cout << "Build Information" << std::endl;
	std::cout << "\tBranch:  " << GIT_BRANCH << std::endl;
	std::cout << "\tVersion: " << GIT_BUILD_VERSION << std::endl;
	std::cout << "\tTag:     " << GIT_TAG_NAME << std::endl;
	return 0;
}

