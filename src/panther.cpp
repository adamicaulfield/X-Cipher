//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#include "panther.h"
#include "omp.h"
#include "Encryptor.h"
#include "helibExamples.h"
#include "XCode.h"
#include "Dataset.h"
#include "Encoder.h"
#include <util.h>
#include <algorithm>
#include <iterator>
#include "helib/timing.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <chrono>
#include <iostream>
#include <fstream>

// Time functions which act on all columns
void time_functions_one(int n){
    // Open file for timing results
    std::ofstream timingResultFile;
    timingResultFile.open("./data/timing/timing-results-others3.csv", std::ofstream::out | std::ofstream::app);
    
    // timingResultFile << n << "," << mults << ",";


    srand(time(NULL));
    unsigned long plaintext_prime_modulus = 53;
    unsigned long phiM = 21845;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 256;
    unsigned long numOfColOfKeySwitchingMatrix = 2;  
    // unsigned long desiredSlotCount = 64;
    // unsigned long securityLevel = 128;

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
                              // desiredSlotCount,
                              // securityLevel);


   int nslots = encryptor.getEncryptedArray()->size();
   std::cout << "Slot count: " << nslots << std::endl;

   int mults = nslots/n;
   timingResultFile << n << "," << mults << ",";

    // Fill Dataset with random data
    util::info("Initializing data...");
    Panther::Dataset<int> dataset(mults*n, n);
    for(int m = 0; m < mults; m++){
        for (int r = 0; r < n-2; ++r){
            for (int c = 0; c < n; ++c){
                dataset.assign(m*n+r, c, rand()%plaintext_prime_modulus); //1);
            }
        }
    }
    // dataset.print();

    // Codeword Generation
    // auto start = std::chrono::high_resolution_clock::now();
    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;  

    Panther::XCode<int> xcode(n,mults, plaintext_prime_modulus);
    xcode.encode(shards, dataset);
    // auto end = std::chrono::high_resolution_clock::now();
    // timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";
    

    // Encrypt Columns
    // start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }
    // end = std::chrono::high_resolution_clock::now();
    // timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    // std::cout << "Ctxt size (bytes): " << sizeof(*(shards[0].getPart1Ctxt())) << std::endl;

    // // Verification keygen
    // start = std::chrono::high_resolution_clock::now();
    // helib::Ctxt integKey = xcode.integrityKeyGen(encryptor);
    // end = std::chrono::high_resolution_clock::now();
    // timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";    

    // // Tag Gen
    // start = std::chrono::high_resolution_clock::now();
    // helib::Ctxt integTag = xcode.integrityTagGen(encryptor, shards, integKey);
    // end = std::chrono::high_resolution_clock::now();
    // timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";    

    // // Verify Ciphertexts
    // start = std::chrono::high_resolution_clock::now();
    // helib::Ctxt newTag = xcode.integrityTagGen(encryptor, shards, integKey);
    // bool verifyResults = xcode.verify2(encryptor, newTag, integTag);
    // end = std::chrono::high_resolution_clock::now();
    // timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    // //Refresh Codewords
    // start = std::chrono::high_resolution_clock::now();
    // xcode.refreshTags(encryptor, shards);
    // end = std::chrono::high_resolution_clock::now();
    // timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    // Simple Verify
    auto start = std::chrono::high_resolution_clock::now();
    bool passVerify = xcode.simpleVerify(encryptor, shards);
    auto end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    timingResultFile << "\n";
    timingResultFile.close();
}

