//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#include "XCode.h"
#include "util.h"
#include "Cache.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <omp.h>

template<class T>
Panther::XCodeTableBase<T>::XCodeTableBase(int cols, int rows) : _columns(cols), _rows(rows) {
}

template<class T>
T
Panther::XCodeTableBase<T>::get(int colID, int rowID) {
    return xcodeTable[colID][rowID];
}

template<class T>
void
Panther::XCodeTableBase<T>::assign(int colID, int rowID, T val) {
    xcodeTable[colID][rowID] = val;
}

template<class T>
void
Panther::XCodeTableBase<T>::print() {
//    printf("Horizontal Columns: \n");
//    for (int c = 0; c < _columns; ++c) {
//        for (int r = 0; r < _rows; ++r) {
//            std::cout << xcodeTable[c][r] << "  ";
//        }
//        std::cout << std::endl;
//    }

    printf("Horizontal Rows: \n");
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _columns; ++c) {
            std::cout << xcodeTable[c][r] << "  ";
        }
        std::cout << std::endl;
    }
}

template<class T>
void
Panther::XCodeTableBase<T>::swtichLastTwoRows(int n, int mults) {
    T temp;
    for (int r = 0; r < mults; r++) {
        int thirdDataRow = n * (r + 1) - 1;
        for (int c = 0; c < _columns; ++c) {
            temp = xcodeTable[c][thirdDataRow];
            xcodeTable[c][thirdDataRow] = xcodeTable[c][thirdDataRow - 1];
            xcodeTable[c][thirdDataRow - 1] = temp;
        }
    }
}

/***/
template<class T>
Panther::XCodeLabelTable<T>::XCodeLabelTable(int cols, int rows) : XCodeTableBase<T>(cols, rows) {
    for (int c = 0; c < cols; ++c) {
        std::vector<T> column;
        for (int r = 0; r < rows; ++r) {
            column.push_back(std::to_string(r) + ',' + std::to_string(c));
        }
        this->xcodeTable.push_back(column);
    }
}

template<class T>
Panther::XCodeDataTable<T>::XCodeDataTable(int cols, int rows) : XCodeTableBase<T>(cols, rows) {
    for (int c = 0; c < cols; ++c) {
        std::vector<T> column(rows);
        this->xcodeTable.push_back(column);
    }
}

template<class T>
Panther::XCode<T>::XCode(int n, int mults, int p) : n(n), mults(mults), p(p){
    this->part_1_labelTable = new Panther::XCodeLabelTable<std::string>(n, n*mults);
    this->part_2_labelTable = new Panther::XCodeLabelTable<std::string>(n, n*mults);
}

template<class T>
Panther::XCodeLabelTable<std::string> *
Panther::XCode<T>::getPart1LabelTable() {
    return part_1_labelTable;
}

template<class T>
Panther::XCodeLabelTable<std::string> *
Panther::XCode<T>::getPart2LabelTable() {
    return part_2_labelTable;
}

template<class T>
void
Panther::XCode<T>::printPart1LabelTable() {
    part_1_labelTable->print();
}

template<class T>
void
Panther::XCode<T>::printPart2LabelTable() {
    part_2_labelTable->print();
}

template<class T>
void
Panther::XCode<T>::encode(std::vector<Panther::Shard<T>> &shards, Panther::Dataset<T> &dataset) {
    int cols = n, rows = n * mults;
    util::info("Beginning X-Code Encoding");
    // printf("Filling xCodeDataTable cols=%d rows=%d\n", cols, rows);
    // Dataset<T> parityLarge(mults*n, n);
    // std::vector<long> parity(2*n*mults);

    Panther::XCodeDataTable<T> xCodeDataTable(cols, rows);
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows - 2; ++r) {
            xCodeDataTable.assign(c, r, dataset.get(r, c));
        }
    }
    // xCodeDataTable.print();

    // calculate parity bits
    // printf("Calculating Parity Bits now: \n");
    T part1Tmp, part2Tmp;
    for (int i = 0; i < mults; i++) {
        int part1RowID = n * (i + 1) - 2;
        int part2RowID = n * (i + 1) - 1;

        for (int c = 0; c < cols; ++c) {
            part1Tmp = xCodeDataTable.get(mod(c + 1, cols), mod(part1RowID - 1, rows));
            part2Tmp = xCodeDataTable.get(mod(c - 1, cols), mod(part2RowID - 2, rows));

            //If int, take sum. If not int, take XOR
            if(typeid(T)==typeid(int)){
                for (int r = 2; r < n - 1; ++r) {
                    part1Tmp += xCodeDataTable.get(mod(c + r, cols), mod(part1RowID - r, rows));
                    part2Tmp += xCodeDataTable.get(mod(c - r, cols), mod(part2RowID - r - 1, rows));
                }
                part1Tmp = part1Tmp%p;
                part2Tmp = part2Tmp%p;
            } else{
                for (int r = 2; r < n - 1; ++r) {
                    part1Tmp ^= xCodeDataTable.get(mod(c + r, cols), mod(part1RowID - r, rows));
                    part2Tmp ^= xCodeDataTable.get(mod(c - r, cols), mod(part2RowID - r - 1, rows));
                }
            }

            xCodeDataTable.assign(c, part1RowID, part1Tmp);
            xCodeDataTable.assign(c, part2RowID, part2Tmp);

            // parityLarge.assign(part1RowID, c, part1Tmp);
            // parityLarge.assign(part2RowID, c, part2Tmp);

            // parity[c+n*(i+i%2)] = xCodeDataTable.get(c, n*(i+1)-2);
            // parity[c+n*(i+1+i%2)] = xCodeDataTable.get(c, n*(i+1)-1);
        }
    }
    //    util::debug("Calculated parity bits.");
    // xCodeDataTable.print();

    //    Fill dataset obj with parity for testing
    for (int i = 0; i < mults; i++){
        for (int c = 0; c < cols; c++) {
            dataset.assign(n * (i + 1) - 2, c, xCodeDataTable.get(c, n * (i + 1) - 2));
            dataset.assign(n * (i + 1) - 1, c, xCodeDataTable.get(c, n * (i + 1) - 1));
        }
    }

    int rotationCnt = 0;
    Panther::XCodeLabelTable<std::string> originalLabelMap(*part_1_labelTable);
    for (int c = 0; c < cols; ++c) {
        Panther::Shard<T> shard;
        for (int r = 0; r < rows; ++r) {
            shard.addToPart1(xCodeDataTable.get(c, r));
            part_1_labelTable->assign(c, r, originalLabelMap.get(c, r));

        }
        shards.push_back(shard);
    }

    xCodeDataTable.swtichLastTwoRows(n, mults);
    originalLabelMap.swtichLastTwoRows(n, mults);
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
             shards[c].addToPart2(xCodeDataTable.get(c, r));
            part_2_labelTable->assign(c, r, originalLabelMap.get(c, r));
        }
    }
    // printf("done\n");
    util::info("Done Encoding");
}

