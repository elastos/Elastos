

import Foundation

internal struct CHDKey {
    var fingerPrint: UInt32?
    
    var chainCode: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    var publicKey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    var seed: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    init() {}
}

internal struct CDerivedKey {

    var publicKey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var privatekey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    var address: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    init() {}
}

@_silgen_name("HDKey_GenerateMnemonic")
internal func HDKey_GenerateMnemonic(_ language: Int32) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_GetSeedFromMnemonic")
internal func HDKey_GetSeedFromMnemonic(_ mmemonic: UnsafePointer<Int8>, _ mnemonicPasswordbase58: UnsafePointer<Int8>, _ language: Int8!,_ seed: UnsafeMutablePointer<Int8>!
    ) ->  UnsafeMutablePointer<Int8>

@_silgen_name("HDKey_MnemonicIsValid")
internal func HDKey_MnemonicIsValid(_ mnemonic: UnsafePointer<Int8>, _ language: Int32) -> Bool

@_silgen_name("HDKey_GetPrivateIdentity")
internal func HDKey_GetPrivateIdentity(_ seed: UnsafeMutablePointer<Int8>,
                                       _ coinType: Int,
                                       _ hdkey: UnsafeMutablePointer<CHDKey>?) ->  UnsafePointer<CHDKey>

@_silgen_name("HDKey_GetSubPrivateKey")
internal func HDKey_GetSubPrivateKey(_ privateIdentity: UnsafePointer<CHDKey>,
                                     _ coinType: Int32!,
                                     _ chain: Int32!,
                                     _ index: Int32!,
                                     _ privatekey: UnsafeMutablePointer<Int8>) -> UnsafeMutablePointer<Int8>

@_silgen_name("HDKey_GetSubPublicKey")
internal func HDKey_GetSubPublicKey(_ privateIdentity: UnsafePointer<CHDKey>!,
                                    _ chain: Int32!,
                                    _ index: Int32!,
                                    _ publickey: UnsafeMutablePointer<Int8>!) -> UnsafeMutablePointer<Int8>!

@_silgen_name("HDKey_GetDerivedKey")
internal func HDKey_GetDerivedKey(_ privateIdentity: UnsafePointer<CHDKey>,
                                  _ derivedkey: UnsafeMutablePointer<CDerivedKey>,
                                  _ coinType: Int32!,
                                  _ chain: Int32!,
                                  _ index: Int32!) ->  UnsafePointer<CDerivedKey>

@_silgen_name("DerivedKey_GetAddress")
internal func DerivedKey_GetAddress(_ derivedkey: UnsafePointer<CDerivedKey>) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_GetAddress")
internal func HDKey_GetAddress(_ publickey: UnsafeMutablePointer<Int8>, _ address: UnsafeMutablePointer<Int8>, _ size_t: Int32) -> UnsafeMutablePointer<Int8>
