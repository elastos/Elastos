import Foundation

public class HDKey: NSObject {
    public static let PUBLICKEY_BYTES: Int = 33
    public static let PRIVATEKEY_BYTES: Int = 32
    
    private var seed: Data!
    init(_ seed: Data) {
        self.seed = seed
    }
    
    public class func generateMnemonic(_ language: Int) -> String {
        let newmnemonic: UnsafePointer<Int8> = HDkey_GenerateMnemonic(Int32(language))
        return (String(cString: newmnemonic))
    }
    
    public class func fromMnemonic(_ mnemonic: String, _ passphrase: String) throws -> HDKey {
        let mnem: String = mnemonic
        let passph: String = passphrase
        let mpointer: UnsafePointer<Int8> = mnem.toUnsafePointerInt8()!
        let passphrasebase58Pointer = passph.toUnsafePointerInt8()
        
        var seedPinter: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 64)
        seedPinter = HDkey_GetSeedFromMnemonic(mpointer, passphrasebase58Pointer!, 0, seedPinter)
        let seedPointToArry: UnsafeBufferPointer<Int8> = UnsafeBufferPointer(start: seedPinter, count: 64)
        let seedData: Data = Data(buffer: seedPointToArry)
        print(seedData.hexEncodedString())
        return HDKey(seedData)
    }
    
    public func getSeed() -> Data {
        return seed
    }
    
    public class func fromSeed(_ seed: Data) -> HDKey {
        return HDKey(seed)
    }
    
    // DerivedKey 初步推断是bip44 对于BitcoinKit的HDKeychain
    public func derive(_ index: Int) throws -> DerivedKey {
        return DerivedKey(seed, Int32(index))
    }
}
