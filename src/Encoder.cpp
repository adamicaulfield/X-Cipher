//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#include "Encoder.h"
#include <iostream>
#include "util.h"
#include <fstream>
#include "Dataset.h"
#include <bitset>
#include <math.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

Panther::Encoder::Encoder(){
    originalSize = 0;
    height = 0;
    length = 0;
    header = "";
}

/** PNG to BINARY **/

Panther::Dataset<bool>
Panther::Encoder::encodeImage(std::string fileName, int n){
    // std::ifstream input( "data/input/172.pgm", std::ios::binary);
    std::ifstream input(fileName, std::ios::binary);

    // copies all data into buffer
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
    originalSize = buffer.size();
    // //Print data as unsigned char
    // for(int c=0; c<buffer.size(); c++){
    //     std::cout << buffer[c] << " ";// << std::endl;
    // }
    // std::cout << std::endl;
    
    std::cout << "Data Size in Bytes: " << originalSize << std::endl;
    std::cout << std::endl;
    
    //Default
    height = 28;
    length = 28;

    std::vector<std::bitset<32>> bitData(originalSize);
    for(int i=0; i<originalSize; i++){
        std::bitset<32> bin(buffer[i]);
        bitData[i] = bin;
    }

    //Print binary data
    for(int i=0; i<bitData.size(); i++){
        std::cout << bitData[i] << " ";
    }
    std::cout << std::endl;

    int cols = n;
    int rows = 8*bitData.size()/cols + 1;
    std::cout << "Dataset Size (Rows,Cols) = (" << rows << "," << cols << ")" << std::endl;
    Panther::Dataset<bool> datasetPNG(rows, cols);
    
    int r = 0;
    int c = 0;
    int bitsetInd = 0;
    int ind = 0;
    
    for(int i=0; i<originalSize*8; i++){
        datasetPNG.assign(r,c,bitData[bitsetInd][ind]);
        ind++;
        c++;
        if(ind == 8){
            ind = 0;
            bitsetInd++;
        }
        if(c == cols){
            c = 0;
            r++;
        }
    }

    return datasetPNG;
}

void
Panther::Encoder::decodeImage(std::string fileName, Panther::Dataset<bool> dataset){
    std::cout << "Decoding dataset into image" << std::endl;
    int size = dataset.rows()*dataset.columns();
    std::vector<std::bitset<8>> bitData(size);


    int bitsetInd = 0;
    int ind = 0;
    int r = 0;
    int c = 0;
    int cols = dataset.columns();
    for(int i=0; i<size; i++){
        bitData[bitsetInd][ind] = dataset.get(r,c);

        ind++;
        c++;

        if(ind == 8){
            ind = 0;
            bitsetInd++;
        }
        if(c == cols){
            c = 0;
            r++;
        }
    }

    std::ofstream out(fileName, std::ios::out|std::ios::binary);
    if(out){

        out << "P5\n"+std::to_string(length)+"\t"+std::to_string(height)+"\n255\n";
        unsigned char nextByte;
        for(int i=13; i<size/8; i++){
            nextByte = static_cast<unsigned char>(bitData[i].to_ulong()); 
            // std::cout << i << ":" << nextByte << std::endl;
            out << nextByte;
        }
        std::cout << std::endl;
        out << std::endl;
    }
    
    std::cout << "wrote to: " << fileName << std::endl;
}

void
Panther::Encoder::encodeImageToInt(std::string filename){
    //assumes pgm format

    std::string line;
    std::ifstream infile(filename);
    int newlineCount = 0;
    char pixel[1];
    int count = 1;
    while(std::getline(infile, line)){
        if(newlineCount < 3){
            header = header + line + "\n";
            newlineCount++;
        } else {
            for(char c : line){
                unsigned int byteVal = 0;
                pixel[count] = c;
                // count++;
                // if(count==3){
                //     for(int i=0; i<3; i++){
                //          byteVal += (static_cast<unsigned int>(pixel[i]) << 8*i);
                //     }
                //     count = 0;
                //     std::cout << byteVal << std::endl;
                // }
                byteVal = (int) static_cast<uint8_t>(c);
                
                std::cout << byteVal << " ";
                
            }
        }
    }
    std::cout << std::endl;
    std::cout << "Header: \n" << header << std::endl;

}

/** string to binary functions **/

