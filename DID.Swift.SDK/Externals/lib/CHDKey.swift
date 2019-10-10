

import Foundation

internal struct CMasterPublicKey {
    var fingerPrint: UInt32?
    
    var chainCode: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    var publicKey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    init() {}
}

@_silgen_name("HDkey_GenerateMnemonic")
internal func HDkey_GenerateMnemonic(_ language: UInt32) -> UnsafePointer<Int8>!

@_silgen_name("HDkey_GetSeedFromMnemonic")
internal func HDkey_GetSeedFromMnemonic(_ mmemonic: UnsafePointer<UInt8>, _ mnemonicPasswordbase58: UnsafePointer<UInt8>, _ language: UInt8!,_ seed: UnsafeMutablePointer<UInt8>!
    ) ->  UnsafeMutablePointer<UInt8>

@_silgen_name("HDkey_GetMasterPublicKey")
internal func HDkey_GetMasterPublicKey(_ seed: UnsafeMutablePointer<UInt8>,
                                       _ coinType: Int,
                                       _ key: UnsafeMutablePointer<CMasterPublicKey>?
    ) ->  UnsafePointer<CMasterPublicKey>


@_silgen_name("HDkey_GetSubPrivateKey")
internal func HDkey_GetSubPrivateKey(_ seed: UnsafePointer<UInt8>!,
                                     _ coinType: Int32!,
                                     _ chain: Int32!,
                                     _ index: Int32!,
                                     _ privatekey: UnsafeMutablePointer<UInt8>) -> UnsafeMutablePointer<UInt8>

@_silgen_name("HDkey_GetSubPublicKey")
internal func HDkey_GetSubPublicKey(_ master: CMasterPublicKey!,
                                    _ chain: Int32!,
                                    _ index: Int32!,
                                    _ publickey: UnsafeMutablePointer<UInt8>!) -> UnsafeMutablePointer<UInt8>!

@_silgen_name("HDkey_GetIdString")
internal func HDkey_GetIdString(_ publickey: UnsafeMutablePointer<CUnsignedChar>!,
                                _ address: UnsafePointer<Int8>!,
                                _ len: UnsafeMutablePointer<Int8>!) -> UnsafeMutablePointer<Int8>!
