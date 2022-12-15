#include <gtest/gtest.h>
#include "panther.h"
#include "Encryptor.h"
#include "XCode.h"
#include "Dataset.h"
#include <util.h>
#include "Encoder.h"

namespace {
	// int n_S;
} 

class EncoderTestEnv : public testing::Environment {
	public:
		explicit EncoderTestEnv(int nn){
			// void
			n_S = nn;
		}
};

TEST(EncoderTest, EncodeMatrix){
	int d = 4;
	Panther::Dataset<int> matrix(d,d);
	for(int r=0; r<d; r++){
		for(int c=0; c<d; c++){
			matrix.assign(r,c,d*r+c);
		}
	}

	printf("Matrix:\n");
	matrix.print();

	Panther::Encoder encoder;
	Panther::Dataset<int> encodedMat = encoder.encodeMatrix(matrix);
	printf("Encoded Matrix:\n");
	encodedMat.print();
}

TEST(EncoderTest, EncodePSI){
	util::info("Initializing data...");
	srand(time(NULL));

	Panther::Encoder encoder;

    // int n_S = 15;
    int n_R = 5;

    std::vector<int> senderSet(n_S); // = {0, 1, 2, 3, 4, 0, 1, 2, 3, 4};
    std::vector<int> receiverSet(n_R); // = {1,3,5,7};
    int val;
    for(int i=0; i<n_S; i++){
        val = rand()%plaintext_prime_modulus;
    	senderSet[i] = val;
    	if(i < n_R){
    		receiverSet[i] = val + rand()%2;
    	}
    }

    std::vector params = encoder.getPSIParams(n_R, n_S);
    n = params[0];
    mults = params[1];
    int max = params[2];
    int totalSenderSplits = params[3];
    printf("n=%d, mults=%d, max=%d, totalSenderSplits=%d \n", n, mults, max, totalSenderSplits);

	Panther::Dataset<int> dataset = encoder.encodePSI(senderSet, receiverSet, plaintext_prime_modulus, n, mults, max, totalSenderSplits);
	printf("Encoded dataset \n n_R = %d, n_S = %d \n rows = %d, cols = %d\n", n_R, n_S, dataset.rows(), dataset.columns());
	dataset.print();

	std::vector<std::vector<Panther::Shard<int>>> allShards(dataset.columns()/n);
	Panther::Dataset<int> subDataset(mults*n, n);
	Panther::XCode<int> xcode(n, mults, plaintext_prime_modulus);

	for(int s=0; s<allShards.size(); s++){
	    for(int c=0; c<n; c++){
	        for(int r = 0; r < mults*n; r++){
	            subDataset.assign(r, c, dataset.get(r, s*n+c));
	        }
	    }

	    std::vector<Panther::Shard<int>> shards;
	    allShards[s] = shards;
	    printf("Initializing XCode Object n=%d, mults=%d\n", n, mults);
	    xcode.encode(allShards[s], subDataset);
	    printf("Subset %d:\n", s);
	    subDataset.print();
	}
}

TEST(EncoderTest, EncodeImageToInt){
	Panther::Encoder encoder;
	std::string fileName = "./data/input/172.pgm";
	encoder.encodeImageToInt(fileName);
}

TEST(EncoderTest, EncodeChar){
	int n = 5;
	Panther::Encoder encoder;
	char h = 'h';
	std::cout << "Encoding \'"<< h <<"\':" << std::endl;
	Panther::Dataset dataset = encoder.encodeChar(h, n);
	std::cout << "Result:" << std::endl;
	dataset.print();
}

TEST(EncoderTest, DecodeChar){
	int n = 5;
	Panther::Encoder encoder;
	Panther::Dataset<bool> dataset = encoder.encodeChar('h', n);
	std::cout << "Decoding: "  << std::endl;
	dataset.print();
	std::cout << "Result:" << std::endl;
	encoder.decodeChar(dataset);
}

TEST(EncoderTest, EncodeImageToBin){
	int n=5;
	Panther::Encoder encoder;
	std::string fileName = "data/input/172.Image";
	// std::cout << "Enter path to input file: ";
	// std::cin >> fileName;
	// std::cout << std::endl;
	// std::cout << std::endl;
	Panther::Dataset dataset = encoder.encodeImage(fileName, n);
	// Panther::Dataset dataset = encoder.encodeImage("data/input/172.ppm", n);
	// dataset.print();
}

TEST(EncoderTest, DecodeBinToImage){
	int n=5;
	Panther::Encoder encoder;
	std::string fileName;// = "data/input/grey-tiny.pgm";
	std::cout << "Enter path to input file: ";
	std::cin >> fileName;
	std::cout << std::endl;
	std::cout << std::endl;
	Panther::Dataset dataset = encoder.encodeImage(fileName, n);

	// dataset.print();
	std::cout << "Enter path to place output file: ";
	std::cin >> fileName;
	std::cout << std::endl;
	std::cout << std::endl;
	// fileName = "data/output/output.pgm";
	encoder.decodeImage(fileName, dataset);
}

TEST(EncoderTest, Expand){
	Panther::Encoder encoder;
	Panther::Dataset<bool> compressed(6,5);	
	for(int r=0; r<6; r++){
		for(int c=0; c<5; c++){
			compressed.assign(r,c,1);
		}
	}
	std::cout << "Compressed Size: "<< compressed.rows() << "x" << compressed.columns() << std::endl;
	// compressed.print();

	Panther::Dataset<bool> expanded = encoder.expandDataset(compressed);
	std::cout << "Expanded Size: " << expanded.rows() << "x" << expanded.columns() << std::endl;
	// expanded.print();
}

TEST(EncoderTest, Compress){
	Panther::Encoder encoder;
	Panther::Dataset<bool> compressed(6,5);	
	for(int r=0; r<6; r++){
		for(int c=0; c<5; c++){
			compressed.assign(r,c,1);
		}
	}
	Panther::Dataset<bool> expanded = encoder.expandDataset(compressed);
	std::cout << "Expanded: " << std::endl;
	expanded.print();

	Panther::Dataset<bool> freshCompressed = encoder.compressDataset(expanded);		
	std::cout << "Compressed: " << std::endl;
	freshCompressed.print();
}

TEST(EncoderTest, DecodeExpanded){
	int n=5;
	Panther::Encoder encoder;
	std::string fileName = "data/input/172.pgm";
	Panther::Dataset dataset = encoder.encodeImage(fileName, n);
	Panther::Dataset expanded = encoder.expandDataset(dataset);

	std::vector<Panther::Shard<bool>> shards;
	Panther::XCode<bool> xcode(expanded.columns(), expanded.rows()/expanded.columns(), 19);
	xcode.encode(shards, expanded);

	fileName = "data/output/172-xcode-encode.pgm";
	encoder.decodeImage(fileName, expanded);
}

