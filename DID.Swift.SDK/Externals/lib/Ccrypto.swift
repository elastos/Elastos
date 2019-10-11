

import Foundation


@_silgen_name("ecdsa_sign")
internal func ecdsa_sign(_ sig: UnsafeMutablePointer<Int8>, _ privatekey: UnsafeMutablePointer<Int8>, _ count: Int32, _ inputs: CVaListPointer?) -> Int32
//ssize_t ecdsa_sign(uint8_t *sig, uint8_t *privatekey, int count, ...);

@_silgen_name("ecdsa_sign_base64")
internal func ecdsa_sign_base64(_ sig: UnsafeMutablePointer<Int8>, _ privatekey: UnsafeMutablePointer<Int8>, _ count: Int32, _ inputs: CVaListPointer?) -> Int32
//ssize_t ecdsa_sign_base64(char *sig, uint8_t *privatekey, int count, ...);
