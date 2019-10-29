

import Foundation


@_silgen_name("encrypt_to_base64")
internal func encrypt_to_base64(_ base64: UnsafeMutablePointer<Int8>, _ passwd: UnsafePointer<Int8>, _ input: UnsafePointer<UInt8>, _ len: Int) -> Int
