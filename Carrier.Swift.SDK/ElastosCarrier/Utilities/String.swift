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

internal extension String {

    internal static func toHardString(_ str: String?) -> String {
        if (str == nil) {
            return "<nil>"
        } else if (str == "") {
            return "<empty>"
        } else {
            return str!
        }
    }

    internal init<T>(cCharPointer pointer: UnsafePointer<T>) {
        self.init(cString: UnsafeRawPointer(pointer).assumingMemoryBound(to: CChar.self))
    }

    @inline(__always) func writeToCCharPointer<T>(_ pointer : UnsafeMutablePointer<T>) {
        self.withCString({ src in
            let ptr = UnsafeMutableRawPointer(pointer).assumingMemoryBound(to: CChar.self)
            strcpy(ptr, src)
        })
    }
}

@inline(__always) internal func createCStringDuplicate(_ field: String?) -> UnsafePointer<CChar>? {
    if field != nil {
        return UnsafePointer<CChar>(field!.withCString {
            return strdup($0)
        })
    } else {
        return nil
    }
}

@inline(__always) internal func deallocCString(_ field: UnsafePointer<CChar>?) {
    if (field != nil) {
        free(UnsafeMutablePointer<CChar>(mutating: field))
    }
}
