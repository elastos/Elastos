/*
 * Copyright (c) 2018 Elastos Foundation
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

internal class Base58 {
    private static let alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"

    internal static func encode(_ input: [UInt8]) -> String {
        var zeroCount = 0
        for ch in input {
            if ch == 0 {
                zeroCount += 1
            }
            else {
                break;
            }
        }

        var number = [UInt]()
        for i in zeroCount ..< input.count {
            var carry = UInt(input[i])

            var j = number.count - 1
            while j >= 0 {
                let temp = number[j] << 8 + carry
                number[j] = temp % 58
                carry = temp / 58
                j -= 1
            }

            while carry > 0 {
                number.insert(carry % 58, at: 0)
                carry /= 58
            }
        }

        let byteAlphabet = alphabet.utf8CString
        var text = [CChar](repeating: byteAlphabet[0], count: zeroCount)
        for index in number {
            text.append(byteAlphabet[Int(index)])
        }
        text.append(0)

        return String(cString: text)
    }

    internal static func decode(_ input: String) -> [UInt8]? {
        let byteInput = input.utf8CString
        let byteAlphabet = alphabet.utf8CString

        var zeroCount = 0
        let zeroChar = byteAlphabet[0]
        for ch in byteInput {
            if ch == zeroChar {
                zeroCount += 1
            }
            else {
                break;
            }
        }

        var number = [UInt8]()
        for i in zeroCount ..< byteInput.count - 1 {
            let ch = byteInput[i]

            guard var carry = byteAlphabet.index(of: ch) else {
                return nil
            }

            var j = number.count - 1
            while j >= 0 {
                let temp = Int(number[j]) * 58 + carry
                number[j] = UInt8(temp & 0xFF)
                carry = (temp & (~0xFF)) >> 8
                j -= 1
            }

            while carry > 0 {
                number.insert(UInt8(carry & 0xFF), at: 0)
                carry = (carry & (~0xFF)) >> 8
            }
        }
        
        return [UInt8](repeating: 0, count: zeroCount) + number
    }
}
