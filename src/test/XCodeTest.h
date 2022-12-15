//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#include <gtest/gtest.h>
#include "panther.h"
#include "Encryptor.h"
#include "XCode.h"
#include "Dataset.h"
#include "Encoder.h"
#include <util.h>
#include "helib/timing.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <gtest/gtest.h>
#include <time.h>       /* time */
#include <chrono>

namespace {
	int n;
	int mults;
    int c1,c2;
    int n_S, n_R;
	unsigned long plaintext_prime_modulus = 131;
	unsigned long phiM = 21845;//104353;
	unsigned long lifting = 1;
	unsigned long numOfBitsOfModulusChain = 256;
	unsigned long numOfColOfKeySwitchingMatrix = 2;
} 

class XCodeTestEnv : public testing::Environment {
	public:
		explicit XCodeTestEnv(int nn, int m, int col1, int col2, int n_Send, int n_Recv){
			n = nn;
			mults = m;
            c1 = col1;
            c2 = col2;
            n_S = n_Send;
            n_R = n_Recv;
		}
};

TEST(XCodeTest, MatrixOps){
    int d = 4;
    Panther::Dataset<int> matrix1(d,d);
    Panther::Dataset<int> matrix2(d,d);
    
    for(int r=0; r<d; r++){
        for(int c=0; c<d; c++){
            matrix1.assign(r,c,d*r+c);
            matrix2.assign(r,c,(d*r+c)%3);
        }
    }

    printf("Matrix 1:\n");
    matrix1.print();

    printf("Matrix 2:\n");
    matrix2.print();

    Panther::Encoder encoder;
    Panther::Dataset<int> encodedMat1 = encoder.encodeMatrix(matrix1);
    Panther::Dataset<int> encodedMat2 = encoder.encodeMatrix(matrix2);
    
    printf("Encoded Matrix 1:\n");
    encodedMat1.print();
    
    printf("Encoded Matrix 2:\n");
    encodedMat2.print();

    n = encodedMat1.columns();
    mults = encodedMat1.rows()/n;

    Panther::XCode<int> xcode(n, mults, plaintext_prime_modulus);
    std::vector<Panther::Shard<int>> mat1shards;
    std::vector<Panther::Shard<int>> mat2shards;

    xcode.encode(mat1shards, encodedMat1);
    xcode.encode(mat2shards, encodedMat2);

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);


    printf("Encrypting matrix 1:\n");
    for (int i = 0; i < mat1shards.size(); ++i) {
        mat1shards[i].encrypt(encryptor);
    }

    printf("Encrypting matrix 2:\n");
    for (int i = 0; i < mat2shards.size(); ++i) {
        mat2shards[i].encrypt(encryptor);
    }

    printf("Beginning matrixAdd()\n");
    
    std::vector<helib::Ctxt> sums = xcode.matrixAdd(encryptor, mat1shards, mat2shards);

    encryptor.decryptAndPrintCondensed("matrix1", *(mat1shards[0].getPart1Ctxt()), n*mults);
    encryptor.decryptAndPrintCondensed("matrix2", *(mat2shards[0].getPart1Ctxt()), n*mults);
    encryptor.decryptAndPrintCondensed("sums", sums[0], n*mults);

    helib::Ctxt prod = xcode.matrixMult(encryptor, mat1shards, mat2shards, d);
    encryptor.decryptAndPrintCondensed("prod", prod, n*mults);

    Panther::Dataset<int> empty(n*mults, n);
    std::vector<Panther::Shard<int>> sumShards;
    xcode.encode(sumShards, empty);
    for (int i = 0; i < mat2shards.size(); ++i) {
        sumShards[i].encrypt(encryptor);
    }
    sumShards[0].setPart1(prod);
}

