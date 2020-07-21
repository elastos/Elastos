import Foundation

public struct KeyPair {
    var publicKey: Data?
    var privatekey: Data?
}

public class HDKey: NSObject {
    public static let PUBLICKEY_BYTES : Int = 33
    public static let PRIVATEKEY_BYTES: Int = 32
    public static let SEED_BYTES: Int = 64
    public static let EXTENDED_KEY_BYTES = 82
    public static let EXTENDED_PRIVATEKEY_BYTES = EXTENDED_KEY_BYTES
    public static let EXTENDED_PUBLICKEY_BYTES = EXTENDED_KEY_BYTES

    private static let PADDING_IDENTITY = 0x67
    private static let PADDING_STANDARD = 0xAD
    
    private var key: UnsafePointer<CHDKey>

    // Derive path: m/44'/0'/0'/0/index
    public static let DERIVE_PATH_PREFIX = "44H/0H/0H/0/"

    // Pre-derive publickey path: m/44'/0'/0'
    public static let PRE_DERIVED_PUBLICKEY_PATH = "44H/0H/0H"

    let PUBLICKEY_BASE58_BYTES = 64

    required init(_ key: UnsafePointer<CHDKey>) {
        self.key = key
    }

    public convenience init(_ mnemonic: String, _ passPhrase: String, _ language: String) {
        let cmnemonic = mnemonic.toUnsafePointerInt8()!
        let cpassphrase = passPhrase.toUnsafePointerInt8()!
        let chdKey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
        let key = HDKey_FromMnemonic(cmnemonic, cpassphrase, language.toUnsafePointerInt8()!, chdKey)
        self.init(key)
    }

    public convenience init(_ seed: Data) {
        let cseed: UnsafePointer<UInt8> = seed.withUnsafeBytes { bytes -> UnsafePointer<UInt8> in
            return bytes
        }

        let chdKey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
        let chdkey = HDKey_FromSeed(cseed, Int32(seed.count), chdKey)
        self.init(chdkey)
    }

    public func getPrivateKeyBytes() -> [UInt8] {
        let privatekeyPointer = HDKey_GetPrivateKey(key)
        let privatekeyPointerToArry: UnsafeBufferPointer<UInt8> = UnsafeBufferPointer(start: privatekeyPointer, count: HDKey.PRIVATEKEY_BYTES)
        let privatekeyData: Data = Data(buffer: privatekeyPointerToArry)

        return [UInt8](privatekeyData)
    }

    public func getPrivateKeyData() -> Data {
        let privatekeyPointer = HDKey_GetPrivateKey(key)
        let privatekeyPointerToArry: UnsafeBufferPointer<UInt8> = UnsafeBufferPointer(start: privatekeyPointer, count: HDKey.PRIVATEKEY_BYTES)

        return Data(buffer: privatekeyPointerToArry)
    }

    public func getPrivateKeyBase58() -> String {
        return Base58.base58FromBytes(getPrivateKeyBytes())
    }

    public func getPublicKeyBytes() ->[UInt8] {
        let cpublicKeyPointer = HDKey_GetPublicKey(key)
        let publicKeyPointerToArry: UnsafeBufferPointer<UInt8> = UnsafeBufferPointer(start: cpublicKeyPointer, count: HDKey.PUBLICKEY_BYTES)
        let publicKeyData: Data = Data(buffer: publicKeyPointerToArry)

        return [UInt8](publicKeyData)
    }

    public func getPublicKeyData() -> Data {
        let cpublicKeyPointer = HDKey_GetPublicKey(key)
        let publicKeyPointerToArry: UnsafeBufferPointer<UInt8> = UnsafeBufferPointer(start: cpublicKeyPointer, count: HDKey.PUBLICKEY_BYTES)

        return Data(buffer: publicKeyPointerToArry)
    }

