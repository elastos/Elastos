import Foundation
//import BitcoinKit

// bip44 对HDKeychain封装
public class DerivedKey: NSObject {
    
    private var index: Int32!
    private var seed: Data!
    private var methodidString: String?
    
    public init(_ seed: Data, _ index: Int32) {
        self.seed = seed
        self.index = index
    }
    
    public func getPublicKeyBytes() throws -> [UInt8] {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> [UInt8] in
            let pukey: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 66)
            let cmasterKey: UnsafeMutablePointer<CMasterPublicKey> = UnsafeMutablePointer<CMasterPublicKey>.allocate(capacity: 66)
            let masterKey: UnsafePointer<CMasterPublicKey> = HDkey_GetMasterPublicKey(seeds, 0, cmasterKey)
            let pk: UnsafeMutablePointer<Int8> = HDkey_GetSubPublicKey(masterKey, 0, index, pukey)
            let pkpointToarry: UnsafeBufferPointer<Int8> = UnsafeBufferPointer(start: pk, count: 33)
            let pkData: Data = Data(buffer: pkpointToarry)
            return [UInt8](pkData)
        }
    }
    
    public func getPublicKeyData() throws -> Data {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> Data in
            let pukey: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 66)
            let cmasterKey: UnsafeMutablePointer<CMasterPublicKey> = UnsafeMutablePointer<CMasterPublicKey>.allocate(capacity: 66)
            let masterKey: UnsafePointer<CMasterPublicKey> = HDkey_GetMasterPublicKey(seeds, 0, cmasterKey)
            let pk: UnsafeMutablePointer<Int8> = HDkey_GetSubPublicKey(masterKey, 0, index, pukey)
            let pkpointToarry: UnsafeBufferPointer<Int8> = UnsafeBufferPointer(start: pk, count: 33)
            let pkData: Data = Data(buffer: pkpointToarry)
            return pkData
        }
    }
    
    public func getPrivateKeyBytes() throws -> [UInt8] {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> [UInt8] in
            let privateKey: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 64)
            let pk: UnsafeMutablePointer<Int8> = HDkey_GetSubPrivateKey(seeds, 0, 0, index, privateKey)
            let privateKeyPointToarry: UnsafeBufferPointer<Int8> = UnsafeBufferPointer(start: pk, count: 33)
            let pkData: Data = Data(buffer: privateKeyPointToarry)
            return [UInt8](pkData)
        }
    }
    
    public func getPrivateKeyData() throws -> Data {
        return seed.withUnsafeMutableBytes { (seeds: UnsafeMutablePointer<Int8>) -> Data in
            let privateKey: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 64)
            _ = HDkey_GetSubPrivateKey(seeds, 0, 0, index, privateKey)
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
        return try Base58.base58FromBytes(getPublicKeyBytes())
    }
    
    public class func getIdString(_ pk: [UInt8]) -> String {
        var pkData: Data = Data(bytes: pk, count: pk.count)
        let pks: UnsafeMutablePointer<Int8> = pkData.withUnsafeMutableBytes { (bytes) -> UnsafeMutablePointer<Int8> in
            return bytes
        }
        let address: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 48)
        let idstring = HDkey_GetIdString(pks, address, 48)
        return (String(cString: idstring!))
    }
}
