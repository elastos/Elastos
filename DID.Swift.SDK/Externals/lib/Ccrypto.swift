

import Foundation


@_silgen_name("ecdsa_sign_base64v")
internal func ecdsa_sign_base64v(_ sig: UnsafeMutablePointer<Int8>, _ privatekey: UnsafeMutablePointer<Int8>, _ count: Int32, _ inputs: CVaListPointer)
//ssize_t ecdsa_sign_base64v(char *sig, uint8_t *privatekey, int count, va_list inputs);