TEST(XCodeTest, PSI) {
    util::info("Initializing data...");
    srand(time(NULL));
    auto start = std::chrono::high_resolution_clock::now();
    
    Panther::Encoder encoder;

    // int n_S = 15;
    // int n_R = rand()%15+15;

    // std::vector<int> receiverSet = {27, 9, 10, 53, 119, 8, 104, 3, 55, 108, 122, 107, 37, 9, 108, 30, 19, 120, 16, 122};
    // std::vector<int> senderSet = {9, 10, 52, 118, 7, 104, 2, 54, 107, 122, 107, 37, 8, 107, 29, 19, 120, 15, 122, 27, 127, 8, 37, 129, 130, 91, 59, 106, 89, 58, 28, 72, 112, 72, 15, 128, 88, 130, 44, 125};

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

    // n_S = senderSet.size();
    // n_R = receiverSet.size();
    printf("n_S = %d, n_R = %d\n", n_S, n_R);
    
    std::vector params = encoder.getPSIParams(n_R, n_S);
    n = params[0];
    mults = params[1];
    int max = params[2];
    int totalSenderSplits = params[3];
    printf("n=%d, mults=%d, max=%d, totalSenderSplits=%d \n", n, mults, max, totalSenderSplits);        

    Panther::Dataset<int> dataset = encoder.encodePSI(senderSet, receiverSet, plaintext_prime_modulus, n, mults, max, totalSenderSplits);
    
    printf("Encoded dataset:\n");
    dataset.print();

    std::vector<std::vector<Panther::Shard<int>>> allShards(dataset.columns()/n);
    Panther::Dataset<int> subDataset(mults*n, n);
    Panther::XCode<int> xcode(n, mults, plaintext_prime_modulus);

    util::info("Creating encryptor...");
    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                                 plaintext_prime_modulus,
                                 phiM,
                                 lifting,
                                 numOfBitsOfModulusChain,
                                 numOfColOfKeySwitchingMatrix);
    std::cout << "Slot Count: " << encryptor.getSlotCount() << std::endl;

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

        for (int i = 0; i < allShards[0].size(); ++i) {
            allShards[s][i].encrypt(encryptor);
        }
    }

    //plaintext expected answers true or false
    std::vector<bool> intersectExp(receiverSet.size());
    for(int i=0; i<receiverSet.size(); i++){
    	for(int j=0; j<senderSet.size(); j++){
    		if(receiverSet[i]==senderSet[j]){
    			intersectExp[i]=true;
    			break;
    		}
    	}
    }

    printf("Sender set: ");
    for(int i=0; i<n_S; i++){
    	printf("%d ", senderSet[i]);
    }
    printf("\n");

    printf("Receiver set: ");
    for(int i=0; i<n_R; i++){
    	printf("%d ", receiverSet[i]);
    }
    printf("\n");

    printf("Expected result: ");
    for(int i=0; i<intersectExp.size(); i++){
    	printf("%s ", intersectExp[i] ? "Zero" : "Non-Zero");
    }
    printf("\n");

    //calling the function to get the encrypted dotProd
    std::vector<long> intersectPtxt(encryptor.getSlotCount());
    util::info("Finding PSI...");
    int allShardsSize = allShards.size();
 
    printf("Sender ctxts");
    for(int sub = 0; sub<totalSenderSplits; sub++){
        helib::Ctxt senderCtxt = *(allShards[allShardsSize-1][n_R%n+sub].getPart1Ctxt());
        encryptor.decryptAndPrintCondensed("Ctxt", senderCtxt, n*mults); 
    }

    for(int i=0; i<n_R; i++){
        bool final = false;
        printf("\tGetting receiver ctxt from allShards[%d][%d]\n", i/n, i%n);
        helib::Ctxt receiverCtxtI = *(allShards[i/n][i%n].getPart1Ctxt());
        
        for(int sub = 0; sub<totalSenderSplits; sub++){
            helib::Ctxt ctxtPsi(*(encryptor.getPublicKey()));
            helib::Ctxt senderCtxt = *(allShards[allShardsSize-1][n_R%n+sub].getPart1Ctxt());
            ctxtPsi = xcode.dotProd(encryptor, receiverCtxtI, senderCtxt);
            // encryptor.decryptAndPrintCondensed("Ctxt", ctxt_dotProd, n); 
            encryptor.getEncryptedArray()->decrypt((ctxtPsi), *encryptor.getSecretKey(), intersectPtxt);
            printf("\tChecking i=%d, sub=%d, intersectPtxt=%ld\n", i, sub, intersectPtxt[0]);
            final = final || ((intersectPtxt[0]==0)==intersectExp[i]);
        }
        printf("c=%d:\tExpecting = %s\t%s\n"
                , i, intersectExp[i] ? "Zero" : "Non-Zero"
                , final ? "CORRECT" : "INCORRECT");
        ASSERT_EQ(final,true);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << std::endl;
}

TEST(XCodeTest, dotProduct) {
    util::info("Initializing data...");
    int rows = n * mults;
    int cols = n;
    int v1_ind = 0;
    int v2_ind = 1;
    Panther::Dataset<int> dataset(rows, cols);
    std::vector<int> v1_pxt(n*mults);
    std::vector<int> v2_pxt(n*mults);

    for (int c = 0; c < cols; ++c)
        for (int r = 0; r < rows - 2; ++r) {
            if (c < 2) {
                if ((r + 1) % n != 0 && (r + 1) % n != n - 1) {
                    dataset.assign(r, c, 2);
                    v1_pxt[r]=2;
                    v2_pxt[r]=2;
                }
            }
        }
    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n, mults, plaintext_prime_modulus);
    xcode.encode(shards, dataset);
    dataset.print();

    util::info("Creating encryptor...");
    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                                 plaintext_prime_modulus,
                                 phiM,
                                 lifting,
                                 numOfBitsOfModulusChain,
                                 numOfColOfKeySwitchingMatrix);

    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    helib::Ctxt ctxt_dotProd(*(encryptor.getPublicKey()));
    std::vector<long> ptxt_dotProd(encryptor.getEncryptedArray()->size());

    long dotProd = 0;
    for(int i=0; i<mults*n; i++){
        if ((i + 1) % n != 0 && (i + 1) % n != n - 1) {
            dotProd += v1_pxt[i] * v2_pxt[i];
        }
    }
    
    //calling the function to get the encrypted dotProd
    ctxt_dotProd = xcode.dotProd(encryptor, *(shards[v1_ind].getPart1Ctxt()), *(shards[v2_ind].getPart1Ctxt()));//, v1_ind, v2_ind);

    encryptor.decryptAndPrintCondensed("Dot Prod Ctxt", ctxt_dotProd, n * mults);
    encryptor.getEncryptedArray()->decrypt((ctxt_dotProd), *encryptor.getSecretKey(), ptxt_dotProd);

    printf("True dotProd: %ld\n", dotProd);
    printf("xcode.dotProd(): %ld\n", ptxt_dotProd[0]);
    ASSERT_EQ(dotProd, ptxt_dotProd[0]);
}

