import Foundation
//import BitcoinKit

// bip44 对HDKeychain封装
public class DerivedKey: NSObject {
    
    private var index: Int32!
    private var seed: Data!
    
    public init(_ seed: Data, _ index: Int32) {
        self.seed = seed
        self.index = index
    }
    
    // 初步猜测是获取公钥bytes数组
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
    
    public class func getIdString(_ pk: [Int8]) -> String {
        var pkData: Data = Data(bytes: pk, count: pk.count)
        let pks: UnsafeMutablePointer<Int8> = pkData.withUnsafeMutableBytes { (bytes) -> UnsafeMutablePointer<Int8> in
            return bytes
        }
        let address: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 48)
        let idstring = HDkey_GetIdString(pks, address, 48)
        return (String(cString: idstring!))
    }
    
    public func getRedeemScript(_ pk: [UInt8]) throws -> [UInt8] {
        
        var script: [UInt8] = [UInt8](repeating: 0, count: 35)
        script[0] = 33
        // https://stackoverflow.com/questions/37200341/how-to-implement-java-arraycopy-in-swift
        script[1...33] = pk[0...32]
        script[34] = 0xAD
        return script
    }
}