/**
*   This method takes the data shards and recalculates the parity and
*   check again with the local copy to match the actual copy
*
*   input parameters: (2)
*       - vector<Shard <T>> shards - vector containing all shards of the X-Code matrix
 *      - std::vector<bool> localCopy - localCopy of the parity bits
*
*   output parameters: True if parity matches, false if parity doesn't match
*
*/
template<class T>
std::vector<helib::Ctxt*> Panther::XCode<T>::verify(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &shards, std::vector<helib::Ctxt*> &tag) {
    util::info("Starting Verification");

    std::vector<helib::Ctxt*> tag_remote(n);

    helib::Ptxt<helib::BGV> mask(*(encryptor.getContext()));
    for(int r=0; r<n*mults; r++){
        if(r%n >= n-2){
            mask[r] = 1;
        } else{
            mask[r] = 0;
        }
    }

    for(int c=0; c<n; c++){
        tag_remote[c] = new helib::Ctxt(*(encryptor.getPublicKey()));
        helib::Ctxt tmpctxt1(*(encryptor.getPublicKey()));

        tmpctxt1 += *(shards[c].getPart1Ctxt());
        tmpctxt1.multByConstant(mask);

        // Subtract
        tmpctxt1 -= *(tag[c]);

        // Multiply        
        tmpctxt1 *= tmpctxt1;

        helib::totalSums(*encryptor.getEncryptedArray(), tmpctxt1);

        *(tag_remote[c]) = tmpctxt1;
    }

    util::info("Done Verification");
    return tag_remote;
}

/**
*   verify2
*   
*   Before execution:
*       User sends [integKey] to evaluator
*       Evaluator uses [integKey] and [shard]s to produce [newTag], sends [newTag] to user
*
*   Verify2 executed by user:
*       Receives [newTag] from Evaluator
*       Decrypts both tag and newTag
*       Returns true if tag=newTag at all slots
*       Returns false if not
*/
template<class T>
bool Panther::XCode<T>::verify2(Panther::Encryptor &encryptor, helib::Ctxt newTag_ctxt, helib::Ctxt tag_ctxt){
    // get slot count
    int nslots = encryptor.getSlotCount();
    // initialize plaintext vectors
    std::vector<long> newTag(nslots);
    std::vector<long> tag(nslots);

    // decrypt ctxts into ptxt vectors
    encryptor.getEncryptedArray()->decrypt(newTag_ctxt, *(encryptor.getSecretKey()), newTag);
    encryptor.getEncryptedArray()->decrypt(tag_ctxt, *(encryptor.getSecretKey()), tag);

    // Iterate through newTag and tag checking equality of each slot val
    for(int i=0; i<nslots; i++){
        if(newTag[i]!=tag[i]){
            //Exit function returning false upon first slot vals that has inequality
            return false;
        }
    }    

    // Return true of all slots vals are equal
    return true;
}

// Local verify by recomputing codewords
template<class T>
bool Panther::XCode<T>::simpleVerify(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &shards){
    
    Panther::Dataset<T> verDataset(mults*n, n);
    std::vector<Panther::Shard<T>> verShards;

    std::vector<long> shardPtxt(encryptor.getEncryptedArray()->size());

    int i, j;
    for(i=0; i<n; i++){
        encryptor.decryptWrapper(*(shards[i].getPart1Ctxt()), &shardPtxt);

        for(j=0; j<n*mults; j++){
            if(j%n < n-2){
                verDataset.assign(j, i, (T) shardPtxt[j]);
            }
        }
    }
    printf("VerDataset (dataset without codewords): \n");
    verDataset.print();

    encode(verShards, verDataset);
    
    bool passVerify = true;
    
    for(i=0; i<n; i++){
        for(j=0; j<n*mults; j++){
            if(j%n >= n-2){
                int shardVal = shards[i].getPart1Ptxt()[j];
                int verShardVal = verShards[i].getPart1Ptxt()[j];
                if(shardVal != verShardVal){
                    printf("at (%d, %d): %d != %d", i, j, shardVal, verShardVal);    
                    passVerify = false;
                    break;
                }
            }
        }
    }
    printf("\n");

    return passVerify;
}

/**
*   integrityTagGen
*   
*   Given the encrypted columns (shards), and encrypted integrityKey, returns the encrypted tag as: tag = sum(shards)+integKey
*/
template<class T>
helib::Ctxt Panther::XCode<T>::integrityTagGen(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &shards, helib::Ctxt integKey){
    helib::Ctxt integTag(*(encryptor.getPublicKey()));

    integTag += integKey;
    for(int c=0; c<n; c++){
        integTag += *(shards[c].getPart1Ctxt());
    }

    return integTag;
}

/**
*   integrityKeyGen
*   
*   Given a column is mults*n in length and plaintext prime modulus of p:
*   Returns a ciphertext integKey which encrypts a random vector R^{n*mults}_p
*/
template<class T>
helib::Ctxt Panther::XCode<T>::integrityKeyGen(Panther::Encryptor &encryptor){
    helib::Ctxt integKey(*(encryptor.getPublicKey()));

    helib::Ptxt<helib::BGV> intKey_ptxt(*(encryptor.getContext()));
    std::cout << "Integrity key: " << std::endl;
    for(int i=0; i<n*mults; i++){
        intKey_ptxt[i] = rand()%p;
        std::cout << intKey_ptxt[i] << " ";
    }
    printf("\n");

    encryptor.getPublicKey()->Encrypt(integKey, intKey_ptxt);

    return integKey;  
}

