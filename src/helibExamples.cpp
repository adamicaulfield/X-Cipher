//
// Created by Peizhao Hu on 7/8/20.
//

#include "helibExamples.h"
#include <helib/binaryArith.h>
#include <helib/helib.h>
#include <helib/intraSlot.h>
#include <iostream>

void
helibExamples::binary_example() {
    std::cout << "Binary Example" << std::endl;
    /*  Example of binary arithmetic using the BGV scheme  */

    // First set up parameters.
    //
    // NOTE: The parameters used in this example code are for demonstration only.
    // They were chosen to provide the best performance of execution while
    // providing the context to demonstrate how to use the "Binary Arithmetic
    // APIs". The parameters do not provide the security level that might be
    // required by real use/application scenarios.

    // Plaintext prime modulus.
    long p = 2;
    // Cyclotomic polynomial - defines phi(m).
    long m = 4095;
    // Hensel lifting (default = 1).
    long r = 1;
    // Number of bits of the modulus chain.
    long bits = 500;
    // Number of columns of Key-Switching matrix (typically 2 or 3).
    long c = 2;
    // Factorisation of m required for bootstrapping.
    std::vector<long> mvec = {7, 5, 9, 13};
    // Generating set of Zm* group.
    std::vector<long> gens = {2341, 3277, 911};
    // Orders of the previous generators.
    std::vector<long> ords = {6, 4, 6};

    std::cout << "Initialising context object..." << std::endl;
    // Intialise the context.
    helib::Context context(m, p, r, gens, ords);

    // Modify the context, adding primes to the modulus chain.
    std::cout << "Building modulus chain..." << std::endl;
    buildModChain(context, bits, c);

    // Make bootstrappable.
    context.makeBootstrappable(
            helib::convert<NTL::Vec<long>, std::vector<long>>(mvec));

    // Print the context.
    context.zMStar.printout();
    std::cout << std::endl;

    // Print the security level.
    std::cout << "Security: " << context.securityLevel() << std::endl;

    // Secret key management.
    std::cout << "Creating secret key..." << std::endl;
    // Create a secret key associated with the context.
    helib::SecKey secret_key(context);
    // Generate the secret key.
    secret_key.GenSecKey();

    // Generate bootstrapping data.
    secret_key.genRecryptData();

    // Public key management.
    // Set the secret key (upcast: SecKey is a subclass of PubKey).
    const helib::PubKey &public_key = secret_key;

    // Get the EncryptedArray of the context.
    const helib::EncryptedArray &ea = *(context.ea);

    // Build the unpack slot encoding.
    std::vector<helib::zzX> unpackSlotEncoding;
    buildUnpackSlotEncoding(unpackSlotEncoding, ea);

    // Get the number of slot (phi(m)).
    long nslots = ea.size();
    std::cout << "Number of slots: " << nslots << std::endl;

    // Generate three random binary numbers a, b, c.
    // Encrypt them under BGV.
    // Calculate a * b + c with HElib's binary arithmetic functions, then decrypt
    // the result.
    // Next, calculate a + b + c with HElib's binary arithmetic functions, then
    // decrypt the result.
    // Finally, calculate popcnt(a) with HElib's binary arithmetic functions, then
    // decrypt the result.  Note that popcnt, also known as hamming weight or bit
    // summation, returns the count of non-zero bits.

    // Note: several numbers can be encoded across the slots of each ciphertext
    // which would result in several parallel slot-wise operations.
    // For simplicity we place the same data into each slot of each ciphertext,
    // printing out only the back of each vector.
    long bitSize = 16;
    long outSize = 2 * bitSize;
    long a_data = NTL::RandomBits_long(bitSize);
    long b_data = NTL::RandomBits_long(bitSize);
    long c_data = NTL::RandomBits_long(bitSize);

    std::cout << "Pre-encryption data:" << std::endl;
    std::cout << "a = " << a_data << std::endl;
    std::cout << "b = " << b_data << std::endl;
    std::cout << "c = " << c_data << std::endl;

    // Use a scratch ciphertext to populate vectors.
    helib::Ctxt scratch(public_key);
    std::vector<helib::Ctxt> encrypted_a(bitSize, scratch);
    std::vector<helib::Ctxt> encrypted_b(bitSize, scratch);
    std::vector<helib::Ctxt> encrypted_c(bitSize, scratch);
    // Encrypt the data in binary representation.
    for (long i = 0; i < bitSize; ++i) {
        std::vector<long> a_vec(ea.size());
        std::vector<long> b_vec(ea.size());
        std::vector<long> c_vec(ea.size());
        // Extract the i'th bit of a,b,c.
        std::cout << "bit vector: ";
        for (auto &slot : a_vec) {
            slot = (a_data >> i) & 1;
            std::cout << slot;
        }
        std::cout << std::endl;
        for (auto &slot : b_vec)
            slot = (b_data >> i) & 1;
        for (auto &slot : c_vec)
            slot = (c_data >> i) & 1;
        // Generate individually encrypted ctxt with all slots filled by the bit value at the i-th index
        // Put all values of a_vec into one ctxt filling all slots
        ea.encrypt(encrypted_a[i], public_key, a_vec);
        ea.encrypt(encrypted_b[i], public_key, b_vec);
        ea.encrypt(encrypted_c[i], public_key, c_vec);
    }

    // Although in general binary numbers are represented here as
    // std::vector<helib::Ctxt> the binaryArith APIs for HElib use the PtrVector
    // wrappers instead, e.g. helib::CtPtrs_vectorCt. These are nothing more than
    // thin wrapper classes to standardise access to different vector types, such
    // as NTL::Vec and std::vector. They do not take ownership of the underlying
    // object but merely provide access to it.
    //
    // helib::CtPtrMat_vectorCt is a wrapper for
    // std::vector<std::vector<helib::Ctxt>>, used for representing a list of
    // encrypted binary numbers.

    // Perform the multiplication first and put it in encrypted_product.
    std::vector<helib::Ctxt> encrypted_product;
    helib::CtPtrs_vectorCt product_wrapper(encrypted_product);
    helib::multTwoNumbers(
            product_wrapper,
            helib::CtPtrs_vectorCt(encrypted_a),
            helib::CtPtrs_vectorCt(encrypted_b),
            /*rhsTwosComplement=*/false, // This means the rhs is unsigned rather than
            // 2's complement.
            outSize, // Outsize is the limit on the number of bits in the output.
            &unpackSlotEncoding); // Information needed for bootstrapping.

    // Now perform the encrypted sum and put it in encrypted_result.
    std::vector<helib::Ctxt> encrypted_result;
    helib::CtPtrs_vectorCt result_wrapper(encrypted_result);
    helib::addTwoNumbers(
            result_wrapper,
            product_wrapper,
            helib::CtPtrs_vectorCt(encrypted_c),
            /*negative=*/false, // This means the number are unsigned rather than 2's
            // complement.
            &unpackSlotEncoding); // Information needed for bootstrapping.

    // Decrypt and print the result.
    std::vector<long> decrypted_result;
    helib::decryptBinaryNums(decrypted_result, result_wrapper, secret_key, ea);
    std::cout << "a*b+c = " << decrypted_result.back() << std::endl;

    // Now calculate the sum of a, b and c using the addManyNumbers function.
    encrypted_result.clear();
    decrypted_result.clear();
    std::vector<std::vector<helib::Ctxt>> summands = {
            encrypted_a, encrypted_b, encrypted_c};
    helib::CtPtrMat_vectorCt summands_wrapper(summands);
    helib::addManyNumbers(
            result_wrapper,
            summands_wrapper,
            0,                    // sizeLimit=0 means use as many bits as needed.
            &unpackSlotEncoding); // Information needed for bootstrapping.

    // Decrypt and print the result.
    helib::decryptBinaryNums(decrypted_result, result_wrapper, secret_key, ea);
    std::cout << "a+b+c = " << decrypted_result.back() << std::endl;

    // This section calculates popcnt(a) using the fifteenOrLess4Four
    // function.
    // Note: the output i.e. encrypted_result should be of size 4
    // because 4 bits are required to represent numbers in [0,15].
    encrypted_result.resize(4lu, scratch);
    decrypted_result.clear();
    encrypted_a.pop_back(); // drop the MSB since we only support up to 15 bits.
    helib::fifteenOrLess4Four(result_wrapper,
                              helib::CtPtrs_vectorCt(encrypted_a));

    // Decrypt and print the result.
    helib::decryptBinaryNums(decrypted_result, result_wrapper, secret_key, ea);
    std::cout << "popcnt(a) = " << decrypted_result.back() << std::endl;
}