TEST(XCodeTest, summation) {
    util::info("Initializing data...");
    int rows = n * mults;
    int cols = n;
    std::cout << "n:" << n << " and mults: " << mults << "\n";
    Panther::Dataset<int> dataset(rows, cols);
    for (int r = 0; r < rows - 2; ++r) {
        for (int c = 0; c < cols; ++c)   //col first
            if ((r + 1) % n != 0 && (r + 1) % n != n - 1) {
                dataset.assign(r, c, 1);
            }
    }
    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n, mults, plaintext_prime_modulus);

    xcode.encode(shards, dataset);
    util::info("Creating encryptor...");
    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                                 plaintext_prime_modulus,
                                 phiM,
                                 lifting,
                                 numOfBitsOfModulusChain,
                                 numOfColOfKeySwitchingMatrix);

    std::vector<long> after1(encryptor.getEncryptedArray()->size());
    std::vector<long> before1_pxt;
    //Having the sum in plaintext from a value we already knew
    int temp = n * mults;
    int sum = n * (n - 2) * mults;
    for (int i = 0; i < n; i++) {
        if (i < n - 2) {
            before1_pxt.push_back(temp % plaintext_prime_modulus);
        } else {
            before1_pxt.push_back(sum % plaintext_prime_modulus);
        }
    }
    std::cout << "Before Sum in Plain Text\n";
    for (int i = 0; i < n; i++) {
        std::cout << " " << before1_pxt[i];
    }

    helib::Ctxt cSum(*(encryptor.getPublicKey()));
    util::info("Finished creating encryptor!");
    std::cout << "Slot count: " << encryptor.getSlotCount() << std::endl;
    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }
    //calling the function to get the encrypted sum
    cSum = xcode.summation(encryptor, shards);

    encryptor.getEncryptedArray()->decrypt((cSum), *encryptor.getSecretKey(), after1);
    encryptor.decryptAndPrintCondensed("Row 3 Summation ", cSum, n);

    for (int i = 0; i < n; i++) {
        ASSERT_EQ(before1_pxt[i], after1[i]);
    }
}

TEST(XCodeTest, Convolution){
    
    int dimA = 5;
    int dimW = 3;
    int matACount = 0;
    int matWCount = 0;

    int aInd = 0;
    int wInd = 1;

    util::info("Initial data...");
    Panther::Dataset<int> dataset(mults*n, n);
    for (int c = 0; c < n; ++c){
        for(int m = 0; m < mults; m++){
            for (int r = 0; r < n-2; ++r){
                if(c==aInd && matACount < dimA*dimA){
                    dataset.assign(m*n+r, c, 2);
                    matACount++;
                } else if(c==wInd && matWCount < dimW*dimW) {
                    dataset.assign(m*n+r, c, 1);
                    matWCount++;
                }              
            }
        }
    }
    dataset.print();
    
    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n,mults, plaintext_prime_modulus);
    util::info("Encoding...");
    xcode.encode(shards, dataset);
    dataset.print();
    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);


    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    helib::Ctxt c_B = xcode.convolution(encryptor, shards, aInd, wInd);
    encryptor.decryptAndPrintCondensed("c_B", c_B, n*mults);
    
    std::vector<long> featureMap(encryptor.getSlotCount());
    encryptor.getEncryptedArray()->decrypt(c_B, *encryptor.getSecretKey(), featureMap);

    int count = 0;
    int dimB = dimA-dimW+1;
    for(int r = 0; count < dimB*dimB; r++){
        if(r%n!=n-1 && r%n!=n-2){
            ASSERT_EQ(featureMap[r],2*dimB*dimB);
            count++;
        }
    }  
}

TEST(XCodeTest, Verify2){
    util::info("Initializing data...");
    Panther::Dataset<int> dataset(mults*n, n);
    int val = 1;
    for(int m = 0; m < mults; m++){
        for (int r = 0; r < n-2; ++r){
            for (int c = 0; c < n; ++c){
                dataset.assign(m*n+r, c,  val % plaintext_prime_modulus);
                val++;
            }
        }
    }

    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n,mults, plaintext_prime_modulus);
    util::info("Encoding...");
    xcode.encode(shards, dataset);
    dataset.print();
    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);


    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    helib::Ctxt integKey = xcode.integrityKeyGen(encryptor);
    encryptor.decryptAndPrintCondensed("Integrity Key:", integKey, n*mults);

    helib::Ctxt integTag = xcode.integrityTagGen(encryptor, shards, integKey);
    encryptor.decryptAndPrintCondensed("Integrity Tag:", integTag, n*mults);

    helib::Ctxt newTag = xcode.integrityTagGen(encryptor, shards, integKey);
    bool b = xcode.verify2(encryptor, newTag, integTag);
    std::cout << "Expecting true: [" << b << "]" << std::endl;
    ASSERT_EQ(b, true);

    // Make shard 0 incorrect
    shards[0].setPart1(*(shards[1].getPart1Ctxt()));
    newTag = xcode.integrityTagGen(encryptor, shards, integKey);
    b = xcode.verify2(encryptor, newTag, integTag);
    std::cout << "Expecting false: [" << b << "]" << std::endl;
    ASSERT_EQ(b, false);
}