Panther::Dataset<bool>
Panther::Encoder::encodeChar(char h, int n){
    int numBits = sizeof(h)*8;
    int mults = 1;

    Panther::Dataset<bool> dataset(n*mults, n);
    // std::cout << sizeof(h) << std::endl;
    char bit;
    for(int r = 0; r < n-2; r ++){
        for(int c = 0; c < n; c++){
            if(r*n+c < numBits){
                bit = 1 << (n*r+c);
                dataset.assign(r, c, h&bit);
            }
            else{
                dataset.assign(r, c, 0);
            }
        }
    }
    // dataset.print();
    return dataset;
}

void
Panther::Encoder::decodeChar(Panther::Dataset<bool> dataset){
    int total = 0;
    int incr = 1;
    int rows = dataset.rows();
    int cols = dataset.columns();
    for(int r = 0; r < rows-2; r++){
        for(int c = 0; c<cols; c++){
            if(dataset.get(r,c)==1){
                total += incr;
            }
            incr *= 2;
        }
    }
    std::cout << char(total) << std::endl;
}

/** Expand and compress functions **/
Panther::Dataset<bool>
Panther::Encoder::expandDataset(Panther::Dataset<bool> compressed){
    int rows = compressed.rows();
    int cols = compressed.columns();

    //Default expanded
    height = 33;
    length = 33;

    int newRows = rows/(cols-2);
    newRows = rows + newRows*(rows*2)/cols;

    std::cout << "cols = " << cols << std::endl;
    std::cout << "rows = " << rows << std::endl;
    std::cout << "newRows = " << newRows << std::endl;
    Panther::Dataset<bool> expanded(newRows, cols);

    int ri = 0;
    std::cout << "Starting expansion..." << std::endl;
    for(int r=0; r<newRows; r++){
        if(ri < rows && r%cols < cols-2){

            for(int c=0; c<cols; c++){
                expanded.assign(r,c, compressed.get(ri,c));
            }  
            // std::cout << r << "," << ri << std::endl;
            ri++;
        } else{
            for(int c=0; c<cols; c++){
                expanded.assign(r,c,0);
            }
        }
    }
    
    return expanded;
}

Panther::Dataset<bool>
Panther::Encoder::compressDataset(Panther::Dataset<bool> expanded){
    int rows = expanded.rows();
    int cols = expanded.columns();
    int newRows = rows*(cols-2)/cols;
    Panther::Dataset<bool> compressed(newRows, cols);

    int ri = 0;
    for(int r=0; r<rows; r++){
        if(r%cols < cols-2){
            for(int c=0; c<cols; c++){
                compressed.assign(ri,c, expanded.get(r,c));
            }    
            ri++;
        }   
    }

    return compressed;   
}

Panther::Dataset<int>
Panther::Encoder::expandDataset(Panther::Dataset<int> compressed){
    int rows = compressed.rows();
    int cols = compressed.columns();

    std::vector<int> smallXCodePrimes = {5, 7, 11, 13};
    int n = smallXCodePrimes[0];
    int ind = 0;
    int m = 1;
    while(m*n < cols){ // 5 7 11 13 10 14
        ind++;
        if(ind == 4){
            m++;
            ind = 0;
        }
        n = smallXCodePrimes[ind];
    }

    int mults = rows/(n-2);
    if(rows%(n-2)>0){
        mults++;
    }
    int newRows = n*mults;

    printf("n=%d, mults=%d\n", n, mults);

    Panther::Dataset<int> expanded(newRows, cols);
    printf("NewRows: %d\n", newRows);
    std::cout << "Starting expansion..." << std::endl;
    int ri = 0;
    for(int r=0; ri<rows; r++){
        if(r%n < n-2){
            for(int c=0; c<cols; c++){
                
                expanded.assign(r,c, compressed.get(ri,c));
            }  
            // std::cout << r << "," << ri << std::endl;
            ri++;
        }
        
    }
    
    return expanded;
}

Panther::Dataset<int>
Panther::Encoder::compressDataset(Panther::Dataset<int> expanded){
    int rows = expanded.rows();
    int cols = expanded.columns();
    int newRows = rows*(cols-2)/cols;
    Panther::Dataset<int> compressed(newRows, cols);

    int ri = 0;
    for(int r=0; r<rows; r++){
        if(r%cols < cols-2){
            for(int c=0; c<cols; c++){
                compressed.assign(ri,c, expanded.get(r,c));
            }    
            ri++;
        }   
    }

    return compressed;   
}

