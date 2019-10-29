

import Foundation


@_silgen_name("encrypt_to_base64")
internal func encrypt_to_base64(_ base64: UnsafeMutablePointer<Int8>, _ passwd: UnsafePointer<Int8>, _ input: UnsafePointer<UInt8>, _ len: Int) -> Int
//        decrypt_from_base64(<#T##plain: UnsafeMutablePointer<UInt8>!##UnsafeMutablePointer<UInt8>!#>, <#T##passwd: UnsafePointer<Int8>!##UnsafePointer<Int8>!#>, <#T##base64: UnsafePointer<Int8>!##UnsafePointer<Int8>!#>)
