//
//  main.h
//  LuaState
//
//  Created by Simon Mikuda on 29/05/14.
//
//  See LICENSE and README.md files

#include <cstdio>
#include <cstdlib>
#include <string>

void runTest(const std::string& filename)
{
    printf("#####################################\n");
    printf("##  Running test: %s\n", filename.c_str());
    printf("#####################################\n\n");

	// TODO: search for executables

#ifdef WIN32
	// NOTE: add your own directory according to project
	std::string prefix = "Debug\\";
	std::string suffix = ".exe";
#else
	std::string prefix = "./";
	std::string suffix = "";
#endif
	if (system(std::string(prefix + filename + suffix).c_str()) == 0)
        printf("Test %s OK...\n\n", filename.c_str());
    else {
        printf("Test %s FAILED!\n\n", filename.c_str());
        exit(1);
    }
}

int main(int argc, char** argv)
{
    runTest("get_test");
    runTest("lambda_test");
    runTest("ref_test");
    runTest("set_test");
    runTest("state_test");
    runTest("types_test");
    runTest("values_test");
    
    return 0;
}