/** Encode Matrix **/
Panther::Dataset<int>
Panther::Encoder::encodeMatrix(Panther::Dataset<int> matrix){
    //Needs 3 + 2*(d-1) = 2d+3-2 = 2d+1 ciphertexts

    int d = matrix.rows();
    Panther::Dataset<int> allTransforms(d*d, 2*d+1);

    for(int i=0; i<d*d; i++){
        int r = i/d;
        int c = i%d;    

        //Row-wise encoding of matrix
        allTransforms.assign(i,0,matrix.get(r,c));

        //Row-wise encoding of sigma transformation
        allTransforms.assign(i, 1, matrix.get(r, (r+c)%d));

        //Row-wise encoding of tau transformation
        allTransforms.assign(i, 1+d, matrix.get((r+c)%d, c));
    }

    for(int i=0; i<d*d; i++){
        int r = i/d;
        int c = i%d;
        int row;
        for(int k=1; k<d; k++){
            //Row-wise encoding of phi transformation
            row = r*d+(c+k)%d;
            allTransforms.assign(i, 1+k, allTransforms.get(row, 1));

            //Row-wise encoding of psi transformation
            row = ((r+k)%d)*d+c;
            allTransforms.assign(i, 1+d+k, allTransforms.get(row, 1+d));
        }
    }

    allTransforms = expandDataset(allTransforms);
    return allTransforms;
}

/** Encode for PSI functions **/
std::vector<int> Panther::Encoder::getPSIParams(int n_R, int n_S){
    std::vector<int> smallXCodePrimes = {5, 7, 11, 13};

    int max = 20;
    while(n_S % max != 0 && max > 2){
        max--;
    }
    int totalSenderSplits = n_S/max;

    int cols = n_R+totalSenderSplits;
    // printf("cols=%d, \n", cols);
    int n = smallXCodePrimes[0];
    int ind = 0;
    int m = 1;
    while(m*n < cols){ // 5 7 11 13 10 14
        ind++;
        if(ind == 4){
            m++;
            ind = 0;
        }
        n = smallXCodePrimes[ind];
    }
    // printf("n = %d, m=%d\n", n, m);

    int mults = (max+1)/(n-2);
    if((max+1)%(n-2)!=0){
        mults++;
    }
    // printf("n = %d, mults = %d\n", n, mults);

    std::vector<int> params(4);
    params[0] = n;
    params[1] = mults;
    params[2] = max;
    params[3] = totalSenderSplits;
    return params;
}

Panther::Dataset<int> Panther::Encoder::encodePSI(std::vector<int> senderSet, std::vector<int> receiverSet, int plaintext_prime_modulus, int n, int mults, int max, int totalSenderSplits){
    srand(time(NULL));

    int n_S = senderSet.size();
    int n_R = receiverSet.size();

    int cols = n_R+totalSenderSplits;

    int dcols = cols;
    while(dcols%n!=0){
        dcols++;
    }

    Panther::Dataset<int> dataset(mults*n, dcols);

    /**** SENDER SIDE ****/
    for(int sub=0; sub<totalSenderSplits; sub++){
        std::vector<int> subset(senderSet.begin()+sub*max, senderSet.begin()+(sub+1)*max);

        std::vector<int> senderOptCoeff = getSenderOptCoeffs(max, subset, plaintext_prime_modulus);
        // int r = rand() % 100;
        int r = 1;

        //mask sender coeffs with random nums
        for(int x = 0; x<senderOptCoeff.size(); x++){
            senderOptCoeff[x] = mod((r*senderOptCoeff[x]),plaintext_prime_modulus);
        }

        int count = 0;
        int c = n_R+sub;
        for(int r=0; count<max+1; r++){
            if(r%n!=n-2 && r%n!=n-1){
                dataset.assign(r,c,senderOptCoeff[count]);
                count++;
            }
        }
    }
    

    /**** RECEIVER SIDE ****/
    std::vector<unsigned int> receiverDataExpanded(n_R*(max+1));

    for(int x = 0; x<n_R; x++){
        for(int i = 0; i<=max; i++){
            receiverDataExpanded[x*(max+1)+i] = modPow(receiverSet[x],max-i,plaintext_prime_modulus);
        }
    }

    for(int c=0; c<n_R; c++){
        int count=0;
        for(int r=0; count<max+1; r++){
            if(r%n!=n-2 && r%n!=n-1){
                // if(c==cols-1){
                    // dataset.assign(r,c,senderOptCoeff[count]);
                // } else{
                dataset.assign(r,c,receiverDataExpanded[c*(max+1)+count]);
                // }
                count++;
            }
        }
    }


    return dataset;
}

