/*
* Copyright (c) 2020 Elastos Foundation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

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

    /// Gernerate a random mnemonic.
    /// - Parameter language: The language for DID.
    /// support language string: “chinese_simplified”, “chinese_traditional”, “czech”, “english”, “french”, “italian”, “japanese”, “korean”, “spanish”.
    /// - Throws: Language is empty or failure to generate mnemonic will throw error.
    /// - Returns: Random mnemonic.
    public class func generate(_ language: String) throws -> String {
        guard !language.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let result = language.withCString { (clanuage) in
            return HDKey_GenerateMnemonic(clanuage)
        }

        guard let _ = result else {
            throw DIDError.illegalArgument()
        }

        return String(cString: result!)
    }
    
    /// Check mnemonic.
    /// - Parameters:
    ///   - language: The language for DID.
    ///   support language string: “chinese_simplified”, “chinese_traditional”, “czech”, “english”, “french”, “italian”, “japanese”, “korean”, “spanish”.
    ///   - mnemonic: mnemonic string.
    /// - Throws: mnemonic or language is empty.
    /// - Returns: true, if mnemonic is valid. or else, return false.
    public class func isValid(_ language: String, _ mnemonic: String) throws -> Bool {
        guard !mnemonic.isEmpty else {
            throw DIDError.illegalArgument("Invalid mnemonic.")
        }

        guard !language.isEmpty else {
            throw DIDError.illegalArgument("Invalid password..")
        }

        return language.withCString { (clang) in
            return mnemonic.withCString { (cmnemonic) in
                return HDKey_MnemonicIsValid(cmnemonic, clang)
            }
        }
    }
}
