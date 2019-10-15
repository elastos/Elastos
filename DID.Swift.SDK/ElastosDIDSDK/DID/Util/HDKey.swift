import Foundation

public class HDKey: NSObject {
    static let CURVE_ALGORITHM: String = "secp256r1"
    private var seed: Data!
    private var subPublicKey: String!
    private var childPrivatedKey: String!
    
    init(_ seed: Data) {
        self.seed = seed
    }
    
    public class func generateMnemonic(_ language: Int) -> String {
        let newmnemonic: UnsafePointer<Int8> = HDkey_GenerateMnemonic(Int32(language))
        return (String(cString: newmnemonic))
    }
    
    public class func fromMnemonic(_ mnemonic: String, _ passphrase: String) throws -> HDKey {
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
    public func derive(_ index: Int32) throws -> DerivedKey {
        return DerivedKey(seed, index)
    }

    
}