TEST(XCodeTest, RefreshTags) {
    // int n=5;
    // int mults=1;
    util::info("Initializing data...");
    Panther::Dataset<int> dataset(mults*n, n);
    
    for(int m = 0; m < mults; m++){
        for (int r = 0; r < n-2; ++r){
            for (int c = 0; c < n; ++c){
                dataset.assign(m*n+r, c, m*n+r % plaintext_prime_modulus);              
            }
        }
    }

    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n, mults, plaintext_prime_modulus);
    util::info("Encoding...");
    xcode.encode(shards, dataset);
    util::info("Done Encoding...");
    dataset.print();

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    std::vector<Panther::Shard<int>> shardsBefore = shards;

    //Execute two-column recovery
    xcode.refreshTags(encryptor, shards);
    
    //Plaintext vector to store results of columns before
    std::vector<long> before1_1(encryptor.getEncryptedArray()->size());
    std::vector<long> before2_1(encryptor.getEncryptedArray()->size());

    //Plaintext vector to store results of columns after
    std::vector<long> after1_1(encryptor.getEncryptedArray()->size());
    std::vector<long> after2_1(encryptor.getEncryptedArray()->size());
    
    for (int c=0; c<n; c++){
        printf("Testing for c=%d\n", c);
        //Decrypt column ciphertexts actual values into 'before' vectors
        encryptor.getEncryptedArray()->decrypt(*(shardsBefore[c].getPart1Ctxt()), *encryptor.getSecretKey(), before1_1);
        encryptor.getEncryptedArray()->decrypt(*(shardsBefore[c].getPart2Ctxt()), *encryptor.getSecretKey(), before2_1);

        //Print 'before' result
        encryptor.decryptAndPrintCondensed("Before Tag Refresh: c p1", *(shardsBefore[c].getPart1Ctxt()) , n);
        encryptor.decryptAndPrintCondensed("Before Tag Refresh: c p2", *(shardsBefore[c].getPart2Ctxt()) , n);

        //Decrypt column ciphertexts after recovery into 'after' vectors
        encryptor.getEncryptedArray()->decrypt(*(shards[c].getPart1Ctxt()), *encryptor.getSecretKey(), after1_1);
        encryptor.getEncryptedArray()->decrypt(*(shards[c].getPart2Ctxt()), *encryptor.getSecretKey(), after2_1);

        //Print 'after' result
        encryptor.decryptAndPrintCondensed("After Tag Refresh: c p1", *(shards[c].getPart1Ctxt()) , n);
        encryptor.decryptAndPrintCondensed("After Tag Refresh: c p2", *(shards[c].getPart2Ctxt()) , n);

        //Compare each value in the vectors, 'before' and 'after' are not equal for any index
        for(int j=0; j<n; j++){
            ASSERT_EQ(before1_1[j], after1_1[j]) << "c = " << c << "  at row" << j << " before1_1 = " << before1_1[j] << " after1_1 = " << after1_1[j];
            ASSERT_EQ(before2_1[j], after2_1[j]) << "c = " << c <<"at row" << j << " before2_1 = " << before2_1[j] << " after2_1 = " << after2_1[j];
        }
    }
}

TEST(XCodeTest, Verify){
	util::info("Initializing data...");
	Panther::Dataset<int> dataset(mults*n, n);
	int val = 1;
	for(int m = 0; m < mults; m++){
		for (int r = 0; r < n-2; ++r){
		    for (int c = 0; c < n; ++c){
		        dataset.assign(m*n+r, c,  val % plaintext_prime_modulus);
		        val++;
		    }
		}
	}
	dataset.print();

    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n,mults, plaintext_prime_modulus);
    util::info("Encoding...");
    xcode.encode(shards, dataset);
    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);


    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    helib::Ptxt<helib::BGV> mask(*(encryptor.getContext()));
    std::vector<helib::Ctxt*> tag(n);
    std::vector<helib::Ctxt*> verifyResults(n);

    util::info("Setting up mask");
    for(int r=0; r<n*mults; r++){
        if(r%n >= n-2){
            mask[r] = 1;
        } else{
            mask[r] = 0;
        }
    }

    util::info("Initializing tags");
    for(int c=0; c<n; c++){
        tag[c] = new helib::Ctxt(*(encryptor.getPublicKey()));
        *(tag[c]) = *(shards[c].getPart1Ctxt());
        (*tag[c]).multByConstant(mask);
        encryptor.decryptAndPrintCondensed("Tag",*(tag[c]), n*mults);
    }

    verifyResults = xcode.verify(encryptor, shards, tag);

    for(int c=0; c<n; c++){
        encryptor.decryptAndPrintCondensed("verifyResults",*(verifyResults[c]), 1);
    }
}

TEST(XCodeTest, OneColumnRecoverySingle) {
    // int n=5;
    // int mults=1;
    util::info("Initializing data...");
    Panther::Dataset<int> dataset(1*n, n);
    // for(int m = 0; m < 1; m++){
	for (int r = 0; r < n-2; ++r){
	    for (int c = 0; c < n; ++c){
	        dataset.assign(r, c, plaintext_prime_modulus % (n*r+c+1));
	    }
	}
    // }
    dataset.print();

    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n,1, plaintext_prime_modulus);
    util::info("Encoding...");
    xcode.encode(shards, dataset);
    util::info("Done Encoding...");
    // unsigned long plaintext_prime_modulus = 2;
    // unsigned long phiM = 21845;
    // unsigned long lifting = 1;
    // unsigned long numOfBitsOfModulusChain = 192;
    // unsigned long numOfColOfKeySwitchingMatrix = 2;

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
    

    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    // xcode.rotateShardsForRecovery(encryptor,shards);
	Panther::Direction dir = Panther::Direction::forward;
	xcode.rotateColumns(encryptor,shards,dir);    

    //Pick column to test randomly
    int c1 = rand()%n;

	//Plaintext vector to store results of columns before
	std::vector<long> before1_1(encryptor.getEncryptedArray()->size());
	std::vector<long> before2_1(encryptor.getEncryptedArray()->size());

	//Plaintext vector to store results of columns after
	std::vector<long> after1_1(encryptor.getEncryptedArray()->size());
	std::vector<long> after2_1(encryptor.getEncryptedArray()->size());

	printf("Testing for c1=%d\n", c1);
	//Decrypt column ciphertexts actual values into 'before' vectors
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart1Ctxt()), *encryptor.getSecretKey(), before1_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart2Ctxt()), *encryptor.getSecretKey(), before2_1);

    //Print 'before' result
    encryptor.decryptAndPrintCondensed("Before One Column Recovery: c1 p1", *(shards[c1].getPart1Ctxt()) , n);
    encryptor.decryptAndPrintCondensed("Before One Column Recovery: c1 p2", *(shards[c1].getPart2Ctxt()) , n);
    
    //Execute two-column recovery
    xcode.recoverFromOneColumnErasure(encryptor, shards, c1);

    //Decrypt column ciphertexts after recovery into 'after' vectors
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart1Ctxt()), *encryptor.getSecretKey(), after1_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart2Ctxt()), *encryptor.getSecretKey(), after2_1);

    //Print 'after' result
    encryptor.decryptAndPrintCondensed("After One Column Recovery: c1 p1", *(shards[c1].getPart1Ctxt()) , n);
    encryptor.decryptAndPrintCondensed("After One Column Recovery: c1 p2", *(shards[c1].getPart2Ctxt()) , n);

    //Compare each value in the vectors, 'before' and 'after' are not equal for any index
    for(int j=0; j<n; j++){
     	ASSERT_EQ(before1_1[j], after1_1[j]) << "c1 = " << c1 << "  at row" << j << " before1_1 = " << before1_1[j] << " after1_1 = " << after1_1[j];
     	ASSERT_EQ(before2_1[j], after2_1[j]) << "c1 = " << c1 <<"at row" << j << " before2_1 = " << before2_1[j] << " after2_1 = " << after2_1[j];
    }
}

