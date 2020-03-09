import Foundation

internal struct CHDKey {
    static let PUBLICKEY_BYTES:     Int = 33
    static let PRIVATEKEY_BYTES:    Int = 32
    static let ADDRESS_LEN:         Int = 48
    static let CHAINCODE_BYTES:     Int = 32
    static let EXTENDEDKEY_BYTES:   Int = 82

    var fingerPrint: UInt32?
    
    var chainCodeForSk: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var privateKey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var chainCodeForPk: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var publicKey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
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

@_silgen_name("HDKey_MnemonicIsValid")
internal func HDKey_MnemonicIsValid(_ mnemonic: UnsafePointer<Int8>, _ language: Int32) -> Bool

@_silgen_name("HDKey_FromMnemonic")
internal func HDKey_FromMnemonic(_ mmemonic: UnsafePointer<Int8>, _ passphrase: UnsafePointer<Int8>, _ language: Int32!,_ hdkey: UnsafeMutablePointer<CHDKey>!
) ->  UnsafeMutablePointer<CHDKey>

@_silgen_name("HDKey_FromSeed")
internal func HDKey_FromSeed(_ seed: UnsafePointer<UInt8>, _ size: Int32, _ hdkey: UnsafeMutablePointer<CHDKey>!
) ->  UnsafeMutablePointer<CHDKey>

@_silgen_name("HDKey_FromExtendedKey")
internal func HDKey_FromExtendedKey(_ extendedkey: UnsafePointer<UInt8>, _ size: Int32, _ hdkey: UnsafeMutablePointer<CHDKey>!
) ->  UnsafeMutablePointer<CHDKey>

@_silgen_name("HDKey_Serialize")
internal func HDKey_Serialize(_ hdkey: UnsafeMutablePointer<CHDKey>, _ extendedkey: UnsafeMutablePointer<UInt8>, _ size: Int32) -> Int32

@_silgen_name("HDKey_Wipe")
internal func HDKey_Wipe(_ hdkey: UnsafeMutablePointer<CHDKey>)

@_silgen_name("HDKey_PublicKey2Address")
internal func HDKey_PublicKey2Address(_ publickey: UnsafeMutablePointer<UInt8>,
                                      _ address: UnsafePointer<Int8>!,
                                      _ len: Int32) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_GetDerivedKey")
internal func HDKey_GetDerivedKey(_ hdkey: UnsafeMutablePointer<CHDKey>,
                                  _ index: Int32,
                                  _ derivedkey: UnsafeMutablePointer<CDerivedKey>) -> UnsafeMutablePointer<CDerivedKey>

@_silgen_name("DerivedKey_GetPublicKey")
internal func DerivedKey_GetPublicKey(_ derivedkey: UnsafeMutablePointer<CDerivedKey>) -> UnsafeMutablePointer<UInt8>

@_silgen_name("DerivedKey_GetPublicKeyBase58")
internal func DerivedKey_GetPublicKeyBase58(_ derivedkey: UnsafeMutablePointer<CDerivedKey>, _ base: UnsafeMutablePointer<Int8>, _ size: Int32) -> UnsafePointer<Int8>

@_silgen_name("DerivedKey_GetPrivateKey")
internal func DerivedKey_GetPrivateKey(_ derivedkey: UnsafeMutablePointer<CDerivedKey>) -> UnsafeMutablePointer<UInt8>

@_silgen_name("DerivedKey_GetAddress")
internal func DerivedKey_GetAddress(_ derivedkey: UnsafeMutablePointer<CDerivedKey>) -> UnsafePointer<Int8>!

@_silgen_name("DerivedKey_Wipe")
internal func DerivedKey_Wipe(_ hdkey: UnsafeMutablePointer<CDerivedKey>)

