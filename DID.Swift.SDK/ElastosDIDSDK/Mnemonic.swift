import Foundation

public class Mnemonic {
    public static let CHINESE_SIMPLIFIED = "chinese_simplified"
    public static let CHINESE_TRADITIONAL = "chinese_traditional";
    public static let CZECH = "Czech";
    public static let ENGLISH = "english";
    public static let FRENCH = "French";
    public static let ITALIAN = "Italian";
    public static let JAPANESE = "japanese";
    public static let KOREAN = "Korean";
    public static let SPANISH = "Spanish";

    public class func generate(_ language: Int) throws -> String {
        guard language >= 0 else {
            throw DIDError.illegalArgument()
        }
        return String(cString: HDKey_GenerateMnemonic(Int32(language)))
    }
    
    public class func isValid(_ language: String, _ mnemonic: String) throws -> Bool {
        guard !language.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard !mnemonic.isEmpty else {
            throw DIDError.illegalArgument()
        }

        return HDKey_MnemonicIsValid(mnemonic.toUnsafePointerInt8()!, Int32(language)!) // TODO:
    }
}
