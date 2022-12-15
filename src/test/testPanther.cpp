
//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#include <gtest/gtest.h>
#include <stdlib.h>
#include "test/XCodeTest.h"
#include "test/EncoderTest.h"

int main(int argc, char **argv) {
	// ./bin/testPanther n mults c1 c2
	int n = std::atoi(argv[1]);
	int mults = std::atoi(argv[2]);
	int c1 = std::atoi(argv[3]);
	int c2 = std::atoi(argv[4]);
	int n_S = std::stoi(argv[5]);
	int n_R = std::stoi(argv[6]);
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new XCodeTestEnv(n, mults, c1, c2, n_S, n_R));
    testing::AddGlobalTestEnvironment(new EncoderTestEnv(n_S));
    return RUN_ALL_TESTS();
}