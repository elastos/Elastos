

import Foundation


@_silgen_name("ecdsa_sign_base64v")
internal func ecdsa_sign_base64v(_ sig: UnsafeMutablePointer<Int8>, _ privatekey: UnsafePointer<UInt8>, _ count: Int, _ inputs: CVaListPointer) -> UnsafeMutablePointer<Int8>!
//ssize_t ecdsa_sign_base64v(char *sig, uint8_t *privatekey, int count, va_list inputs);


