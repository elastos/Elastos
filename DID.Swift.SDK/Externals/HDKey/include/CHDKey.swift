import Foundation

internal struct CHDKey {
    static let PUBLICKEY_BYTES:     Int = 33
    static let PRIVATEKEY_BYTES:    Int = 32
    static let ADDRESS_LEN:         Int = 48
    static let CHAINCODE_BYTES:     Int = 32
    static let EXTENDEDKEY_BYTES:   Int = 82

    var depth: UInt8?
    var fingerPrint: UInt32?
    var childnumber: UInt32?
    var prvChainCode: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var privatekey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var pubChainCode: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var publicKey: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    var address: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    
    init() {}
}

@_silgen_name("HDKey_GenerateMnemonic")
internal func HDKey_GenerateMnemonic(_ language: UnsafePointer<Int8>) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_FreeMnemonic")
internal func HDKey_FreeMnemonic(_ mnemonic: UnsafePointer<Int8>)

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

@_silgen_name("HDKey_FromExtendedKeyBase58")
internal func HDKey_FromExtendedKeyBase58(_ extendedkeyBase58: UnsafePointer<Int8>, _ size: Int32, _ hdkey: UnsafeMutablePointer<CHDKey>!
) ->  UnsafePointer<CHDKey>

@_silgen_name("HDKey_SerializePrv")
internal func HDKey_SerializePrv(_ hdkey: UnsafePointer<CHDKey>, _ extendedkey: UnsafeMutablePointer<UInt8>, _ size: Int32) -> Int32

@_silgen_name("HDKey_SerializePub")
internal func HDKey_SerializePub(_ hdkey: UnsafePointer<CHDKey>, _ extendedkey: UnsafeMutablePointer<UInt8>, _ size: Int32) -> Int32

@_silgen_name("HDKey_Deserialize")
internal func HDKey_Deserialize(_ hdkey: UnsafeMutablePointer<CHDKey>, _ extendedkey: UnsafePointer<UInt8>, _ size: Int32) -> UnsafePointer<CHDKey>

@_silgen_name("HDKey_DeserializeBase58")
internal func HDKey_DeserializeBase58(_ hdkey: UnsafeMutablePointer<CHDKey>, _ extendedkeyBase58: UnsafePointer<Int8>, _ size: Int32) -> UnsafePointer<CHDKey>

@_silgen_name("HDKey_SerializePrvBase58")
internal func HDKey_SerializePrvBase58(_ hdkey: UnsafePointer<CHDKey>, _ extendedkeyBase58: UnsafeMutablePointer<Int8>, _ size: Int32) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_SerializePubBase58")
internal func HDKey_SerializePubBase58(_ hdkey: UnsafePointer<CHDKey>, _ extendedkeyBase58: UnsafeMutablePointer<Int8>, _ size: Int32) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_Wipe")
internal func HDKey_Wipe(_ hdkey: UnsafeMutablePointer<CHDKey>)

@_silgen_name("HDKey_PublicKey2Address")
internal func HDKey_PublicKey2Address(_ publickey: UnsafeMutablePointer<UInt8>,
                                      _ address: UnsafePointer<Int8>!,
                                      _ len: Int32) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_GetvDerivedKey")
internal func HDKey_GetvDerivedKey(_ hdkey: UnsafePointer<CHDKey>, _ derivedkey: UnsafeMutablePointer<CHDKey>, _ depth: Int32, _ list: CVaListPointer) -> UnsafePointer<CHDKey>

@_silgen_name("HDKey_GetPublicKey")
internal func HDKey_GetPublicKey(_ hdkey: UnsafePointer<CHDKey>) -> UnsafePointer<UInt8>

@_silgen_name("HDKey_GetPublicKeyBase58")
internal func HDKey_GetPublicKeyBase58(_ hdkey: UnsafePointer<CHDKey>, _ base: UnsafeMutablePointer<Int8>, _ size: Int32) -> UnsafePointer<Int8>

@_silgen_name("HDKey_GetPrivateKey")
internal func HDKey_GetPrivateKey(_ hdkey: UnsafePointer<CHDKey>) -> UnsafePointer<UInt8>

@_silgen_name("HDKey_GetAddress")
internal func HDKey_GetAddress(_ hdkey: UnsafePointer<CHDKey>) -> UnsafePointer<Int8>!

@_silgen_name("HDKey_Wipe")
internal func HDKey_Wipe(_ hdkey: UnsafePointer<CHDKey>)

@_silgen_name("HDKey_PaddingToExtendedPrivateKey")
internal func HDKey_PaddingToExtendedPrivateKey(_ privatekey: UnsafeMutablePointer<UInt8>, _ psize: UInt32, _ extendedkey: UnsafeMutablePointer<UInt8>, _ esize: UInt32) -> Int32

@_silgen_name("PEM_WritePublicKey")
internal func PEM_WritePublicKey(_ publicKey: UnsafePointer<UInt8>, _ buffer: UnsafePointer<Int8>, _ size: UnsafeMutablePointer<Int32>) -> Int32

@_silgen_name("PEM_WritePrivateKey")
internal func PEM_WritePrivateKey(_ publickey: UnsafePointer<UInt8>, _ privatekey: UnsafePointer<UInt8>, _ buffer: UnsafeMutablePointer<UInt8>, _ size: UnsafeMutablePointer<Int>!) -> Int32