void
helibExamples::packed_example() {
    std::cout << "Ciphertext Packing Example" << std::endl;

    /*  Example of BGV scheme  */

    // Plaintext prime modulus
    unsigned long p = 4999;
    // Cyclotomic polynomial - defines phi(m)
    unsigned long m = 32109;
    // Hensel lifting (default = 1)
    unsigned long r = 1;
    // Number of bits of the modulus chain
    unsigned long bits = 500;
    // Number of columns of Key-Switching matrix (default = 2 or 3)
    unsigned long c = 2;

    std::cout << "Initialising context object..." << std::endl;
    // Intialise context
    helib::Context context(m, p, r);
    // Modify the context, adding primes to the modulus chain
    std::cout << "Building modulus chain..." << std::endl;
    buildModChain(context, bits, c);

    // Print the context
    context.zMStar.printout();
    std::cout << std::endl;

    // Print the security level
    std::cout << "Security: " << context.securityLevel() << std::endl;

    // Secret key management
    std::cout << "Creating secret key..." << std::endl;
    // Create a secret key associated with the context
    helib::SecKey secret_key(context);
    // Generate the secret key
    secret_key.GenSecKey();
    std::cout << "Generating key-switching matrices..." << std::endl;
    // Compute key-switching matrices that we need
    helib::addSome1DMatrices(secret_key);

    // Public key management
    // Set the secret key (upcast: SecKey is a subclass of PubKey)
    const helib::PubKey &public_key = secret_key;

    // Get the EncryptedArray of the context
    const helib::EncryptedArray &ea = *(context.ea);

    // Get the number of slot (phi(m))
    long nslots = ea.size();
    std::cout << "Number of slots: " << nslots << std::endl;

    // Create a vector of long with nslots elements
    helib::Ptxt<helib::BGV> ptxt(context);
    // Set it with numbers 0..nslots - 1
    for (int i = 0; i < ptxt.size(); ++i) {
        ptxt[i] = i;
    }

    // Print the plaintext
    std::cout << "Initial Plaintext: " << ptxt << std::endl;

    // Create a ciphertext
    helib::Ctxt ctxt(public_key);
    // Encrypt the plaintext using the public_key
    public_key.Encrypt(ctxt, ptxt);

    // Square the ciphertext
    ctxt.multiplyBy(ctxt);
    // Plaintext version
    ptxt.multiplyBy(ptxt);

    // Divide the ciphertext by itself
    // To do this we must calculate the multiplicative inverse using Fermat's
    // Little Theorem.  We calculate a^{-1} = a^{p-2} mod p, where a is non-zero
    // and p is our plaintext prime.
    // First make a copy of the ctxt using copy constructor
    helib::Ctxt ctxt_divisor(ctxt);
    // Raise the copy to the exponenet p-2
    ctxt_divisor.power(p - 2);
    // a^{p-2}*a = a^{-1}*a = a / a = 1;
    ctxt.multiplyBy(ctxt_divisor);

    // Plaintext version
    helib::Ptxt<helib::BGV> ptxt_divisor(ptxt);
    ptxt_divisor.power(p - 2);
    ptxt.multiplyBy(ptxt_divisor);

    // Double it (using additions)
    ctxt += ctxt;
    // Plaintext version
    ptxt += ptxt;

    // Subtract it from itself (result should be 0)
    ctxt -= ctxt;
    // Plaintext version
    ptxt -= ptxt;

    // Create a plaintext for decryption
    helib::Ptxt<helib::BGV> plaintext_result(context);
    // Decrypt the modified ciphertext
    secret_key.Decrypt(plaintext_result, ctxt);

    // Print the decrypted plaintext
    std::cout << "Decrypted Plaintext: " << plaintext_result << std::endl;
    // Print the plaintext version result, should be the same as the ctxt version
    std::cout << "Plaintext version: " << ptxt << std::endl;

    // We can also add constants
    ctxt.addConstant(NTL::ZZX(1l));
    // Plaintext version
    ptxt.addConstant(NTL::ZZX(1l));

    // And multiply by constants
    ctxt *= NTL::ZZX(1l);
    // Plaintext version
    ptxt *= NTL::ZZX(1l);

    // We can also perform ciphertext-plaintext operations
    ctxt += ptxt;

    // Decrypt the modified ciphertext into a new plaintext
    helib::Ptxt<helib::BGV> new_plaintext_result(context);
    secret_key.Decrypt(new_plaintext_result, ctxt);

    // Print the decrypted plaintext
    std::cout << "Decrypted Plaintext: " << new_plaintext_result << std::endl;
}

