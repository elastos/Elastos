import Foundation

public class Mnemonic {
    public class func generate(_ language: Int) throws -> String {
        guard language >= 0 else {
            throw DIDError.illegalArgument()
        }
        return String(cString: HDKey_GenerateMnemonic(Int32(language)))
    }
    
    public class func isValid(_ language: Int, _ mnemonic: String) throws -> Bool {
        guard language >= 0 else {
            throw DIDError.illegalArgument()
        }
        guard !mnemonic.isEmpty else {
            throw DIDError.illegalArgument()
        }
        return HDKey_MnemonicIsValid(mnemonic.toUnsafePointerInt8()!, Int32(language))
    }
}