// Functions which depend on specific columns
void time_functions_two(int n, int c1, int c2){
    // Open file for timing results
    std::ofstream timingResultFile;
    timingResultFile.open("./data/timing/timing-results-recovery2.csv", std::ofstream::out | std::ofstream::app);

    srand(time(NULL));
    unsigned long plaintext_prime_modulus = 53;
    unsigned long phiM = 21845;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 256;
    unsigned long numOfColOfKeySwitchingMatrix = 2;  
    // unsigned long desiredSlotCount = 64;
    // unsigned long securityLevel = 128;

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
                              // desiredSlotCount,
                              // securityLevel);

    int nslots = encryptor.getEncryptedArray()->size();
    std::cout << "Slot count: " << nslots << std::endl;

    int mults = nslots/n;

    timingResultFile << n << "," << mults << "," << c1 << "," << c2 << "," ;

     // Fill Dataset with random data
     util::info("Initializing data...");
     Panther::Dataset<int> dataset(mults*n, n);
     for(int m = 0; m < mults; m++){
         for (int r = 0; r < n-2; ++r){
             for (int c = 0; c < n; ++c){
                 dataset.assign(m*n+r, c, rand()%plaintext_prime_modulus); //1);
             }
         }
     }
     // dataset.print();

     // Encode & Encrypt
     util::info("Generating shards...");
     std::vector<Panther::Shard<int>> shards;  

     Panther::XCode<int> xcode(n,mults, plaintext_prime_modulus);
     xcode.encode(shards, dataset);
     for (int i = 0; i < shards.size(); ++i) {
         shards[i].encrypt(encryptor);
     }
     std::cout << "Ctxt size (bytes): " << sizeof(*(shards[0].getPart1Ctxt())) << std::endl;

    // Rotate for Recovery
    auto start = std::chrono::high_resolution_clock::now();
    xcode.rotateColumns(encryptor,shards,Panther::Direction::forward);
    auto end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    //One Column Recovery
    start = std::chrono::high_resolution_clock::now();
    xcode.recoverFromOneColumnErasure(encryptor, shards, c1);
    end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    //Two Column Recovery
    start = std::chrono::high_resolution_clock::now();
    xcode.recoverFromTwoColumnErasure(encryptor, shards, c1, c2);
    end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";


    timingResultFile << "\n";
    timingResultFile.close();
}

// Time basic ops
void time_functions_three(int n){
    // Open file for timing results
    std::ofstream timingResultFile;
    timingResultFile.open("./data/timing/timing-basic-ops2.csv", std::ofstream::out | std::ofstream::app);
    
    srand(time(NULL));
    unsigned long plaintext_prime_modulus = 53;
    unsigned long phiM = 21845;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 256;
    unsigned long numOfColOfKeySwitchingMatrix = 2;  
    // unsigned long desiredSlotCount = 64;
    // unsigned long securityLevel = 128;

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
                              // desiredSlotCount,
                              // securityLevel);


   int nslots = encryptor.getEncryptedArray()->size();
   std::cout << "Slot count: " << nslots << std::endl;

   int mults = nslots/n;

   timingResultFile << n << "," << mults << ",";

    // Fill Dataset with random data
    util::info("Initializing data...");
    Panther::Dataset<int> dataset(mults*n, n);
    for(int m = 0; m < mults; m++){
        for (int r = 0; r < n-2; ++r){
            for (int c = 0; c < n; ++c){
                dataset.assign(m*n+r, c, rand()%plaintext_prime_modulus); //1);
            }
        }
    }

    // Codeword Generation
    util::info("Generating shards...");
    std::vector<Panther::Shard<int>> shards;  

    Panther::XCode<int> xcode(n,mults, plaintext_prime_modulus);
    xcode.encode(shards, dataset);
    
    // Encrypt Columns
    for (int i = 0; i < shards.size(); ++i) {
        shards[i].encrypt(encryptor);
    }

    helib::Ctxt cSum(*(encryptor.getPublicKey()));
    helib::Ctxt ctxt_dotProd(*(encryptor.getPublicKey()));
    
    // Summation
    auto start = std::chrono::high_resolution_clock::now();
    cSum = xcode.summation(encryptor, shards);
    auto end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    // Dot Prod
    start = std::chrono::high_resolution_clock::now();
    // ctxt_dotProd = xcode.dotProd(encryptor, shards, 0, 1);
    xcode.refreshTags(encryptor, shards);
    end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    timingResultFile << "\n";
    timingResultFile.close();
}

