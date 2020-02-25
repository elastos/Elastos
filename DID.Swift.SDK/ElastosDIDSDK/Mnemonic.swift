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

    enum Language: Int {
        case LANGUAGE_ENGLISH = 0
        case LANGUAGE_FRENCH = 1
        case LANGUAGE_SPANISH = 2
        case LANGUAGE_JAPANESE = 3
        case LANGUAGE_CHINESE_SIMPLIFIED = 4
        case LANGUAGE_CHINESE_TRADITIONAL = 5

        static func valueOf(_ str: String) -> Language? {
            let language: Language?

            switch str.lowercased() {
            case Mnemonic.ENGLISH:
                language = .LANGUAGE_ENGLISH

            case Mnemonic.FRENCH:
                language = .LANGUAGE_FRENCH

            case Mnemonic.SPANISH:
                language = .LANGUAGE_SPANISH

            case Mnemonic.JAPANESE:
                language = .LANGUAGE_JAPANESE

            case Mnemonic.CHINESE_SIMPLIFIED:
                language = .LANGUAGE_CHINESE_SIMPLIFIED

            case Mnemonic.CHINESE_TRADITIONAL:
                language = .LANGUAGE_CHINESE_TRADITIONAL

            default:
                language = nil
            }
            return language
        }
    }

    public class func generate(_ language: String) throws -> String {
        let lang = Language.valueOf(language)
        guard let _ = lang else {
            throw DIDError.illegalArgument()
        }

        return String(cString: HDKey_GenerateMnemonic(Int32(lang!.rawValue)))
    }
    
    public class func isValid(_ language: String, _ mnemonic: String) throws -> Bool {
        guard !language.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard !mnemonic.isEmpty else {
            throw DIDError.illegalArgument()
        }
        let lang = Language.valueOf(language)
        guard let _ = lang else {
            throw DIDError.illegalArgument()
        }

        return HDKey_MnemonicIsValid(mnemonic.toUnsafePointerInt8()!, Int32(lang!.rawValue))
    }

    class func getLanguageId(_ language: String) -> Int {
        return Language.valueOf(language)?.rawValue ?? -1
    }
}