void
helibExamples::test_slot() {
    /*  Example of BGV scheme  */

    // Plaintext prime modulus
    unsigned long p = 2;
    // Cyclotomic polynomial - defines phi(m)
    unsigned long m = 0; //53261; //32109;
    // Hensel lifting (default = 1)
    unsigned long r = 1;
    // Number of bits of the modulus chain
    unsigned long bits =128; //4096;
    // Number of columns of Key-Switching matrix (default = 2 or 3)
    unsigned long c = 2;


    /**
    * @brief Returns smallest parameter m satisfying various constraints:
    * @param k security parameter
    * @param L number of levels
    * @param c number of columns in key switching matrices
    * @param p characteristic of plaintext space
    * @param d embedding degree (d ==0 or d==1 => no constraint)
    * @param s at least that many plaintext slots
    * @param chosen_m preselected value of m (0 => not preselected)
    * Fails with an error message if no suitable m is found
    * prints an informative message if verbose == true
    **/
    //long FindM(long k, long nBits, long c, long p, long d, long s, long chosen_m, bool verbose=false);
    m = helib::FindM(128, bits, c, p, 0, 1024, 0, false);

    std::cout << "Initialising context object..." << std::endl;
    // Intialise context
    helib::Context context(m, p, r);

    // Modify the context, adding primes to the modulus chain
    std::cout << "Building modulus chain..." << std::endl;
    buildModChain(context, bits, c);

    // Print the context
    context.zMStar.printout();
    std::cout << std::endl;

    // Print the security level
    std::cout << "Security: " << context.securityLevel() << std::endl;

    // Secret key management
    std::cout << "Creating secret key..." << std::endl;
    // Create a secret key associated with the context
    helib::SecKey secret_key(context);
    // Generate the secret key
    secret_key.GenSecKey();
    std::cout << "Generating key-switching matrices..." << std::endl;
    // Compute key-switching matrices that we need
    helib::addSome1DMatrices(secret_key);

    // Public key management
    // Set the secret key (upcast: SecKey is a subclass of PubKey)
    const helib::PubKey &public_key = secret_key;

    // Get the EncryptedArray of the context
    const helib::EncryptedArray &ea = *(context.ea);

    // Get the number of slot (phi(m))
    long nslots = ea.size();
    std::cout << "Number of slots: " << nslots << std::endl;

    std::cout << "getM()= " << ea.getContext().zMStar.getM() << std::endl;
}