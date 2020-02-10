import Foundation

public class DerivedKey: NSObject {
    
    private var index: Int32
    private var seed: Data
    
    public init(_ seed: Data, _ index: Int32) {
        self.seed = seed
        self.index = index
    }
    
    public func getPublicKeyBytes() throws -> [UInt8] {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> [UInt8] in
            let pukey: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 66)
            let chdKey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
            let hdKey: UnsafePointer<CHDKey> = HDKey_GetPrivateIdentity(seeds, 0, chdKey)
            let pk: UnsafeMutablePointer<Int8> = HDKey_GetSubPublicKey(hdKey, 0, index, pukey)
            let pkpointToarry: UnsafeBufferPointer<Int8> = UnsafeBufferPointer(start: pk, count: 33)
            let pkData: Data = Data(buffer: pkpointToarry)
            return [UInt8](pkData)
        }
    }
    
    public func getPublicKeyData() throws -> Data {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> Data in
            let pukey: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 66)
            let chdKey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
            let hdKey: UnsafePointer<CHDKey> = HDKey_GetPrivateIdentity(seeds, 0, chdKey)
            let pk: UnsafeMutablePointer<Int8> = HDKey_GetSubPublicKey(hdKey, 0, index, pukey)
            let pkpointToarry: UnsafeBufferPointer<Int8> = UnsafeBufferPointer(start: pk, count: 33)
            let pkData: Data = Data(buffer: pkpointToarry)
            return pkData
        }
    }
    
    public func getPrivateKeyBytes() throws -> [UInt8] {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> [UInt8] in
            let privateKey: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 64)
            let chdKey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
            let hdKey: UnsafePointer<CHDKey> = HDKey_GetPrivateIdentity(seeds, 0, chdKey)
            let pk: UnsafeMutablePointer<Int8> = HDKey_GetSubPrivateKey(hdKey, 0, 0, index, privateKey)
            let privateKeyPointToarry: UnsafeBufferPointer<Int8> = UnsafeBufferPointer(start: pk, count: 33)
            let pkData: Data = Data(buffer: privateKeyPointToarry)
            return [UInt8](pkData)
        }
    }
    
    public func getPrivateKeyData() throws -> Data {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> Data in
            let privateKey: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 64)
            let chdKey: UnsafeMutablePointer<CHDKey> = UnsafeMutablePointer<CHDKey>.allocate(capacity: 66)
            let hdKey: UnsafePointer<CHDKey> = HDKey_GetPrivateIdentity(seeds, 0, chdKey)
            _ = HDKey_GetSubPrivateKey(hdKey, 0, 0, index, privateKey)
            let privateKeyPointToarry: UnsafeBufferPointer<Int8> = UnsafeBufferPointer(start: privateKey, count: 33)
            let pkData: Data = Data(buffer: privateKeyPointToarry)
            return pkData
        }
    }
    
    public func getPrivateKeyBase58() throws -> String {
        let data8: [UInt8] = try getPrivateKeyBytes()
        return Base58.base58FromBytes(data8)
    }
    
    public func getPublicKeyBase58() throws -> String {
        var pkData: Data = try getPublicKeyData()
        let base58: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 2048)
        let d = pkData.withUnsafeMutableBytes { re -> UnsafeMutablePointer<UInt8> in
            return re
        }
        _ = base58_encode(base58, d, pkData.count)
        let base58Str = String(cString: base58)
        return base58Str
    }
    
    public class func getIdString(_ pk: [UInt8]) -> String {
        var pkData: Data = Data(bytes: pk, count: pk.count)
        let pks: UnsafeMutablePointer<Int8> = pkData.withUnsafeMutableBytes { (bytes) -> UnsafeMutablePointer<Int8> in
            return bytes
        }
        let address: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 48)
        let idstring = HDKey_GetAddress(pks, address, 48)
        return (String(cString: idstring))
    }
}
