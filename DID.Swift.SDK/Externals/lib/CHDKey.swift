

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
internal func HDkey_GetSeedFromMnemonic(_ mmemonic: UnsafePointer<Int8>, _ mnemonicPasswordbase58: UnsafePointer<Int8>, _ language: Int8!,_ seed: UnsafeMutablePointer<Int8>!
    ) ->  UnsafeMutablePointer<Int8>

@_silgen_name("HDkey_GetMasterPublicKey")
internal func HDkey_GetMasterPublicKey(_ seed: UnsafeMutablePointer<Int8>,
                                       _ coinType: Int,
                                       _ key: UnsafeMutablePointer<CMasterPublicKey>?
    ) ->  UnsafePointer<CMasterPublicKey>


@_silgen_name("HDkey_GetSubPrivateKey")
internal func HDkey_GetSubPrivateKey(_ seed: UnsafePointer<Int8>!,
                                     _ coinType: Int32!,
                                     _ chain: Int32!,
                                     _ index: Int32!,
                                     _ privatekey: UnsafeMutablePointer<Int8>) -> UnsafeMutablePointer<Int8>

@_silgen_name("HDkey_GetSubPublicKey")
internal func HDkey_GetSubPublicKey(_ master: UnsafePointer<CMasterPublicKey>!,
                                    _ chain: Int32!,
                                    _ index: Int32!,
                                    _ publickey: UnsafeMutablePointer<Int8>!) -> UnsafeMutablePointer<Int8>!

@_silgen_name("HDkey_GetIdString")
internal func HDkey_GetIdString(_ publickey: UnsafeMutablePointer<Int8>,
                                _ address: UnsafeMutablePointer<Int8>!,
                                _ len: Int!) -> UnsafeMutablePointer<Int8>!