template<class T>
helib::Ctxt Panther::XCode<T>::convolution(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &shards, int aInd, int wInd){
    util::info("Begining Convolution...");
    // extract from structure
    helib::Ctxt c_A = *(shards[aInd].getPart1Ctxt());
    helib::Ctxt c_W = *(shards[wInd].getPart1Ctxt());

    // create output ctxt
    helib::Ctxt c_B(*(encryptor.getPublicKey()));

    //dimensions
    int dimA = 5;
    int dimW = 3;
    int dimB = dimA-dimW+1;

    //variables
    int subMat = 0;

    // Mask out the codewords
    helib::Ptxt<helib::BGV> mask(*(encryptor.getContext()));
    for(int r=0; r<n*mults; r++){
        if(r%n==n-2 || r%n==n-1){
            mask[r]=0;
        } else{
            mask[r]=1;
        }
    }
    c_A.multByConstant(mask);
    c_W.multByConstant(mask);

    // Loop through all submatrices
    while(subMat < dimB*dimB){
        int startRow = subMat%dimW;
        int startCol = subMat/dimW;
        
        int count = 0;
        int rotAmt = 0;
        int i=0; 
        int j=0;

        //get starting slot
        int startSlot=0;
        for(int r=0; r<n*mults; r++){
            if(r%n!=n-2 && r%n!=n-1){
                
                if(count == dimA*(subMat/dimW)+subMat%dimW){ //2 for 2 codeword rows
                    startSlot = r;
                    break;
                } else{
                    count++;
                }
                i++;
                if(i==dimA){    
                    i = 0;
                    j++;
                }
            }   
        }
        // printf("startSlot=%d, i=%d, j=%d\n", startSlot, i, j);
        count = 0;
        helib::Ctxt c_Prod(*(encryptor.getPublicKey()));

        //make mask
        int lastInd = startSlot;
        helib::Ptxt<helib::BGV> u(*(encryptor.getContext()));
        while(count < dimW * dimW){
            // printf("count=%d\n", count);
            helib::Ctxt c_W_tmp(*(encryptor.getPublicKey()));
            helib::Ptxt<helib::BGV> u_tmp(*(encryptor.getContext()));
            u = u_tmp;
            for(int r=lastInd; r<n*mults; r++){
                
                if(count!=0 && r%n==n-2){
                    lastInd += 2;
                    // printf("hit BREAK at slot=%d, lastInd=%d\n", r, lastInd);
                    break;
                } else{
                    
                    if(i >= startRow && i < startRow+dimW && j >= startCol && j < startCol+dimW){ 
                        // i%dimW >= subMat && i%dimW < subMat + dimW && j/dimW >= subMat && j/dimW < subMat + dimW
                        u[r]=1;
                        count++;
                        // printf("\tu[r] set to 1 at r=%d\n", r);
                    } else{
                        u[r]=0;
                        // printf("\tset to 0 at r=%d\n", r);
                    }

                    i++;
                    if(i==dimA){    
                        i = 0;
                        j++;
                    }
                }
 
                lastInd++;
            }

            //accumulate section of submat and filter prod
            c_W_tmp = c_W;
            encryptor.getEncryptedArray()->rotate(c_W_tmp, 2*rotAmt+startSlot);
            c_W_tmp.multByConstant(u);
            
            c_W_tmp.multiplyBy(c_A);
            
            c_Prod += c_W_tmp;

            rotAmt++;
        }

        // Add total into   slot b_subMat
        helib::totalSums(*encryptor.getEncryptedArray(), c_Prod);
        count = -1;
        for(int i=0; i<n*mults; i++){
            if(i%n!=n-1 && i%n!=n-2){
                count++;
            }

            if(count==subMat){
                u[i]=1;
                count++;
            } else{
                u[i]=0;
            }
 
        }
        c_Prod.multByConstant(u);
        c_B += c_Prod;
        printf("[CONV] Done sub matrix %d of %d...\n", subMat+1, dimB*dimB);
        subMat++;
    }
    util::info("Done Convolution");
    return c_B;
}

/**
*   This method rotates the columns based upon the specified direction
*
*   input parameters: (3)
*       - Encryptor &encryptor - shared encryptor object
*       - vector<Shard <T>> shards - vector containing all shards of the X-Code matrix
*       - Direction - forward (rotate into recovery position) or backwards (rotate into original positions)
*
*   output parameters: (None)
*  
*/
template<class T>
void
Panther::XCode<T>::rotateColumns(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &shards, Direction dir) {

    Panther::XCodeLabelTable<std::string> originalLabelMap(*part_1_labelTable);

    int c;

    #pragma omp parallel for shared(shards, encryptor, originalLabelMap) private(c)
    for(c=0; c<n; c++){
        printf("[INFO] Rotating Shard 1-%d...\n",c);
        
        int leftRotCount;
        int rightRotCount;

        helib::Ptxt<helib::BGV> left(*(encryptor.getContext()));
        helib::Ptxt<helib::BGV> right(*(encryptor.getContext()));

        helib::Ctxt rightCtxt = helib::Ctxt(*(encryptor.getPublicKey()));
        helib::Ctxt leftCtxt = helib::Ctxt(*(encryptor.getPublicKey()));

        switch(dir){
            case forward:
                leftRotCount = n-1-c;
                break;
            case backward:
                leftRotCount = c+1;
                break;
        }

        rightRotCount = n - leftRotCount;

        for(int r=0; r<mults*n; r++){
            if(r%n<leftRotCount){
                left[r] = 1;
                right[r] = 0;
            } else {
                left[r] = 0;
                right[r] = 1;
            }
            part_1_labelTable->assign(c, r, originalLabelMap.get(c, (r/n)*n+(r + leftRotCount) % n));
        }

        leftCtxt = *(shards[c].getPart1Ctxt());
        rightCtxt = *(shards[c].getPart1Ctxt());

        leftCtxt.multByConstant(left);
        rightCtxt.multByConstant(right);

        encryptor.getEncryptedArray()->rotate(rightCtxt, -1 * leftRotCount);
        encryptor.getEncryptedArray()->rotate(leftCtxt, rightRotCount);
        rightCtxt += leftCtxt;
        shards[c].setPart1(rightCtxt);

        std::string tmp;
        for(int m = 0; m<mults; m++){
            tmp = originalLabelMap.get(c,m*n+n - 2);
            originalLabelMap.assign(c,m*n+n - 2, originalLabelMap.get(c,m*n+n - 1));
            originalLabelMap.assign(c,m*n+n - 1, tmp);
        }

        printf("[INFO] Rotating Shard 2-%d...\n", c);
        
        switch(dir){
            case forward:
                leftRotCount = c;
                break;
            case backward:
                leftRotCount = n-c;
                break;
        }

        rightRotCount = n - leftRotCount;

        for(int r=0; r<n*mults; r++){
            if(r%n<leftRotCount){
                left[r] = 1;
                right[r] = 0;
            } else {
                left[r] = 0;
                right[r] = 1;
            }

            part_2_labelTable->assign(c, r, originalLabelMap.get(c, (r/n)*n+(r + leftRotCount) % n));
        }

        leftCtxt = *(shards[c].getPart2Ctxt());
        rightCtxt = *(shards[c].getPart2Ctxt());

        leftCtxt.multByConstant(left);
        rightCtxt.multByConstant(right);

        encryptor.getEncryptedArray()->rotate(rightCtxt, -1 * leftRotCount);
        encryptor.getEncryptedArray()->rotate(leftCtxt, rightRotCount);
        rightCtxt += leftCtxt;
        shards[c].setPart2(rightCtxt);
    }
}

