import Foundation

internal struct CHDKey {
    static let PUBLICKEY_BYTES:     Int = 33
    static let PRIVATEKEY_BYTES:    Int = 32
    static let ADDRESS_LEN:         Int = 48
    static let CHAINCODE_BYTES:     Int = 32
    static let EXTENDEDKEY_BYTES:   Int = 82

    var fingerPrint: UInt32?
    
    var prvChainCode: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var privatekey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var pubChainCode: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

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
internal func HDKey_GenerateMnemonic(_ language: UnsafePointer<Int8>) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_MnemonicIsValid")
internal func HDKey_MnemonicIsValid(_ mnemonic: UnsafePointer<Int8>, _ language: UnsafePointer<Int8>) -> Bool

@_silgen_name("HDKey_FromMnemonic")
internal func HDKey_FromMnemonic(_ mmemonic: UnsafePointer<Int8>, _ passphrase: UnsafePointer<Int8>, _ language: UnsafePointer<Int8>,_ hdkey: UnsafeMutablePointer<CHDKey>!
) ->  UnsafePointer<CHDKey>

@_silgen_name("HDKey_FromSeed")
internal func HDKey_FromSeed(_ seed: UnsafePointer<UInt8>, _ size: Int32, _ hdkey: UnsafeMutablePointer<CHDKey>!
) ->  UnsafePointer<CHDKey>

@_silgen_name("HDKey_FromExtendedKey")
internal func HDKey_FromExtendedKey(_ extendedkey: UnsafePointer<UInt8>, _ size: Int32, _ hdkey: UnsafeMutablePointer<CHDKey>!
) ->  UnsafePointer<CHDKey>

@_silgen_name("HDKey_SerializePrv")
internal func HDKey_SerializePrv(_ hdkey: UnsafePointer<CHDKey>, _ extendedkey: UnsafeMutablePointer<UInt8>, _ size: Int32) -> Int32

@_silgen_name("HDKey_SerializePrv")
internal func HDKey_SerializePub(_ hdkey: UnsafePointer<CHDKey>, _ extendedkey: UnsafeMutablePointer<UInt8>, _ size: Int32) -> Int32

@_silgen_name("HDKey_Wipe")
internal func HDKey_Wipe(_ hdkey: UnsafeMutablePointer<CHDKey>)

@_silgen_name("HDKey_PublicKey2Address")
internal func HDKey_PublicKey2Address(_ publickey: UnsafeMutablePointer<UInt8>,
                                      _ address: UnsafePointer<Int8>!,
                                      _ len: Int32) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_GetDerivedKey")
internal func HDKey_GetDerivedKey(_ hdkey: UnsafePointer<CHDKey>,
                                  _ index: Int32,
                                  _ derivedkey: UnsafeMutablePointer<CDerivedKey>) -> UnsafePointer<CDerivedKey>

@_silgen_name("DerivedKey_GetPublicKey")
internal func DerivedKey_GetPublicKey(_ derivedkey: UnsafePointer<CDerivedKey>) -> UnsafePointer<UInt8>

@_silgen_name("DerivedKey_GetPublicKeyBase58")
internal func DerivedKey_GetPublicKeyBase58(_ derivedkey: UnsafePointer<CDerivedKey>, _ base: UnsafePointer<Int8>, _ size: Int32) -> UnsafePointer<Int8>

@_silgen_name("DerivedKey_GetPrivateKey")
internal func DerivedKey_GetPrivateKey(_ derivedkey: UnsafePointer<CDerivedKey>) -> UnsafePointer<UInt8>

@_silgen_name("DerivedKey_GetAddress")
internal func DerivedKey_GetAddress(_ derivedkey: UnsafePointer<CDerivedKey>) -> UnsafePointer<Int8>!

@_silgen_name("DerivedKey_Wipe")
internal func DerivedKey_Wipe(_ hdkey: UnsafePointer<CDerivedKey>)


@_silgen_name("PEM_WritePublicKey")
internal func PEM_WritePublicKey(_ publicKey: UnsafePointer<UInt8>, _ buffer: UnsafePointer<Int8>, _ size: UnsafeMutablePointer<Int32>) -> Int32

@_silgen_name("PEM_WritePrivateKey")
internal func PEM_WritePrivateKey(_ publickey: UnsafePointer<UInt8>, _ privatekey: UnsafePointer<UInt8>, _ buffer: UnsafeMutablePointer<UInt8>, _ size: UnsafeMutablePointer<Int>!) -> Int32

