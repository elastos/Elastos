import Foundation

class HDKey: NSObject {
    static let PUBLICKEY_BYTES : Int = CHDKey.PUBLICKEY_BYTES
    static let PRIVATEKEY_BYTES: Int = CHDKey.PRIVATEKEY_BYTES
    static let EXTENDED_PRIVATE_BYTES: Int = CHDKey.EXTENDEDKEY_BYTES
    static let SEED_BYTES: Int = 64
    
    private var chdKey: UnsafePointer<CHDKey>

    class DerivedKey {
        private var cderivedKey: UnsafePointer<CDerivedKey>

        init(_ cderivedKey: UnsafePointer<CDerivedKey>) {
            self.cderivedKey = cderivedKey
        }

        public class func getAddress(_ pk: [UInt8]) -> String  {
            // TODO:
            return "TODO"
        }

        func serialize() -> Data {
            // TODO:
            return Data()
        }

        func getAddress() -> String {
            // TODO:
            return "TODO"
        }

        func getPublicKeyBase58() -> String {
            // TODO:
            return "TODO"
        }

        class func deserialize(_ data: Data) -> DerivedKey? {
            // TODO:
            return nil
        }

        func wipe() {
            // TODO:
        }
    }

    init(_ chdKey: UnsafePointer<CHDKey>) {
        self.chdKey = chdKey
    }

    class func fromMnemonic(_ mnemonic: String, _ passPhrase: String, _ language: Int) -> HDKey {
        let buffer = UnsafeMutablePointer<CHDKey>.allocate(capacity: 1)
        let chdKey = mnemonic.withCString { cmnemonic in
            return passPhrase.withCString { cpassphase in
                return HDKey_FromMnemonic(cmnemonic, cpassphase, language, buffer)
            }
        }
        return HDKey(chdKey)
    }

    class func fromSeed(_ seed: Data) -> HDKey {
        let buffer = UnsafeMutablePointer<CHDKey>.allocate(capacity: 1)
        let chdKey = seed.withUnsafeBytes { (cseed: UnsafePointer<Int8>) in
            return HDKey_FromSeed(cseed, seed.count, buffer)
        }
        return HDKey(chdKey)
    }

    class func fromExtendedKey(_ extendedKey: Data) -> HDKey {
        let buffer = UnsafeMutablePointer<CHDKey>.allocate(capacity: 1)
        let chdKey = extendedKey.withUnsafeBytes { (ckey: UnsafePointer<Int8>) in
            return HDKey_FromExtendedKey(ckey, extendedKey.count, buffer)
        }
        return HDKey(chdKey)
    }

    func derivedKey(_ index: Int) -> HDKey.DerivedKey {
        let buffer = UnsafeMutablePointer<CDerivedKey>.allocate(capacity: 1)
        let cdrivedKey = HDKey_GetDerivedKey(chdKey, index, buffer)
        return DerivedKey(cdrivedKey)
    }

    func serialize() -> Data {
        let length = MemoryLayout.size(ofValue: CHDKey.self)
        let buffer = UnsafeMutablePointer<Int8>.allocate(capacity: length)
        let result = HDKey_Serialize(chdKey, buffer, length)
        return Data(bytes: UnsafeRawPointer(buffer), count: result)
    }

    func serialize() -> String {
        // TODO:
        return "TODO"
    }

    func wipe() {
        HDKey_Wipe(UnsafeMutablePointer<CHDKey>(mutating: chdKey))
    }
}