template<class T>
void
Panther::XCode<T>::refreshTags(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &shards) {
    HELIB_TIMER_START;
    util::info("Starting Refresh Tags");
    int rows = mults*n, cols = n;
    int colID;
    rotateColumns(encryptor, shards, Panther::Direction::forward);
    #pragma omp parallel for shared(shards, encryptor) private(colID)
    for(colID=0; colID<n; colID++){
        helib::Ctxt recoveredShard_part1(*(encryptor.getPublicKey()));
        helib::Ctxt recoveredShard_part2(*(encryptor.getPublicKey()));

        helib::Ptxt<helib::BGV> bitmask(*(encryptor.getContext()));
        helib::Ptxt<helib::BGV> otherpart_bitmask(*(encryptor.getContext()));
        std::vector<long> tmp(encryptor.getEncryptedArray()->size());

        helib::Ctxt tmpctxt(*(encryptor.getPublicKey()));
        helib::Ctxt bitmaskctxt(*(encryptor.getPublicKey()));

        // Part 1 codeword
        for(int c=0; c<cols; c++){
            if(c!=colID){    
                for(int r=0; r<rows; r++){ 
                    if(c==r%n){
                        bitmask[r] = 0;
                        // printf("0 ");
                    } 
                    else{
                        if(colID==mod(r+1,n)){
                            bitmask[r] = 1;
                        }
                        else{
                            bitmask[r] = 0;
                            // printf("1 ");
                        }
                    }
                }
                // printf("\t");
                tmpctxt = *(shards[c].getPart1Ctxt());
                tmpctxt.multByConstant(bitmask);
                recoveredShard_part1+=tmpctxt;
            }
        }

        // Part 2 codeword
        for(int c=0; c<cols; c++){
            if(c!=colID){   
                // printf("Bitmask for col=%d: ", c);     
                for(int r=0; r<rows; r++){
                    if(c+r%n==n-1){
                        bitmask[r] = 0;
                        // printf("0 ");
                    } else{
                        if(mod((colID+r),n)==n-2){
                             bitmask[r] = 1;
                        }
                        else{
                            bitmask[r] = 0;
                        }
                    }
                }
                // printf("\t");
                tmpctxt = *(shards[c].getPart2Ctxt());
                tmpctxt.multByConstant(bitmask);
                recoveredShard_part2+=tmpctxt;
            }
        }

        //Transfer recovered Part 1 parity to correct location in recovered Part 2
        int from = ((n-1)+colID)%n;
        int to = (n-1)-colID;

        for(int r=0; r<rows; r++){
            if(r%n==from){
                otherpart_bitmask[r] = 1;
            }
            else{
                otherpart_bitmask[r] = 0;
            }
        }
        tmpctxt = recoveredShard_part1;
        tmpctxt.multByConstant(otherpart_bitmask);
        encryptor.getEncryptedArray()->rotate(tmpctxt,to-from);
        recoveredShard_part2 += tmpctxt;

        //Transfer recovered Part 1 parity to correct location in recovered Part 2
        from = ((n-1)+(n-1-colID))%n;
        to = colID;
        for(int r=0; r<rows; r++){
            if(r%n==from){
                otherpart_bitmask[r] = 1;
            }
            else{
                otherpart_bitmask[r] = 0;
            }
        }
        tmpctxt = recoveredShard_part2;
        tmpctxt.multByConstant(otherpart_bitmask);
        encryptor.getEncryptedArray()->rotate(tmpctxt,to-from);
        recoveredShard_part1 += tmpctxt;

        // Final bit mask to add new tags into columns
        // printf("new masks\n");
        for(int r = 0; r<rows; r++){
            if(r%n==colID){
                //p_(n-1)
                bitmask[r] = 1;
                otherpart_bitmask[r] = 0;

                //p_(n-2)
                bitmask[mod((r-1),n)] = 1;
                otherpart_bitmask[mod((r-1),n)] = 0;
            } else {
                bitmask[r] = 0;
                otherpart_bitmask[r] = 1;
            }
        }

        //No return, just set the shards to the recovered values
        tmpctxt = *(shards[colID].getPart1Ctxt());
        tmpctxt.multByConstant(otherpart_bitmask);
        recoveredShard_part1.multByConstant(bitmask);
        tmpctxt+=recoveredShard_part1;
        shards[colID].setPart1(tmpctxt);    


        for(int r = 0; r<rows; r++){
            if(mod(colID+r,n)==n-1){
                //p_(n-1)
                bitmask[r] = 1;
                otherpart_bitmask[r] = 0;

                //p_(n-2)
                bitmask[mod((r-1),n)] = 1;
                otherpart_bitmask[mod((r-1),n)] = 0;
            } else {
                bitmask[r] = 0;
                otherpart_bitmask[r] = 1;
            }
        }

        tmpctxt = *(shards[colID].getPart2Ctxt());
        tmpctxt.multByConstant(otherpart_bitmask);
        recoveredShard_part2.multByConstant(bitmask);
        tmpctxt+=recoveredShard_part2;
        shards[colID].setPart2(tmpctxt);    

    }
    rotateColumns(encryptor, shards, Panther::Direction::backward);
    HELIB_TIMER_STOP;

    util::info("Done Refreshing Tags");
}

