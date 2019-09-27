import Foundation
import BitcoinKit

// bip44 对HDKeychain封装
public class DerivedKey: NSObject {
    
    private var _privateKey: HDPrivateKey!
    private var _wallet: HDWallet!
    
    public init(_ pk: HDPrivateKey, _ wallet: HDWallet) {
        _privateKey = pk
        _wallet = wallet
    }
    
    // 初步猜测是获取公钥bytes数组
    public func getPublicKeyBytes() throws -> [UInt8] {
        
        let publicKeyData = try _wallet.publicKey(index: 0).raw
        let byteArray = [UInt8](publicKeyData)
        return byteArray
    }
    
    public func getRedeemScript(_ pk: [UInt8]) throws -> [UInt8] {
        
        var script: [UInt8] = [UInt8](repeating: 0, count: 35)
        script[0] = 33
        // https://stackoverflow.com/questions/37200341/how-to-implement-java-arraycopy-in-swift
        script[1...33] = pk[0...32]
        script[34] = 0xAD
        return script;
    }
    
    public func sha256Ripemd160(_ input: [UInt8]) -> [UInt8] {
        // bytes 数组 -> data
        let data: Data = Data(bytes: input, count: input.count)
        let out: Data = Crypto.sha256ripemd160(data)
        return [UInt8](out)
    }
    
    // 从公钥导出一个跟address相关的bytes数组
    public func getBinAddress(_ pk: [UInt8]) throws -> [UInt8] {
        
        let script = try getRedeemScript(pk)
        var hash: [UInt8] = sha256Ripemd160(script)
        var programHash: [UInt8] = [UInt8](repeating: 0, count: hash.count + 1)
        programHash[0] = 0x67
        programHash[1...hash.count] = hash[0...hash.count - 1]
        
        let hashData: Data = Crypto.sha256sha256(Data(bytes: &programHash, count: programHash.count))
        hash = [UInt8](hashData)
        
        var binAddress: [UInt8] = [UInt8](repeating: 0, count: programHash.count + 4)
        binAddress[0...programHash.count - 1] = programHash[0...programHash.count - 1]
        binAddress[programHash.count...programHash.count + 3] = hash[0...3]
        return binAddress
    }
    
   class public func getAddress(_ pk: [UInt8]) {
            return Base58.encode(pk)
    }

}