// Example basic helib arithmetic
void test_slot_counts(unsigned long p) {
    unsigned long plaintext_prime_modulus = p;
    // unsigned long phiM = 26651;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 512;
    unsigned long numOfColOfKeySwitchingMatrix = 2;
    unsigned long securityLevel = 128;
    unsigned long desiredSlotCount = 0;

    // Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
    //                           plaintext_prime_modulus,
    //                           phiM,
    //                           lifting,
    //                           numOfBitsOfModulusChain,
    //                           numOfColOfKeySwitchingMatrix);
    
    Panther::Encryptor encryptor ("/tmp/sk.txt", "/tmp/pk.txt", 
                                plaintext_prime_modulus, 
                                lifting, 
                                numOfBitsOfModulusChain, 
                                numOfColOfKeySwitchingMatrix, 
                                desiredSlotCount, 
                                securityLevel);

    std::cout << "Slot count: " << encryptor.getEncryptedArray()->size() << std::endl;
    util::info("Finished creating encryptor.");

    // Create a vector of long with nslots elements
    helib::Ptxt<helib::BGV> ptxt(*(encryptor.getContext()));
}

void psi(int n_S, int n_R){
    util::info("Starting PSI");
    srand(time(NULL));

    std::ofstream timingResultFile;
    timingResultFile.open("./data/timing/timing-psi.csv", std::ofstream::out | std::ofstream::app);

    timingResultFile << n_R << "," << n_S << ",";

    Panther::Encoder encoder;

    unsigned long plaintext_prime_modulus = 131;
    unsigned long phiM = 21845;//104353;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 256;
    unsigned long numOfColOfKeySwitchingMatrix = 2;

    Panther::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                                 plaintext_prime_modulus,
                                 phiM,
                                 lifting,
                                 numOfBitsOfModulusChain,
                                 numOfColOfKeySwitchingMatrix);
    std::cout << "Slot Count: " << encryptor.getSlotCount() << std::endl;

    std::vector<int> senderSet(n_S); // = {0, 1, 2, 3, 4, 0, 1, 2, 3, 4};
    std::vector<int> receiverSet(n_R); // = {1,3,5,7};
    int val;
    for(int i=0; i<n_S; i++){
        senderSet[i] = rand()%plaintext_prime_modulus;
    }

    for(int i=0; i<n_R; i++){
        receiverSet[i] = senderSet[rand()%senderSet.size()]+rand()%2;
    }

    // printf("n_S = %d, n_R = %d\n", n_S, n_R);

    //PSI encoding starts here
    auto start = std::chrono::high_resolution_clock::now();

    std::vector params = encoder.getPSIParams(n_R, n_S);
    int n = params[0];
    int mults = params[1];
    int max = params[2];
    int totalSenderSplits = params[3];
    // printf("n=%d, mults=%d, max=%d, totalSenderSplits=%d \n", n, mults, max, totalSenderSplits);        

    Panther::Dataset<int> dataset = encoder.encodePSI(senderSet, receiverSet, plaintext_prime_modulus, n, mults, max, totalSenderSplits);
    
    // printf("Encoded dataset:\n");
    // dataset.print();

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
        // printf("Initializing XCode Object n=%d, mults=%d\n", n, mults);
        xcode.encode(allShards[s], subDataset);
        // printf("Subset %d:\n", s);
        // subDataset.print();

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
    
    // printf("Sender ctxts");
    // for(int sub = 0; sub<totalSenderSplits; sub++){
    //     helib::Ctxt senderCtxt = *(allShards[allShardsSize-1][n_R%n+sub].getPart1Ctxt());
    //     encryptor.decryptAndPrintCondensed("Ctxt", senderCtxt, n*mults); 
    // }

    for(int i=0; i<n_R; i++){
        bool final = false;
        // printf("\tGetting receiver ctxt from allShards[%d][%d]\n", i/n, i%n);
        helib::Ctxt receiverCtxtI = *(allShards[i/n][i%n].getPart1Ctxt());
        
        for(int sub = 0; sub<totalSenderSplits; sub++){
            helib::Ctxt ctxtPsi(*(encryptor.getPublicKey()));
            helib::Ctxt senderCtxt = *(allShards[allShardsSize-1][n_R%n+sub].getPart1Ctxt());
            ctxtPsi = xcode.dotProd(encryptor, receiverCtxtI, senderCtxt);
            // encryptor.decryptAndPrintCondensed("Ctxt", ctxt_dotProd, n); 
            encryptor.getEncryptedArray()->decrypt((ctxtPsi), *encryptor.getSecretKey(), intersectPtxt);
            // printf("\tChecking i=%d, sub=%d, intersectPtxt=%ld\n", i, sub, intersectPtxt[0]);
            final = final || ((intersectPtxt[0]==0)==intersectExp[i]);
        }
        printf("c=%d:\tExpecting = %s\t%s\n"
                , i, intersectExp[i] ? "Zero" : "Non-Zero"
                , final ? "CORRECT" : "INCORRECT");
        // ASSERT_EQ(final,true);
    }
    auto end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    timingResultFile << "\n";
    timingResultFile.close();
    
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << std::endl;
    util::info("Done PSI");
}