template<class T>
void
Panther::XCode<T>::recoverFromOneColumnErasure(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &shards,
                                               int colID) {
    
    HELIB_TIMER_START;
    util::info("Starting One-Col Recovery");
    int rows = mults*n, cols = n;
    helib::Ctxt recoveredShard_part1(*(encryptor.getPublicKey()));
    helib::Ctxt recoveredShard_part2(*(encryptor.getPublicKey()));

    helib::Ptxt<helib::BGV> bitmask(*(encryptor.getContext()));
    helib::Ptxt<helib::BGV> otherpart_bitmask(*(encryptor.getContext()));
    std::vector<long> tmp(encryptor.getEncryptedArray()->size());

    helib::Ctxt tmpctxt(*(encryptor.getPublicKey()));
    helib::Ctxt bitmaskctxt(*(encryptor.getPublicKey()));

    //Recover bits using part 1 shards
    for(int c=0; c<cols; c++){
        if(c!=colID){   
            // printf("Bitmask for col=%d: ", c);     
            for(int r=0; r<rows; r++){ 
                if(c==r%n){
                    bitmask[r] = 0;
                    // printf("0 ");
                } 
                else{
                    if(colID==mod(r+1,n)){
                        bitmask[r] = 1;
                    }
                    else{
                        if(c==mod(r+1,n)){
                            bitmask[r] = 1;
                        } else{
                           bitmask[r] = -1; 
                        }
                        
                        // bitmask[r] = 1;
                        // printf("1 ");
                    }
                }
            }
            // printf("\t");
            
            tmpctxt = *(shards[c].getPart1Ctxt());

            tmpctxt.multByConstant(bitmask);

            recoveredShard_part1+=tmpctxt;
        }
    }

    //Recover bits using part 2 shards
    for(int c=0; c<cols; c++){
        if(c!=colID){   
            // printf("Bitmask for col=%d: ", c);     
            for(int r=0; r<rows; r++){
                if(c+r%n==n-1){
                    bitmask[r] = 0;
                    // printf("0 ");
                } else{
                    if(mod((colID+r),n)==n-2){
                         bitmask[r] = 1;
                    }
                    else{
                        if(mod((c+r),n)==n-2){
                            bitmask[r] = 1;
                        } else{
                           bitmask[r] = -1; 
                        }
                    }
                    // bitmask[r] = 1;
                    // printf("1 ");
                }
            }
            // printf("\t");
            
            tmpctxt = *(shards[c].getPart2Ctxt());

            tmpctxt.multByConstant(bitmask);

            recoveredShard_part2+=tmpctxt;
        }
    }

    //Transfer recovered Part 1 parity to correct location in recovered Part 2
    int from = ((n-1)+colID)%n;
    int to = (n-1)-colID;

    for(int r=0; r<rows; r++){
        if(r%n==from){
            otherpart_bitmask[r] = 1;
        }
        else{
            otherpart_bitmask[r] = 0;
        }
    }
    tmpctxt = recoveredShard_part1;
    tmpctxt.multByConstant(otherpart_bitmask);
    encryptor.getEncryptedArray()->rotate(tmpctxt,to-from);
    recoveredShard_part2 += tmpctxt;

    //Transfer recovered Part 1 parity to correct location in recovered Part 2
    from = ((n-1)+(n-1-colID))%n;
    to = colID;
    for(int r=0; r<rows; r++){
        if(r%n==from){
            otherpart_bitmask[r] = 1;
        }
        else{
            otherpart_bitmask[r] = 0;
        }
    }
    tmpctxt = recoveredShard_part2;
    tmpctxt.multByConstant(otherpart_bitmask);
    encryptor.getEncryptedArray()->rotate(tmpctxt,to-from);
    recoveredShard_part1 += tmpctxt;

    //No return, just set the shards to the recovered values
    shards[colID].setPart1(recoveredShard_part1);
    shards[colID].setPart2(recoveredShard_part2);

    HELIB_TIMER_STOP;
    util::info("Done One-Col Recovery");
}