std::vector<int> Panther::Encoder::getSenderOptCoeffs(int n, std::vector<int> senderCoeff, int plaintext_prime_modulus){
    util::debug("Starting setup sender optimized coefficients");
    std::vector<int> optimizedCoeff(n+1);
    
    int sum, prod, k;
    #pragma parallel for private(k, sum, prod) shared(optimizedCoeff)
    for(k=n; k>=0; k--){
        
        // printf("%d\n",k);
        if(k==0){
            sum = 1;
        }
        else{
            sum = 0;
            int t = n_chose_k(n,k);
            Panther::Dataset<int> all_combo = getCombinations(n,k,t);
            // printf("Done k=%d\n",k);
            // if(k<2){
            //     printf("All combos at k=%d, t=%d, n=%d\n", k, t, n);
            //     all_combo.print();
            // }

            for(int i=0; i<t; i++){
                prod = 1;
                for(int j=0; j<k; j++){
                    if(all_combo.get(i,j)<senderCoeff.size()){
                        // prod = (-1)*mod((prod*senderCoeff[all_combo.get(i,j)]), plaintext_prime_modulus);
                        prod = (prod*(-1)*senderCoeff[all_combo.get(i,j)])%plaintext_prime_modulus; //negative because subtracting
                    }
                }
                // sum = mod(sum+prod,plaintext_prime_modulus);
                sum = (sum+prod)%plaintext_prime_modulus;

                // if(k==n){
                //     printf("k=%d, sum=%d\n", k, sum);
                // }
            }
        }
        // printf("\tCompleted sum: optCoef[%d] = %d\n",k, sum);
        optimizedCoeff[k] = sum;
    }
    util::debug("Done getting coeff");
    return optimizedCoeff;
}

Panther::Dataset<int> Panther::Encoder::getCombinations(int n, int k, int t){
    // util::debug("Starting getCombinations...");

    std::vector<int> elt(k);
    std::vector<int> max_val(k);
    Panther::Dataset<int> all_combo(t, k);;
    // all_combo.resize(t, std::vector<int>(k, 0));

    for(int i=0; i<k; i++){
        elt[i] = i;
        max_val[i] = n-(k-i);
    }

    int count = 0;
    bool checking;
    int check_ind;
    while(count<t){
        check_ind = k-1;
        checking = true;
        // for(int i=0; i<k; i++){
            // printf("%d ", elt[i]);
        // }
        // printf("\n");

        for(int i=0; i<k; i++){
            all_combo.assign(count,i, elt[i]);
        }


        elt[k-1]++;
        // printf("\telt[k-1]++ --> elt[k-1]=%d\n", elt[k-1]);
        while(checking){
            // printf("\tchecking %d>%d\n", elt[check_ind], max_val[check_ind]);
            if(check_ind < 0){
                checking = false;
            }
            else{
                if(elt[check_ind]>max_val[check_ind]){
                    // printf("\tTRUE decrementing check_ind from %d to %d\n", check_ind, check_ind-1);
                    check_ind--;
                    if(check_ind>=0){
                        elt[check_ind]++;
                        // printf("\tIncrementing elt[%d]to %d\n", check_ind, elt[check_ind]);
                    }
                }
                else{
                    // printf("\tFALSE\n");
                    checking = false;
                }
            }
        }

        int c = check_ind+1;
        // printf("\tEntering final while loop with c=%d and check_ind = %d\n", c, check_ind);
        while(0<c && c<k){
            // printf("\tChanging elt[%d] from %d to %d\n",c, elt[c], elt[c-1]+1);
            elt[c] = elt[c-1]+1;
            c++;
        }

        count++;
        
    }
    // util::debug("Done getCombinations.");
    return all_combo;
}

int Panther::Encoder::n_chose_k(int n, int k){
    // util::debug("Starting n_chose_k.");
    int result = 1;
    for(int i=1; i<=k; i++){
        result = (int) (result * (n-(k-i))/i);
    }
    // util::debug("Done n_chose_k.");
    return result;
}

int Panther::Encoder::modPow(int a, int b, int p){
    // returns a^b mod p
    unsigned int result = 1;
    for(int i=0; i<b; i++){
        result = mod(result * a, p);
    }

    return result;
}