TEST(XCodeTest, OneColumnRecoveryMultiple) {
    // int n=5;
    // int mults=2;

    util::info("Initializing data...");
    Panther::Dataset<int> dataset(mults*n, n);
    int val = 1;
    for(int m = 0; m < mults; m++){
    	for (int r = 0; r < n-2; ++r){
    	    for (int c = 0; c < n; ++c){
    	        dataset.assign(m*n+r, c,  val % plaintext_prime_modulus);
    	        val++;
    	    }
    	}
    }
    

    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n,mults, plaintext_prime_modulus);
    xcode.encode(shards, dataset);
    dataset.print();
    // unsigned long plaintext_prime_modulus = 2;
    // unsigned long phiM = 21845;
    // unsigned long lifting = 1;
    // unsigned long numOfBitsOfModulusChain = 192;
    // unsigned long numOfColOfKeySwitchingMatrix = 2;

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
    

    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    Panther::Direction dir = Panther::Direction::forward;
    xcode.rotateColumns(encryptor,shards,dir);  
    
    //Pick column to test randomly
    // int c1 = rand()%n;

	//Plaintext vector to store results of columns before
	std::vector<long> before1_1(encryptor.getEncryptedArray()->size());
	std::vector<long> before2_1(encryptor.getEncryptedArray()->size());

	//Plaintext vector to store results of columns after
	std::vector<long> after1_1(encryptor.getEncryptedArray()->size());
	std::vector<long> after2_1(encryptor.getEncryptedArray()->size());

	printf("Testing for c1=%d\n", c1);
	//Decrypt column ciphertexts actual values into 'before' vectors
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart1Ctxt()), *encryptor.getSecretKey(), before1_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart2Ctxt()), *encryptor.getSecretKey(), before2_1);

    //Print 'before' result
    encryptor.decryptAndPrintCondensed("Before One Column Recovery: c1 p1", *(shards[c1].getPart1Ctxt()) , mults*n);
    encryptor.decryptAndPrintCondensed("Before One Column Recovery: c1 p2", *(shards[c1].getPart2Ctxt()) , mults*n);
    
    //Execute two-column recovery
    xcode.recoverFromOneColumnErasure(encryptor, shards, c1);

    //Decrypt column ciphertexts after recovery into 'after' vectors
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart1Ctxt()), *encryptor.getSecretKey(), after1_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart2Ctxt()), *encryptor.getSecretKey(), after2_1);

    //Print 'after' result
    encryptor.decryptAndPrintCondensed("After One Column Recovery: c1 p1", *(shards[c1].getPart1Ctxt()) , mults*n);
    encryptor.decryptAndPrintCondensed("After One Column Recovery: c1 p2", *(shards[c1].getPart2Ctxt()) , mults*n);

    //Compare each value in the vectors, 'before' and 'after' are not equal for any index
    for(int j=0; j<mults*n; j++){
     	ASSERT_EQ(before1_1[j], after1_1[j]) << "c1 = " << c1 << "  at row" << j << " before1_1 = " << before1_1[j] << " after1_1 = " << after1_1[j];
     	ASSERT_EQ(before2_1[j], after2_1[j]) << "c1 = " << c1 <<"at row" << j << " before2_1 = " << before2_1[j] << " after2_1 = " << after2_1[j];
    }
}