template<class T>
void
Panther::XCode<T>::recoverFromTwoColumnErasure(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &shards,
                                               int colID1, int colID2) {

    std::vector<int> countsP1(n);
    std::vector<int> countsP2(n);

    Dataset<bool> recoveredP1(mults*n,n);
    Dataset<bool> recoveredP2(mults*n,n);

    helib::Ctxt recoveredShard_c1_part1(*(encryptor.getPublicKey()));
    helib::Ctxt recoveredShard_c2_part1(*(encryptor.getPublicKey()));

    helib::Ctxt recoveredShard_c1_part2(*(encryptor.getPublicKey()));
    helib::Ctxt recoveredShard_c2_part2(*(encryptor.getPublicKey()));

    helib::Ptxt<helib::BGV> bitmask(*(encryptor.getContext()));
    helib::Ptxt<helib::BGV> invBitmask(*(encryptor.getContext()));

    bool missingbits = true;

    helib::Ctxt tmp1(*(encryptor.getPublicKey()));
    helib::Ctxt tmp2(*(encryptor.getPublicKey()));

    //Initialize 
    // std::cout << "Counts: " << std::endl;
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            if(c != colID1 && c != colID2){
                if(r != c){
                    countsP1[r]++;
                    recoveredP1.assign(r,c,1);    
                }
                if(r+c != n-1){
                    countsP2[r]++;
                    recoveredP2.assign(r,c,1);
                }
            }
        }
        // std::cout << countsP1[r] << " " << countsP2[r] << std::endl;
    }
    // printf("\n");
    // std::cout << "Recovered P1: " << std::endl;
    // recoveredP1.print();
    // printf("\n");
    // std::cout << "Recovered P2: " << std::endl;
    // recoveredP2.print();

    // printf("\n");
    // std::cout << "CountsP1 \tCountsP2 " << std::endl;
    // for(int r=0; r<n; r++){
    //     std::cout << countsP1[r] << "\t\t" << countsP2[r] << std::endl;
    // }

    int t=0;
    util::info("Starting Two-Col Recovery...");
    while(missingbits==true){
    // while(t<3){        

        // printf("------------ Iter %d------------\n", t);

        // printf("Recovering bits\n");
        // Recover available bits
        
        for(int r=0; r<n; r++){
            helib::Ctxt tmp1(*(encryptor.getPublicKey()));
            helib::Ctxt tmp2(*(encryptor.getPublicKey()));

            if(countsP1[r] == n-2){
                // printf("Getting for r=%d\n",r);            
                
                int recCol;  
                if(r!=colID1 && recoveredP1.get(r,colID1)!=1){
                    recCol = colID1;
                } else if(r!=colID2 && recoveredP1.get(r,colID2)!=1){
                    recCol = colID2;
                }
                // printf("recCol=%d\n",recCol);
                

                // printf("\n");
                //Add shards where bits are available
                for(int c = 0; c<n; c++){
                    if(recoveredP1.get(r,c)==true && c!=r){
                        for(int ri=0; ri<mults*n; ri++){ 
                            if(c==ri%n){
                                bitmask[ri] = 0;
                                // printf("0 ");
                            } 
                            else{
                                if(recCol==mod(ri+1,n)){
                                    bitmask[ri] = 1;
                                }
                                else{
                                    if(c==mod(ri+1,n)){
                                        bitmask[ri] = 1;
                                    } else{
                                       bitmask[ri] = -1; 
                                    }
                                }
                            }
                        }
                        // std::cout << "c = " << c << std::endl;
                        // encryptor.decryptAndPrintCondensed("shard part1ctxt", *(shards[c].getPart1Ctxt()), n);
                        tmp1 = *(shards[c].getPart1Ctxt());
                        tmp1.multByConstant(bitmask);
                        tmp2+=tmp1;
                    }
                }

                //setup bitmasks for accumulation
                // printf("bitmask row=%d: ",r);
                for(int ri = 0; ri<mults*n; ri++){
                    if(ri%n==r){
                        bitmask[ri] = 1;
                        invBitmask[ri] = 0;
                        // printf("1 ");

                    } else{
                        bitmask[ri] = 0;
                        invBitmask[ri] = 1;
                        // printf("0 ");
                    }
                }
                // encryptor.decryptAndPrintCondensed("tmp2", tmp2, n);

                //Shard = shard * inv_mask + mask*tmp
                tmp2.multByConstant(bitmask);
                // printf("Recovering part=1 countsP1=%d colID=%d row=%d  ", countsP1[r], recCol, r);
                if(recCol == colID1){
                    // encryptor.decryptAndPrintCondensed("Recovered bit", tmp2, n);
                    // tmp1 = recoveredShard_c1_part1;
                    // tmp1.multByConstant(invBitmask);
                    // recoveredShard_c1_part1 += tmp1;
                    recoveredShard_c1_part1.multByConstant(invBitmask);
                    recoveredShard_c1_part1 += tmp2;
                } else if(recCol == colID2){
                    // encryptor.decryptAndPrintCondensed("Recovered bit", tmp2, n);
                    // tmp1 = recoveredShard_c2_part1;
                    // tmp1.multByConstant(invBitmask);
                    // recoveredShard_c2_part1 += tmp1;
                    recoveredShard_c2_part1.multByConstant(invBitmask);
                    recoveredShard_c2_part1 += tmp2;
                }
                
                // countsP1[r]++;
            }
        }
        for(int r=0; r<n; r++){
            helib::Ctxt tmp1(*(encryptor.getPublicKey()));
            helib::Ctxt tmp2(*(encryptor.getPublicKey()));

            if(countsP2[r] == n-2){ //3 available bits
                // printf("Getting for r=%d\n",r);            
                
                int recCol;  
                if(r+colID1!=n-1 && recoveredP2.get(r,colID1)!=1){
                    recCol = colID1;
                } else if(r+colID2!=n-1 && recoveredP2.get(r,colID2)!=1){
                    recCol = colID2;
                }
            
                // printf("recCol=%d\n",recCol);
                
                //Add shards where bits are available
                for(int c = 0; c<n; c++){
                    //Setup bitmask depending on the parity bit locations
                    if(recoveredP2.get(r,c)==true && c+r!=n-1){
                        for(int ri = 0; ri<mults*n; ri++){
                            if(c+ri%n==n-1){
                                bitmask[ri] = 0;
                                // printf("0 ");
                            } else{
                                if(mod((recCol+ri),n)==n-2){
                                     bitmask[ri] = 1;
                                }
                                else{
                                    if(mod((c+ri),n)==n-2){
                                        bitmask[ri] = 1;
                                    } else{
                                       bitmask[ri] = -1; 
                                    }
                                }
                            }
                        }
                        // std::cout << "c = " << c << std::endl;
                        // encryptor.decryptAndPrintCondensed("shard part2ctxt", *(shards[c].getPart2Ctxt()), n);
                        tmp1 = *(shards[c].getPart2Ctxt());
                        tmp1.multByConstant(bitmask);
                        tmp2+=tmp1;
                    }
                }
                // encryptor.decryptAndPrintCondensed("tmp2", tmp2, n);

                //setup bitmasks
                // printf("bitmask row=%d: ",r);
                for(int ri = 0; ri<mults*n; ri++){
                    if(ri%n==r){
                        bitmask[ri] = 1;
                        invBitmask[ri] = 0;
                        // printf("1 ");
                    } else{
                        bitmask[ri] = 0;
                        invBitmask[ri] = 1;
                        // printf("0 ");
                    }
                }
                // printf("\n");

                //Shard = shard * inv_mask + mask*tmp
                // printf("Recovering part=2 countsP2=%d colID=%d row=%d  ", countsP2[r], recCol, r);
                tmp2.multByConstant(bitmask);
                if(recCol==colID1){
                    // encryptor.decryptAndPrintCondensed("Recovered bit", tmp2, n);
                    recoveredShard_c1_part2.multByConstant(invBitmask);
                    recoveredShard_c1_part2 += tmp2;
                } else if(recCol == colID2){
                    // encryptor.decryptAndPrintCondensed("Recovered bit", tmp2, n);
                    recoveredShard_c2_part2.multByConstant(invBitmask);
                    recoveredShard_c2_part2 += tmp2;
                }
               // countsP2[r]++;
            }
        } //recover bits

        // std::cout << "Part 1 and 2 update each other" << std::endl;
        int rotamt;
        std::vector<int> coords(2);
        //Part 1 and Part 2 update each other
        for(int r = 0; r<n; r++){
            if(countsP2[r] == n-2){ //Part 2 update Part 1
                int recCol;
                for(int c = 0; c<n; c++){
                    if(r+c!=n-1 && recoveredP2.get(r,c)!=1){
                        recCol = c;
                    }
                }
                // printf("recCol=%d\n",recCol);

                if(recCol == colID1){
                    // std::cout << "Part 2 --> Part 1: Col " << colID1 << std::endl;
                    //Put recovered shards into temp variables
                    tmp2 = recoveredShard_c1_part2;

                    //Find the coordinates of a part2 label in part1
                    coords = findLabel(part_2_labelTable->get(colID1,r), 1);
                    
                    //Determine amount to rotate part2 to align with part1 coordinates
                    rotamt = coords[1]-r;

                    //Complete rotation
                    // printf("r = %d  coords[1] = %d  rotamt = %d  ", r, coords[1], rotamt);
                    // encryptor.decryptAndPrintCondensed("c1 p2 into c1 p1 - before rotation", tmp2, n);
                    encryptor.getEncryptedArray()->rotate(tmp2, rotamt);
                    // encryptor.decryptAndPrintCondensed("c1 p2 into c1 p1 - after rotation", tmp2, n);

                    //Setup bitmasks to only add the bit of interest into part 1
                    // printf("bitmask ");
                    for(int ri = 0; ri<mults*n; ri++){
                        if(ri%n==coords[1]){
                            bitmask[ri] = 1;
                            invBitmask[ri] = 0;
                            // printf("1 ");
                        } else{
                            bitmask[ri] = 0;
                            invBitmask[ri] = 1;
                            // printf("0 ");
                        }
                    }
                    // printf("\n");

                    //Add rotated part 2 into 
                    recoveredShard_c1_part1.multByConstant(invBitmask);
                    tmp2.multByConstant(bitmask);
                    // encryptor.decryptAndPrintCondensed("after bitmask", tmp2, n);
                    recoveredShard_c1_part1+=tmp2;
                }

                else if(recCol == colID2){
                    // std::cout << "Part 2 --> Part 1: Col " << colID2 << std::endl;
                    //Put recovered shards into temp variables
                    tmp2 = recoveredShard_c2_part2;

                    //Find the coordinates of a part2 label in part1
                    coords = findLabel(part_2_labelTable->get(colID2,r), 1);
                    
                    //Determine amount to rotate part2 to align with part1 coordinates
                    rotamt = coords[1]-r;

                    //Complete rotation
                    // printf("r = %d  coords[1] = %d  rotamt = %d  ", r, coords[1], rotamt);
                    // encryptor.decryptAndPrintCondensed("c2 p2 into c2 p1- after rotation", tmp2, n);
                    encryptor.getEncryptedArray()->rotate(tmp2, rotamt);
                    // encryptor.decryptAndPrintCondensed("c2 p2 into c2 p1 - after rotation", tmp2, n);                    
                    
                    //Setup bitmasks to only add the bit of interest into part 1
                    // printf("bitmask ");
                    for(int ri = 0; ri<mults*n; ri++){
                        if(ri%n==coords[1]){
                            bitmask[ri] = 1;
                            invBitmask[ri] = 0;
                            // printf("1 ");
                        } else{
                            bitmask[ri] = 0;
                            invBitmask[ri] = 1;
                            // printf("0 ");
                        }
                    }
                    // printf("\n");

                    //Add rotated part 2 into 
                    recoveredShard_c2_part1.multByConstant(invBitmask);
                    tmp2.multByConstant(bitmask);
                    // encryptor.decryptAndPrintCondensed("after bitmask", tmp2, n);
                    recoveredShard_c2_part1+=tmp2;
                }
                recoveredP1.assign(coords[1],recCol,1); 
                recoveredP2.assign(r,recCol,1);
            } 

            if(countsP1[r] == n-2){ //Part 1 update Part 2
                int recCol;
                for(int c = 0; c<n; c++){
                    if(r!=c && recoveredP1.get(r,c)!=1){
                        recCol = c;
                    }
                }
                // printf("recCol=%d\n",recCol);
                if(recCol == colID1){
                    // std::cout << "Part 1 --> Part 2: Col " << colID1 << std::endl;
                    //Put recovered shards into temp variables
                    tmp1 = recoveredShard_c1_part1;
                    // tmp2 = recoveredShard_c1_part2;

                    //Find the coordinates in part 1 of this recovered bit in part 2
                    coords = findLabel(part_1_labelTable->get(colID1,r), 2);
                    //returns 3 for colId = 0, r = 2


                    //Determine amount to rotate part 1 to align with part 2 coordinates
                    rotamt = coords[1]-r;

                    //Complete rotation
                    // printf("r = %d  coords[1] = %d  rotamt = %d  ", r, coords[1], rotamt);
                    // encryptor.decryptAndPrintCondensed("c1 p1 into c1 p2 - before rotation", tmp1, n);
                    encryptor.getEncryptedArray()->rotate(tmp1, rotamt);
                    // encryptor.decryptAndPrintCondensed("c1 p1 into c1 p2 - before rotation ", tmp1, n);
                    
                    //Setup bitmasks to only add the bit of interest into part 2
                    // printf("bitmask ");
                    for(int ri = 0; ri<mults*n; ri++){
                        if(ri%n==coords[1]){
                            bitmask[ri] = 1;
                            invBitmask[ri] = 0;
                            // printf("1 ");
                        } else{
                            bitmask[ri] = 0;
                            invBitmask[ri] = 1;
                            // printf("0 ");
                        }
                    }
                    // printf("\n");

                    //Add rotated part 1 into 
                    recoveredShard_c1_part2.multByConstant(invBitmask);
                    tmp1.multByConstant(bitmask);
                    // encryptor.decryptAndPrintCondensed("after bitmask", tmp1, n);
                    recoveredShard_c1_part2+=tmp1;
                }
                else if(recCol == colID2){
                    // std::cout << "Part 1 --> Part 2: Col " << colID2 << std::endl;
                    //Put recovered shards into temp variables
                    tmp1 = recoveredShard_c2_part1;
                    // tmp2 = recoveredShard_c2_part2;

                    //Find the coordinates in part 1 of this recovered bit in part 2
                    coords = findLabel(part_1_labelTable->get(colID2,r), 2);
                    
                    //Determine amount to rotate part1 to align with part2 coordinates
                    rotamt = coords[1]-r;

                    //Complete rotation
                    // printf("r = %d  coords[1] = %d  rotamt = %d  ", r, coords[1], rotamt);
                    // encryptor.decryptAndPrintCondensed("c2 p1 into c2 p2 - before rotation:", tmp1, n);
                    encryptor.getEncryptedArray()->rotate(tmp1, rotamt);
                    // encryptor.decryptAndPrintCondensed("c2 p1 into c2 p2 - after rotation:", tmp1, n);
                    //Setup bitmasks to only add the bit of interest into part 2
                    // printf("bitmask ");
                    for(int ri = 0; ri<mults*n; ri++){
                        if(ri%n==coords[1]){
                            bitmask[ri] = 1;
                            invBitmask[ri] = 0;
                            // printf("1 ");
                        } else{
                            bitmask[ri] = 0;
                            invBitmask[ri] = 1;
                            // printf("0 ");
                        }
                    }
                    // printf("\n");

                    //Add rotated part 1 into 
                    recoveredShard_c2_part2.multByConstant(invBitmask);
                    tmp1.multByConstant(bitmask);
                    // encryptor.decryptAndPrintCondensed("after bitmask", tmp1, n);
                    recoveredShard_c2_part2+=tmp1;
                }
                recoveredP1.assign(r,recCol,1);
                recoveredP2.assign(coords[1],recCol,1); 
            }
        } //update parts

        // Increment counts, reset check vectors
        int countc1p1 = 0;
        int countc1p2 = 0;
        int countc2p1 = 0;
        int countc2p2 = 0;
        for(int r=0; r<n; r++){
            countsP1[r] = 0;
            countsP2[r] = 0;
        }

        //Check and update flags
        missingbits = false;
        for(int r=0; r<n; r++){
            countc1p1 += recoveredP1.get(r,colID1);
            countc1p2 += recoveredP1.get(r,colID2);
            countc2p1 += recoveredP2.get(r,colID1);
            countc2p2 += recoveredP2.get(r,colID2);

            for(int c=0; c<n; c++){
                if(r!=c && recoveredP1.get(r,c)){
                    countsP1[r]++;
                }
                if(r+c!=n-1 && recoveredP2.get(r,c)){
                    countsP2[r]++;
                }
            }
        }
        missingbits = (countc1p1 < n) || (countc1p2 < n) || (countc2p1 < n) || (countc2p2 < n);

        // printf("\n");
        // std::cout << "Recovered P1: " << std::endl;
        // recoveredP1.print();
        // printf("\n");
        // std::cout << "Recovered P2: " << std::endl;
        // recoveredP2.print();

        // printf("\n");
        // std::cout << "CountsP1 \tCountsP2 " << std::endl;
        // for(int r=0; r<n; r++){
        //     std::cout << countsP1[r] << "\t\t" << countsP2[r] << std::endl;
        // }

        
        // std::cout << std::endl;
        // encryptor.decryptAndPrintCondensed("Recovered c1 p1", recoveredShard_c1_part1, n);
        // encryptor.decryptAndPrintCondensed("Recovered c2 p1", recoveredShard_c2_part1, n);
        // encryptor.decryptAndPrintCondensed("Recovered c1 p2", recoveredShard_c1_part2, n);
        // encryptor.decryptAndPrintCondensed("Recovered c2 p2", recoveredShard_c2_part2, n);
        // std::cout << std::endl;

        t++;
        // printf("\n");
    }
    

    // encryptor.decryptAndPrintCondensed("Recovered c1 p1", recoveredShard_c1_part1, n);
    // encryptor.decryptAndPrintCondensed("Recovered c2 p1", recoveredShard_c2_part1, n);
    // encryptor.decryptAndPrintCondensed("Recovered c1 p2", recoveredShard_c1_part2, n);
    // encryptor.decryptAndPrintCondensed("Recovered c2 p2", recoveredShard_c2_part2, n);
    // std::cout << std::endl;

    //No return, just set the shards to the recovered values
    shards[colID1].setPart1(recoveredShard_c1_part1);
    shards[colID2].setPart1(recoveredShard_c2_part1);
    shards[colID1].setPart2(recoveredShard_c1_part2);
    shards[colID2].setPart2(recoveredShard_c2_part2);

    util::info("Done Two-Col Recovery");
}

