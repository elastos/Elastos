import Foundation

public class Mnemonic {

    public class func generate(_ language: Int) throws -> String {
        guard language >= 0 else {
            throw DIDError.illegalArgument("")
        }
        let newmnemonic: UnsafePointer<Int8> = HDKey_GenerateMnemonic(Int32(language))
        return (String(cString: newmnemonic))
    }
    
    public class func isValid(_ language: Int, _ mnemonic: String) throws -> Bool {
        guard language >= 0 else {
            throw DIDError.illegalArgument("")
        }
        let mpointer: UnsafePointer<Int8> = mnemonic.toUnsafePointerInt8()!
        return HDKey_MnemonicIsValid(mpointer, Int32(language))
    }
}
