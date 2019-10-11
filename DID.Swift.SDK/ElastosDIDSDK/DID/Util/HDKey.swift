import Foundation
import BitcoinKit

public class HDKey: NSObject {
    static let CURVE_ALGORITHM: String = "secp256r1"
    private var seed: Data!
    private var rootPrivateKey: HDPrivateKey!
    private var childPrivatedKey: HDPrivateKey!
    
    init(_ seed: Data) {
        self.seed = seed
//        rootPrivateKey = HDPrivateKey.init(seed: seed, network: .testnet)
    }
    
    public static func fromMnemonic(_ mnemonic: String, _ passphrase: String) throws -> HDKey {
        let mpointer: UnsafePointer<Int8> = mnemonic.toUnsafePointerInt8()!
        let passphrasebase58Pointer = passphrase.toUnsafePointerInt8()
        
        var seed: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 64)
        seed = HDkey_GetSeedFromMnemonic(mpointer, passphrasebase58Pointer!, 0, seed)
        let seedPointToArry: UnsafeBufferPointer<Int8> = UnsafeBufferPointer(start: seed, count: 64)
        let seedData: Data = Data(buffer: seedPointToArry)
        return HDKey(seedData)
    }
    
    public func getSeed() -> Data {
        return seed
    }
    
    public class func fromSeed(_ seed: Data) -> HDKey {
        return HDKey(seed)
    }
    
    // DerivedKey 初步推断是bip44 对于BitcoinKit的HDKeychain
    public func derive(_ index: UInt32) throws -> DerivedKey {
        
        let wallet: HDWallet = HDWallet(seed: seed, network: .mainnet)
        let extendedPrivateKey = try wallet.extendedPrivateKey(index: index)
        return DerivedKey(extendedPrivateKey, seed)
    }
}