template<class T>
std::vector<int>
Panther::XCode<T>::findLabel(std::string label, int tableNum){
    Panther::XCodeLabelTable<std::string> *table;
    std::vector<int> labelCoords(2);
    switch(tableNum){
        case 1:
            // std::cout << "Select Part 1 to search..." << std::endl;
            table = part_1_labelTable;
            break;
        case 2:
            // std::cout << "Select Part 2 to search..." << std::endl;
            table = part_2_labelTable;
            break;
        default:
            break; 
    }

    for(int r=0; r<mults*n; r++){
        for(int c=0; c<n; c++){
            if(table->get(c, r) == label){
                // std::cout << "Found at (" << c << "," << r << ")" << std::endl;
                labelCoords[0] = c;
                labelCoords[1] = r;
                break;
            }
        }
    }

    return labelCoords;
}

/**
    summation function in X-Cipher
    log2(mults*n) complexity
    Finds total sum of the entire structure by folding upon each block and adding each column
    returns a ctxt with total sum in the n-2 and n-1 slots
**/
template<class T>
helib::Ctxt
Panther::XCode<T>::summation(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &shards) {
    std::cout << "---- Summation Function ----- \n";
    int cols = n, rows = n * mults;
    std::cout << "n:" << n << " and mults: " << mults << "\n";
    helib::Ctxt cSum(*(encryptor.getPublicKey()));
    helib::Ctxt cSumTemp(*(encryptor.getPublicKey()));
    helib::Ptxt<helib::BGV> bitmask(*(encryptor.getContext()));

    //Component-wise addition of encrypted columns.
    for (int c = 0; c < n; c++) {
        cSum += *(shards[c].getPart1Ctxt());
    }


    for (int j = 0; j < log2(mults); j++) {
        int a = pow(2, j);
        std::cout << "Power a: " << a << "\n";
        cSumTemp = cSum;
        encryptor.getEncryptedArray()->shift(cSumTemp, -a * n);
        cSum += cSumTemp;
    }

    if (mults % 2 != 0) {
        encryptor.getEncryptedArray()->shift(cSumTemp, -(mults + 1) * n);
        cSum += cSumTemp;
    }

    //Mask out unwanted data from row 0 - row2
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            if (row < n) {
                bitmask[row] = 1;
            } else {
                bitmask[row] = 0;
            }
        }
    }
    cSum.multByConstant(bitmask);
    encryptor.decryptAndPrintCondensed("cSum", cSum, n);
    return cSum;
}