void matrix(int d){
    Panther::Dataset<int> matrix1(d,d);
    Panther::Dataset<int> matrix2(d,d);
    
    for(int r=0; r<d; r++){
        for(int c=0; c<d; c++){
            matrix1.assign(r,c,d*r+c);
            matrix2.assign(r,c,(d*r+c)%d);
        }
    }

    std::ofstream timingResultFile;
    timingResultFile.open("./data/timing/timing-matOps.csv", std::ofstream::out | std::ofstream::app);
    timingResultFile << d << ",";

    // Time to Encode Matrices
    auto start = std::chrono::high_resolution_clock::now();
    Panther::Encoder encoder;
    Panther::Dataset<int> encodedMat1 = encoder.encodeMatrix(matrix1);
    Panther::Dataset<int> encodedMat2 = encoder.encodeMatrix(matrix2);    
    auto end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    int n = encodedMat1.columns();
    int mults = encodedMat1.rows()/n;
    timingResultFile << n << "," << mults << ",";
    unsigned long plaintext_prime_modulus = 131;
    unsigned long phiM = 21845;//104353;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 256;
    unsigned long numOfColOfKeySwitchingMatrix = 2;

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
    start = std::chrono::high_resolution_clock::now();
    std::vector<helib::Ctxt> sums = xcode.matrixAdd(encryptor, mat1shards, mat2shards);
    end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    start = std::chrono::high_resolution_clock::now();
    helib::Ctxt prod = xcode.matrixMult(encryptor, mat1shards, mat2shards, d);
    end = std::chrono::high_resolution_clock::now();
    timingResultFile << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << ",";

    timingResultFile << "\n";
    timingResultFile.close();
}

int main(int argc, char **argv) {

    std::cout << "Program Start!!!" << std::endl;
    // decimal_arithmetic_example();
    int mode = std::atoi(argv[1]);
    int n, c1, c2, n_S, n_R, d;
    unsigned long p;
    unsigned long phiM;
    switch(mode){
        case 0:
            p = (unsigned long) std::atoi(argv[2]);
            test_slot_counts(p);
            break;
        case 1:
            n = std::atoi(argv[2]);
            // mults = std::atoi(argv[3]);
            time_functions_one(n);
            break;
        case 2:
            n = std::atoi(argv[2]);
            // mults = std::atoi(argv[3]);
            c1 =  std::atoi(argv[3]);//2;
            c2 = std::atoi(argv[4]);//n-1;
            time_functions_two(n, c1, c2);
            break;
        case 3:
            n = std::atoi(argv[2]);
            time_functions_three(n);
            break;
        case 4:
            n_S = std::atoi(argv[2]);
            n_R = std::atoi(argv[3]);
            psi(n_S, n_R);
            break;
        case 5:
            d = std::atoi(argv[2]);
            matrix(d);
            break;
        default:
            break;
    }

    std::cout << "Program Finished!!!" << std::endl;


    // std::cout << std::endl;
}