TEST(XCodeTest, TwoColumnRecoverySingle){
	// int n=5;
    srand(time(NULL));
    // plaintext_prime_modulus = 2;
    util::info("Initializing data...");
    Panther::Dataset<int> dataset(n, n);
    int val = 1;
    for (int r = 0; r < n-2; ++r){
        for (int c = 0; c < n; ++c){
            dataset.assign(r, c, val % plaintext_prime_modulus); //1);
        	val++;
        }
    }
    dataset.print();

    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n,1, plaintext_prime_modulus);
    xcode.encode(shards, dataset);
    
    // unsigned long plaintext_prime_modulus = 2;
    // unsigned long phiM = 21845;
    // unsigned long lifting = 1;
    // unsigned long numOfBitsOfModulusChain = 192;
    // unsigned long numOfColOfKeySwitchingMatrix = 2;

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);

    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    Panther::Direction dir = Panther::Direction::forward;
    xcode.rotateColumns(encryptor,shards,dir);  

    //Plaintext vector to store results of columns before
    std::vector<long> before1_1(encryptor.getEncryptedArray()->size());
    std::vector<long> before1_2(encryptor.getEncryptedArray()->size());
    std::vector<long> before2_1(encryptor.getEncryptedArray()->size());
    std::vector<long> before2_2(encryptor.getEncryptedArray()->size());

    //Pick two columns c1 and c2 randomly [0-n) ensuring that c1 != c2
    int c1, c2;
    c1 = rand()%n;
    c2 = c1;
    while(c2==c1){
    	c2 = rand()%n;
    }
	printf("Testing for c1=%d, c2=%d\n", c1, c2);

	//Decrypt column ciphertexts actual values into 'before' vectors
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart1Ctxt()), *encryptor.getSecretKey(), before1_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c2].getPart1Ctxt()), *encryptor.getSecretKey(), before1_2);
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart2Ctxt()), *encryptor.getSecretKey(), before2_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c2].getPart2Ctxt()), *encryptor.getSecretKey(), before2_2);

    //Print 'before' result
    encryptor.decryptAndPrintCondensed("Before Two Column Recovery: c1 p1", *(shards[c1].getPart1Ctxt()) , n);
    encryptor.decryptAndPrintCondensed("Before Two Column Recovery: c2 p1", *(shards[c2].getPart1Ctxt()) , n);
    encryptor.decryptAndPrintCondensed("Before Two Column Recovery: c1 p2", *(shards[c1].getPart1Ctxt()) , n);
    encryptor.decryptAndPrintCondensed("Before Two Column Recovery: c2 p2", *(shards[c2].getPart1Ctxt()) , n);
    
    //Execute two-column recovery
    xcode.recoverFromTwoColumnErasure(encryptor, shards, c1, c2);

    //Plaintext vector to store results of columns after
    std::vector<long> after1_1(encryptor.getEncryptedArray()->size());
    std::vector<long> after1_2(encryptor.getEncryptedArray()->size());
    std::vector<long> after2_1(encryptor.getEncryptedArray()->size());
    std::vector<long> after2_2(encryptor.getEncryptedArray()->size());

    //Decrypt column ciphertexts after recovery into 'after' vectors
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart1Ctxt()), *encryptor.getSecretKey(), after1_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c2].getPart1Ctxt()), *encryptor.getSecretKey(), after1_2);
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart2Ctxt()), *encryptor.getSecretKey(), after2_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c2].getPart2Ctxt()), *encryptor.getSecretKey(), after2_2);

    //Print 'after' result
    encryptor.decryptAndPrintCondensed("After Two Column Recovery: c1 p1", *(shards[c1].getPart1Ctxt()) , n);
    encryptor.decryptAndPrintCondensed("After Two Column Recovery: c2 p1", *(shards[c2].getPart1Ctxt()) , n);
    encryptor.decryptAndPrintCondensed("After Two Column Recovery: c1 p2", *(shards[c1].getPart1Ctxt()) , n);
    encryptor.decryptAndPrintCondensed("After Two Column Recovery: c2 p2", *(shards[c2].getPart1Ctxt()) , n);

    //Compare each value in the vectors, 'before' and 'after' are not equal for any index
    for(int j=0; j<n; j++){
     	ASSERT_EQ(before1_1[j], after1_1[j]) << "at row" << j << " before1_1 = " << before1_1[j] << " after1_1 = " << after1_1[j];
     	ASSERT_EQ(before1_2[j], after1_2[j]) << "at row" << j << " before1_2 = " << before1_2[j] << " after1_2 = " << after1_2[j];
     	ASSERT_EQ(before2_1[j], after2_1[j]) << "at row" << j << " before2_1 = " << before2_1[j] << " after2_1 = " << after2_1[j];
     	ASSERT_EQ(before2_2[j], after2_2[j]) << "at row" << j << " before2_2 = " << before2_2[j] << " after2_2 = " << after2_2[j];
    }
}