/**
    dotProd function
    Assumes two vectors of the same length are encoded into indices v1_ind and v2_ind of the shards
    Returns their dot product by masking out the codewords, taking their comp-wise product, and totalSum
    Returns a ciphertext with the result in the first slot
**/
template<class T>
helib::Ctxt
Panther::XCode<T>::dotProd(Panther::Encryptor &encryptor, helib::Ctxt v1Ctxt,  helib::Ctxt v2Ctxt) {

    helib::Ctxt c_dotProd(*(encryptor.getPublicKey()));
    helib::Ctxt tempctxt(*(encryptor.getPublicKey()));
    helib::Ptxt<helib::BGV> bitmask(*(encryptor.getContext()));

    c_dotProd += v1Ctxt;//*(shards[v1_ind].getPart1Ctxt());

    //masking the tags
    for (int r = 0; r < encryptor.getSlotCount(); r++) {
        if(r>=n*mults){
            bitmask[r] = 0;
        }
        else if (r % n == n - 2 || r % n == n - 1) {
            bitmask[r] = 0;
        } else {
            bitmask[r] = 1;
        }
    }
    //applied bitmask
    // std::cout << "BitMask Applied:  " << bitmask << std::endl;
    c_dotProd.multByConstant(bitmask);

    // encryptor.decryptAndPrintCondensed("V1: ", c_dotProd, n*mults);
    // encryptor.decryptAndPrintCondensed("V2: ", *(shards[v2_ind].getPart1Ctxt()), n*mults);

    //muliplying v1 * v2
    c_dotProd.multiplyBy(v2Ctxt);//*(shards[v2_ind].getPart1Ctxt()));
    
    // encryptor.decryptAndPrintCondensed("Mult: ", c_dotProd, n*mults);
    helib::totalSums(*encryptor.getEncryptedArray(), c_dotProd);
    // encryptor.decryptAndPrintCondensed("DotProd: ", c_dotProd, n*mults);

    helib::Ptxt<helib::BGV> mask(*(encryptor.getContext()));
    mask[0] = 1;

    c_dotProd.multByConstant(mask);

    return c_dotProd;
}



template<class T>
std::vector<helib::Ctxt>
Panther::XCode<T>::matrixAdd(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &matrix1, std::vector<Panther::Shard<T>> &matrix2){
    int c;
    
    std::vector<helib::Ctxt> sums;
    
    for(c=0; c<n; c++){
        // printf("tmp = matrix1[%d]\n", c);
        helib::Ctxt tmp = *(matrix1[c].getPart1Ctxt());
        // printf("tmp+= matrix2[%d]\n", c);
        tmp += *(matrix2[c].getPart1Ctxt());
        // printf("matrix1[%d]=tmp\n", c);
        sums.push_back(tmp);
    }

    return sums;
}

template<class T>
helib::Ctxt
Panther::XCode<T>::matrixMult(Panther::Encryptor &encryptor, std::vector<Panther::Shard<T>> &matrix1, std::vector<Panther::Shard<T>> &matrix2, int d){
    
    // A x B = sum_0^d-1[ (phi^k * sigmaA)*(psi^k*tauB)

    helib::Ctxt prod(*(encryptor.getPublicKey()));
    for(int k=0; k<d; k++){
        helib::Ctxt phi_k_sig_A = *(matrix1[1+k].getPart1Ctxt());
        helib::Ctxt psi_k_tau_B = *(matrix2[1+d+k].getPart1Ctxt());
        phi_k_sig_A *= psi_k_tau_B;
        prod += phi_k_sig_A;
    }

    return prod;
}

template class Panther::XCode<bool>;
template class Panther::XCode<int>;

