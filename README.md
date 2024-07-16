# X-Cipher: Achieving Data Resiliency in Homomorphic Ciphertexts

GitHub repository for the prototype of X-Cipher, formerly called Panther.

## Accepted Paper

X-Cipher was accepted to the 2024 International Conference on Information and Communications Security (ICICS 2024). Author's version of the paper can be found [here](https://people.rit.edu/ac7717/xcipher_icics.pdf).

### Abstract
Homomorphic encryption (HE) allows for computations over ciphertexts while they are encrypted. Because of this, HE supports the outsourcing of computation on private data. Due to the additional risks caused by data outsourcing, the ability to recover from losses is essential, but doing so on data encrypted under an HE scheme introduces additional challenges for recovery and usability. This work introduces X-Cipher, which aims to make HE ciphertexts resilient by ensuring they are private and recoverable simultaneously at all stages during data outsourcing. X-Cipher allows data recovery without requiring the decryption of HE ciphertexts and maintains its ability to recover and keep data private when a cluster server has been compromised. X-Cipher allows for reduced ciphertext storage overhead by introducing novel encoding and leveraging previously introduced ciphertext packing. X-Cipher's capabilities were evaluated on a synthetic dataset to demonstrate that X-Cipher enables secure availability capabilities while enabling privacy-preserving outsourced computations.

## How to run it:
```bash
bash bootstrap.sh
```

## Reference for HElib
* https://homenc.github.io/HElib/
* https://eprint.iacr.org/2014/106.pdf
* https://eprint.iacr.org/2018/244.pdf
* http://people.csail.mit.edu/shaih/pubs/he-library.pdf