TEST(XCodeTest, TwoColumnRecoveryMultiple){
	// int n=5;
	// int mults = 2;
    srand(time(NULL));
	// plaintext_prime_modulus = 2;
    util::info("Initializing data...");
    Panther::Dataset<int> dataset(mults*n, n);
    int val = 1;
    for(int m = 0; m < mults; m++){
    	for (int r = 0; r < n-2; ++r){
    	    for (int c = 0; c < n; ++c){
    	        dataset.assign(m*n+r, c, val % plaintext_prime_modulus); //1);
    	    	val++;
    	    }
    	}
    }
    dataset.print();

    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n,mults, plaintext_prime_modulus);
    xcode.encode(shards, dataset);
    
    // unsigned long plaintext_prime_modulus = 2;
    // unsigned long phiM = 21845;
    // unsigned long lifting = 1;
    // unsigned long numOfBitsOfModulusChain = 192;
    // unsigned long numOfColOfKeySwitchingMatrix = 2;

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);

    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    xcode.rotateColumns(encryptor,shards,Panther::Direction::forward);

    //Plaintext vector to store results of columns before
    std::vector<long> before1_1(encryptor.getEncryptedArray()->size());
    std::vector<long> before1_2(encryptor.getEncryptedArray()->size());
    std::vector<long> before2_1(encryptor.getEncryptedArray()->size());
    std::vector<long> before2_2(encryptor.getEncryptedArray()->size());

    // Pick two columns c1 and c2 randomly [0-n) ensuring that c1 != c2
    // int c1, c2;
    // c1 = rand()%n;
    // c2 = c1;
    // while(c2==c1){
    // 	c2 = rand()%n;
    // }
	printf("Testing for c1=%d, c2=%d\n", c1, c2);

	//Decrypt column ciphertexts actual values into 'before' vectors
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart1Ctxt()), *encryptor.getSecretKey(), before1_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c2].getPart1Ctxt()), *encryptor.getSecretKey(), before1_2);
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart2Ctxt()), *encryptor.getSecretKey(), before2_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c2].getPart2Ctxt()), *encryptor.getSecretKey(), before2_2);

    //Print 'before' result
    encryptor.decryptAndPrintCondensed("Before Two Column Recovery: c1 p1", *(shards[c1].getPart1Ctxt()) , mults*n);
    encryptor.decryptAndPrintCondensed("Before Two Column Recovery: c2 p1", *(shards[c2].getPart1Ctxt()) , mults*n);
    encryptor.decryptAndPrintCondensed("Before Two Column Recovery: c1 p2", *(shards[c1].getPart2Ctxt()) , mults*n);
    encryptor.decryptAndPrintCondensed("Before Two Column Recovery: c2 p2", *(shards[c2].getPart2Ctxt()) , mults*n);
    
    //Execute two-column recovery
    xcode.recoverFromTwoColumnErasure(encryptor, shards, c1, c2);

    //Plaintext vector to store results of columns after
    std::vector<long> after1_1(encryptor.getEncryptedArray()->size());
    std::vector<long> after1_2(encryptor.getEncryptedArray()->size());
    std::vector<long> after2_1(encryptor.getEncryptedArray()->size());
    std::vector<long> after2_2(encryptor.getEncryptedArray()->size());

    //Decrypt column ciphertexts after recovery into 'after' vectors
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart1Ctxt()), *encryptor.getSecretKey(), after1_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c2].getPart1Ctxt()), *encryptor.getSecretKey(), after1_2);
    encryptor.getEncryptedArray()->decrypt(*(shards[c1].getPart2Ctxt()), *encryptor.getSecretKey(), after2_1);
    encryptor.getEncryptedArray()->decrypt(*(shards[c2].getPart2Ctxt()), *encryptor.getSecretKey(), after2_2);

    //Print 'after' result
    encryptor.decryptAndPrintCondensed("After Two Column Recovery: c1 p1", *(shards[c1].getPart1Ctxt()) , mults*n);
    encryptor.decryptAndPrintCondensed("After Two Column Recovery: c2 p1", *(shards[c2].getPart1Ctxt()) , mults*n);
    encryptor.decryptAndPrintCondensed("After Two Column Recovery: c1 p2", *(shards[c1].getPart2Ctxt()) , mults*n);
    encryptor.decryptAndPrintCondensed("After Two Column Recovery: c2 p2", *(shards[c2].getPart2Ctxt()) , mults*n);

    //Compare each value in the vectors, 'before' and 'after' are not equal for any index
    for(int j=0; j<mults*n; j++){
     	ASSERT_EQ(before1_1[j], after1_1[j]) << "at row" << j << " before1_1 = " << before1_1[j] << " after1_1 = " << after1_1[j];
     	ASSERT_EQ(before1_2[j], after1_2[j]) << "at row" << j << " before1_2 = " << before1_2[j] << " after1_2 = " << after1_2[j];
     	ASSERT_EQ(before2_1[j], after2_1[j]) << "at row" << j << " before2_1 = " << before2_1[j] << " after2_1 = " << after2_1[j];
     	ASSERT_EQ(before2_2[j], after2_2[j]) << "at row" << j << " before2_2 = " << before2_2[j] << " after2_2 = " << after2_2[j];
    }
}

TEST(XCodeTest, SimpleVerify){
    util::info("Initializing data...");
    Panther::Dataset<int> dataset(mults*n, n);
    int val = 1;
    for(int m = 0; m < mults; m++){
        for (int r = 0; r < n-2; ++r){
            for (int c = 0; c < n; ++c){
                dataset.assign(m*n+r, c, val % plaintext_prime_modulus); //1);
                val++;
            }
        }
    }
    dataset.print();

    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;
    Panther::XCode<int> xcode(n,mults, plaintext_prime_modulus);
    xcode.encode(shards, dataset);
    
    // unsigned long plaintext_prime_modulus = 2;
    // unsigned long phiM = 21845;
    // unsigned long lifting = 1;
    // unsigned long numOfBitsOfModulusChain = 192;
    // unsigned long numOfColOfKeySwitchingMatrix = 2;

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);

    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    util::info("Running SimpleVerify()...");
    bool passVerify = xcode.simpleVerify(encryptor, shards);
    printf("Returned: %s\n", passVerify ? "True" : "False");
}

// TEST(XCodeTest, RotateAfterEncrypt){
//     // int n=5;
    
//     // Fill dataset with same initial data
//     Panther::Dataset<bool> dataset(mults*n, n);
//     for (int r = 0; r < n-2; ++r){
//         for (int c = 0; c < n; ++c){
//             dataset.assign(r, c, (r+c)%plaintext_prime_modulus);
//         }
//     }

//     std::vector<Panther::Shard<bool>> shards;
//     Panther::XCode<bool> xcode(n,mults);
//     xcode.encode(shards, dataset);