    public func getPublicKeyBase58() -> String {
        let basePointer: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: HDKey.PUBLICKEY_BYTES)
        let cpublickeybase58 = HDKey_GetPublicKeyBase58(key, basePointer, Int32(PUBLICKEY_BASE58_BYTES))
        print(String(cString: cpublickeybase58))
        return String(cString: cpublickeybase58)
    }

    public func serialize() throws -> Data {
        let data = Base58.bytesFromBase58(serializeBase58())
        return Data(bytes: data, count: data.count)
    }

    public func serializeBase58() -> String {
        let extendedkeyPointer: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 512)
        let cextendedkey = HDKey_SerializePrvBase58(key, extendedkeyPointer, 512)
        
        return String(cString: cextendedkey!)
    }

    public func serializePublicKey() throws -> [UInt8] {
        return try Base58.bytesFromBase58(serializePublicKeyBase58())
    }

    public func serializePublicKeyBase58() throws -> String {

        let extendedkeyPointer: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 256)
        let cextendedkey = HDKey_SerializePubBase58(key, extendedkeyPointer,Int32(256))
        guard let _ = cextendedkey else {
            throw DIDError.notFoundError("HDKey_SerializePubBase58 error.")
        }

        return String(cString: cextendedkey!)
    }

    public class func deserialize(_ keyData: [UInt8]) -> HDKey {
        let chdKey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
        let cdata: UnsafePointer<UInt8> = Data(bytes: keyData, count: keyData.count).withUnsafeBytes { bytes -> UnsafePointer<UInt8> in
            return bytes
        }
        let k = HDKey_Deserialize(chdKey, cdata, Int32(keyData.count))
        return self.init(k)
    }

    public class func deserialize(_ keyData: Data) -> HDKey {
        var extendedkeyData = keyData
        let cextendedkey = extendedkeyData.withUnsafeMutableBytes { re -> UnsafeMutablePointer<UInt8> in
            return re
        }
        let chdKey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
        let chdkey = HDKey_FromExtendedKey(cextendedkey, Int32(keyData.count), chdKey)
        return self.init(chdkey)
    }

    public class func deserializeBase58(_ keyData: String) -> HDKey {
        let chdKey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
        let hdkey = HDKey_DeserializeBase58(chdKey, keyData.toUnsafePointerInt8()!, Int32(keyData.count))
        return self.init(hdkey)
    }

    public class func paddingToExtendedPrivateKey(_ privateKeyBytes: Data) -> Data {
        var pkData: Data = privateKeyBytes
        let cpks: UnsafeMutablePointer<UInt8> = pkData.withUnsafeMutableBytes { (bytes) -> UnsafeMutablePointer<UInt8> in
            return bytes
        }
        let cextenedkey: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: EXTENDED_PRIVATEKEY_BYTES)

        _ = HDKey_PaddingToExtendedPrivateKey(cpks, 32, cextenedkey, UInt32(EXTENDED_PRIVATEKEY_BYTES))
        let extenedToArrary: UnsafeBufferPointer<UInt8> = UnsafeBufferPointer(start: cextenedkey, count: EXTENDED_PRIVATEKEY_BYTES)
        let extenedData: Data = Data(buffer: extenedToArrary)

        return extenedData
//        return [UInt8](extenedData)
    }

    public func derive(_ path: String) throws -> HDKey {
        let cderivedkey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 256)
        let childNum = try childList(path)
        let hkey = HDKey_GetvDerivedKey(key, cderivedkey, Int32(childNum.count), getVaList(childNum))

        return HDKey(hkey)
    }

    // "44H/0H/0H"
    private func childList(_ path: String) throws -> [CVarArg] {
        var childNum: [CVarArg] = []
        let arraySubstrings: [Substring] = path.split(separator: "/")
        try arraySubstrings.forEach { str in
            if (str.suffix(1) == "H") {
                let v = String(str.prefix(str.count - 1))
                let iV: UInt32 = try UInt32(value: v)
                let value = iV | 0x80000000
                childNum.append(value)
            }
            else {
                let iV: UInt32 = try UInt32(value: String(str))
                childNum.append(UInt32(iV))
            }
        }

        return childNum
    }

    public func derive(_ index: Int, _ hardened: Bool) -> HDKey {
        // TODO: TODO
        return HDKey(key)
    }

    public func derive(_ index: Int) -> HDKey {

        return derive(index, false)
    }

    public func getAddress() -> String {
        let cid = HDKey_GetAddress(self.key)
        return (String(cString: cid!))
    }

    public class func toAddress(_ pk: [UInt8]) -> String {
        var pkData: Data = Data(bytes: pk, count: pk.count)
        let cpks: UnsafeMutablePointer<UInt8> = pkData.withUnsafeMutableBytes { (bytes) -> UnsafeMutablePointer<UInt8> in
            return bytes
        }
        let address: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 48)
        let cid = HDKey_PublicKey2Address(cpks, address, 48)

        return (String(cString: cid!))
    }

    class func PEM_ReadPublicKey(_ publicKey: Data) -> String {
        let cpub: UnsafePointer<UInt8> = publicKey.withUnsafeBytes { (bytes) -> UnsafePointer<UInt8> in
            return bytes
        }
        let cprivateKey: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 512)
        var size: Int32 = 512
        let re = PEM_WritePublicKey(cpub, cprivateKey, &size)
        if re < 0 {
            //TODO: throws
        }
        let cstr = String(cString: cprivateKey)
        return cstr
    }

    class func PEM_ReadPrivateKey(_ publicKey: Data, _ privatekey: Data) throws -> String {
        let cpub: UnsafePointer<UInt8> = publicKey.withUnsafeBytes { bytes -> UnsafePointer<UInt8> in
            return bytes
        }
        let cpri: UnsafePointer<UInt8> = privatekey.withUnsafeBytes { bytes -> UnsafePointer<UInt8> in
            return bytes
        }
        let cPEM_privateKey: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 512)
        var count = 512
        let re = PEM_WritePrivateKey(cpub, cpri, cPEM_privateKey, &count)
        if re < 0 {
            //TODO: throws
        }
        let cstr = String(cString: cPEM_privateKey)
        return cstr
    }

    func wipe() {
        HDKey_Wipe(UnsafeMutablePointer<CHDKey>(mutating: key))
    }
}
