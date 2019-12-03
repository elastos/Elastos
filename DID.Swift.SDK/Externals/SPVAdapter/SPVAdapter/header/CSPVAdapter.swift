
import Foundation

@_silgen_name("SpvDidAdapter_Create")
internal func SpvDidAdapter_Create(_ walletDir: UnsafePointer<Int8>, _ walletId: UnsafePointer<Int8>, _ network: UnsafePointer<Int8>,_ resolver: UnsafePointer<Int8>!
    ) -> OpaquePointer

@_silgen_name("SpvDidAdapter_Destroy")
internal func SpvDidAdapter_Destroy(_ adapter: OpaquePointer)

@_silgen_name("SpvDidAdapter_IsAvailable")
internal func SpvDidAdapter_IsAvailable(_ adapter: OpaquePointer) -> Int32

@_silgen_name("SpvDidAdapter_CreateIdTransaction")
internal func SpvDidAdapter_CreateIdTransaction(_ adapter: OpaquePointer, _ payload: UnsafePointer<Int8>, _ memo: UnsafePointer<Int8>?,_ password: UnsafePointer<Int8>!
    ) -> Int32

@_silgen_name("SpvDidAdapter_Resolve")
internal func SpvDidAdapter_Resolve(_ adapter: OpaquePointer, _ did: UnsafePointer<Int8>) -> UnsafePointer<Int8>

@_silgen_name("SpvDidAdapter_FreeMemory")
internal func SpvDidAdapter_FreeMemory(_ adapter: OpaquePointer, _ mem: UnsafeMutableRawPointer?)