//     //Rotated Matrices
//     Panther::Dataset<bool> rotation_mat1(n, n);    
//     for(int c=n-1; c>=0; c--){
//         for(int r=0; r<n; r++){
//             rotation_mat1.assign(c,r,dataset.get((r+(n-1-c))%n,c));
//         }
//     }
//     //Switch rows for correct part 2 
//     int tmp_row1;
//     int tmp_row2;
//     for(int c=0; c<n; c++){
//         tmp_row1 = dataset.get(n-2,c);
//         tmp_row2 = dataset.get(n-1,c);
//         dataset.assign(n-2,c,tmp_row2);
//         dataset.assign(n-1,c,tmp_row1);
//     }
//     Panther::Dataset<bool> rotation_mat2(n, n);
//     for(int c=0; c<n; c++){
//         for(int r=0; r<n; r++){
//             rotation_mat2.assign(c,r,dataset.get((r+c)%n,c));
//         }
//     }

//     printf("Rotated Matrix 1 (shards part 1): \n");
//     for(int i=0; i<n; i++){
//         for(int j=0; j<n; j++){
//             printf("%d ", rotation_mat1.get(i,j));
//         }
//         printf("\n");
//     }

//     printf("Rotated Matrix 2 (shards part 2): \n");
//     for(int i=0; i<n; i++){
//         for(int j=0; j<n; j++){
//             printf("%d ", rotation_mat2.get(i,j));
//         }
//         printf("\n");
//     }

//     util::info("Creating encryptor...");
//     // unsigned long plaintext_prime_modulus = 2;
//     // unsigned long phiM = 21845;
//     // unsigned long lifting = 1;
//     // unsigned long numOfBitsOfModulusChain = 64;
//     // unsigned long numOfColOfKeySwitchingMatrix = 2;

//     Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
//                               plaintext_prime_modulus,
//                               phiM,
//                               lifting,
//                               numOfBitsOfModulusChain,
//                               numOfColOfKeySwitchingMatrix);
//     util::info("Finished creating encryptor!");

//    for (int i = 0; i < shards.size(); ++i) {
//        shards[i].encrypt(encryptor);
//    }

//     Panther::Direction dir = Panther::Direction::forward;
//     xcode.rotateColumns(encryptor,shards,dir);  

//     printf("Decrypting Rotated Shards for recovery (Part 1)...\n");
//     for (int i = 0; i < shards.size(); ++i) {
//         std::vector<long> res(encryptor.getEncryptedArray()->size());
//         encryptor.getEncryptedArray()->decrypt(*(shards[i].getPart1Ctxt()), *encryptor.getSecretKey(), res);
//         for(int j=0; j<n; j++){
//             printf("%ld ", res[j]);
//             ASSERT_EQ(res[j],rotation_mat1.get(i,j));
//         }
//         printf("\n");
//     }
//     printf("\n");

//     printf("Decrypting Rotated Shards for recovery (Part 2)...\n");
//     for (int i = 0; i < shards.size(); ++i) {
//         std::vector<long> res2(encryptor.getEncryptedArray()->size());
//         encryptor.getEncryptedArray()->decrypt(*(shards[i].getPart2Ctxt()), *encryptor.getSecretKey(), res2);
//         for(int j=0; j<n; j++){
//             printf("%ld ", res2[j]);
//             ASSERT_EQ(res2[j],rotation_mat2.get(i,j));
//         }
//         printf("\n");
//     }
//     printf("\n");
// }

// TEST(XCodeTest, ShardEncoding){
//     // int n=5;
    
//     // Fill dataset and test_dataset with same initial data
//     Panther::Dataset<bool> test_dataset(n, n);
//     Panther::Dataset<bool> dataset(n, n);
//     for (int r = 0; r < n-2; ++r){
//         for (int c = 0; c < n; ++c){
//             dataset.assign(r, c, (r+c)%plaintext_prime_modulus);
//             test_dataset.assign(r, c, (r+c)%plaintext_prime_modulus);
//         }
//     }

//     // Calculate and assign parity for test_dataset
//     std::vector<int> check_row1(n,0);
//     int start = 1;
//     ;
//     for(int c=n-1; c>=0; c--){
//         for(int r=0; r<n-2; r++){
//             check_row1[(100*n+start-(n-1-c))%n] = check_row1[(100*n+start-(n-1-c))%n]+int(test_dataset.get((100*(n-2)+n-1-c+r)%(n-2),(100*(n-2)+c-r)%n));
//         }
//         check_row1[(100*n+start-(n-1-c))%n] = check_row1[(100*n+start-(n-1-c))%n]%2;
//     }
//     for(int c=0; c<n; c++){
//         test_dataset.assign(n-2, c, check_row1[c]);
//     }
//     std::vector<int> check_row2(n,0);
//     start = n-2;
//     for(int c=0; c<n; c++){
//         for(int r=0; r<n-2; r++){
//             check_row2[(start+c)%n] = check_row2[(start+c)%n]+int(test_dataset.get((c+r)%(n-2),(c+r)%n));
//         }
//         check_row2[(start+c)%n] = check_row2[(start+c)%n]%2;
//     }

//     for(int c=0; c<n; c++){
//         test_dataset.assign(n-1, c, check_row2[c]);
//     }

//     std::vector<Panther::Shard<bool>> shards;
//     Panther::XCode<bool> xcode(n,1,plaintext_prime_modulus);
//     xcode.encode(shards, dataset);

//     printf("Dataset: \n");
//     for(int i=0; i<n; i++){
//         for(int j=0; j<n; j++){
//             printf("%d ", dataset.get(i,j));
//         }
//         printf("\n");
//     }

//     printf("Test Dataset: \n");
//     for(int i=0; i<n; i++){
//         for(int j=0; j<n; j++){
//             printf("%d ", test_dataset.get(i,j));
//         }
//         printf("\n");
//     }

//     //Check Equality
//     for(int i=0; i<n; i++){
//         for(int j=0; j<n; j++){
//             ASSERT_EQ(test_dataset.get(i,j),dataset.get(i,j));
//         }    
//     }
// }

