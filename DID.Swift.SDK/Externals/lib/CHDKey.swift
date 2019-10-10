

import Foundation

internal struct CMasterPublicKey {
    var fingerPrint: UInt32?
    
    var chainCode: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    var publicKey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    init() {}
}

@_silgen_name("HDkey_GenerateMnemonic")
internal func HDkey_GenerateMnemonic(_ language: Int32) -> UnsafePointer<Int8>!

@_silgen_name("HDkey_GetSeedFromMnemonic")
internal func HDkey_GetSeedFromMnemonic(_ mmemonic: UnsafePointer<Int8>!, _ mnemonicPassword: UnsafePointer<Int8>!, _ language: Int32, _ seed: UnsafeMutablePointer<Int8>!) -> UnsafeMutablePointer<Int8>!

@_silgen_name("HDkey_GetMasterPublicKey")
internal func HDkey_GetMasterPublicKey(_ seed: UnsafePointer<Int8>!,
                      _ coinType: Int32!,
                      _ key: OpaquePointer!) -> OpaquePointer!

@_silgen_name("HDkey_GetSubPrivateKey")
internal func HDkey_GetSubPrivateKey(_ seed: UnsafePointer<Int8>!,
                                       _ coinType: Int32!,
                                       _ chain: Int32!,
                                       _ index: Int32!,
                                       _ privatekey: UInt32!) -> UnsafeMutablePointer<Int8>!

@_silgen_name("HDkey_GetSubPublicKey")
internal func HDkey_GetSubPublicKey(_ master: OpaquePointer!,
                                     _ chain: Int32!,
                                     _ index: Int32!,
                                     _ publickey: UInt32!) -> UnsafeMutablePointer<Int8>!

@_silgen_name("HDkey_GetIdString")
internal func HDkey_GetIdString(_ publickey: UnsafeMutablePointer<CUnsignedChar>!,
                                    _ address: UnsafePointer<Int8>!,
                                    _ len: UInt32!) -> UnsafeMutablePointer<Int8>!
