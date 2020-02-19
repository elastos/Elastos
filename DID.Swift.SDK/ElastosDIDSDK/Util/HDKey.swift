import Foundation

class HDKey: NSObject {
    static let PUBLICKEY_BYTES : Int = 33
    static let PRIVATEKEY_BYTES: Int = 32
    
    private var _seed: Data

    init(_ seed: Data) {
        self._seed = seed
    }

    class func fromMnemonic(_ mnemonic: String, _ passphrase: String) throws -> HDKey {
        var seedPinter = UnsafeMutablePointer<Int8>.allocate(capacity: 64)
        seedPinter = HDKey_GetSeedFromMnemonic(mnemonic.toUnsafePointerInt8()!,
                                               passphrase.toUnsafePointerInt8()!,
                                               0,
                                               seedPinter)
        let seed = Data(buffer: UnsafeBufferPointer(start: seedPinter, count: 64))
        // print(seed.hexEncodedString())
        return HDKey(seed)
    }

    var seed: Data {
        return _seed
    }

    func derive(_ index: Int) throws -> DerivedKey {
        return DerivedKey(seed, Int32(index))
    }

    func wipe() {
        // TODO:
    }
}
