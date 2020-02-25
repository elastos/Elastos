import Foundation

public class DerivedKey: NSObject {
    private var index: Int32
    private var seed: Data

    public init(_ seed: Data, _ index: Int32) {
        self.seed = seed
        self.index = index
    }

    /*
    public func getPublicKeyBytes() throws -> [UInt8] {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> [UInt8] in
            let pubKey = UnsafeMutablePointer<Int8>.allocate(capacity: 66)
            let chdKey = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
            let hdKey  = HDKey_GetPrivateIdentity(seeds, 0, chdKey)
            let pk     = HDKey_GetSubPublicKey(hdKey, 0, index, pubKey)
            return [UInt8](Data(buffer: UnsafeBufferPointer(start: pk, count: 33)))
        }
    }
    
    public func getPublicKeyData() throws -> Data {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> Data in
            let pubKey = UnsafeMutablePointer<Int8>.allocate(capacity: 66)
            let chdKey = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
            let hdKey  = HDKey_GetPrivateIdentity(seeds, 0, chdKey)
            let pk     = HDKey_GetSubPublicKey(hdKey, 0, index, pubKey)
            let pkToArray = UnsafeBufferPointer(start: pk, count: 33)
            return Data(buffer: pkToArray)
        }
    }
    
    public func getPrivateKeyBytes() throws -> [UInt8] {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> [UInt8] in
            let prvKey = UnsafeMutablePointer<Int8>.allocate(capacity: 64)
            let chdKey = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
            let hdKey  = HDKey_GetPrivateIdentity(seeds, 0, chdKey)
            let pk     = HDKey_GetSubPrivateKey(hdKey, 0, 0, index, prvKey)
            let pkToArray = UnsafeBufferPointer(start: pk, count: 33)
            return [UInt8](Data(buffer: pkToArray))
        }
    }
    
    public func getPrivateKeyData() throws -> Data {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> Data in
            let prvKey = UnsafeMutablePointer<Int8>.allocate(capacity: 64)
            let chdKey = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
            let hdKey  = HDKey_GetPrivateIdentity(seeds, 0, chdKey)
            _ = HDKey_GetSubPrivateKey(hdKey, 0, 0, index, prvKey)
            let prvToArray = UnsafeBufferPointer(start: prvKey, count: 33)
            return Data(buffer: prvToArray)
        }
    }
    
    public func getPrivateKeyBase58() throws -> String {
        return Base58.base58FromBytes(try getPrivateKeyBytes())
    }
    
    public func getPublicKeyBase58() throws -> String {
        var pkData = try getPublicKeyData()
        let base58 = UnsafeMutablePointer<Int8>.allocate(capacity: 2048)
        let data = pkData.withUnsafeMutableBytes { result -> UnsafeMutablePointer<UInt8> in
            return result
        }
        _ = base58_encode(base58, data, pkData.count)
        return String(cString: base58)
    }
    
    public class func getAddress(_ pk: [UInt8]) -> String {
        var pkData = Data(bytes: pk, count: pk.count)
        let pks = pkData.withUnsafeMutableBytes { (bytes) -> UnsafeMutablePointer<Int8> in
            return bytes
        }
        let address = UnsafeMutablePointer<Int8>.allocate(capacity: 48)
        let addrstr = HDKey_GetAddress(pks, address, 48)
        return String(cString: addrstr)
    }

    func serialize() -> Data {
        // TODO
        return Data()
    }

    class func deserialize(_ data: Data) -> DerivedKey? {
        // TODO:
        return nil
    }

    func getAddress() -> String {
        // TODO:
        return "TODO"
    }

    func wipe() {
        // TODO:
    }
    */
}
