

import Foundation
import BitcoinKit

public class HDKey: NSObject {
    static let CURVE_ALGORITHM: String = "secp256r1"
    private var seed: Data!
    private var rootPrivateKey: HDPrivateKey!
    private var childPrivatedKey: HDPrivateKey!

    init(_ seed: Data) {
        self.seed = seed
        rootPrivateKey = HDPrivateKey.init(seed: seed, network: .testnet)
    }

    public static func fromMnemonic(_ mnemonic: String, _ passphrase: String) throws -> HDKey {
        // TODO: mnemonic conver to array string
        let seed = Mnemonic.seed(mnemonic: [], passphrase: passphrase)
        return HDKey(seed)
    }

    public func getSeed() -> Data {
        return seed
    }

    public class func fromSeed(_ seed: Data) -> HDKey {
        return HDKey(seed)
    }

    public func derive(_ index: Int) throws -> HDPrivateKey {
       childPrivatedKey = try rootPrivateKey.derived(at: UInt32(index))
        return childPrivatedKey
    }

}
