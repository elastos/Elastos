/**
 * Copyright IBM Corporation 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

import Foundation

// MARK: Header
public class Header {

    public static let JWT_TYPE: String = "JWT"
    /// Type Header Parameter
    public static let TYPE: String = "typ"
    /// Algorithm Header Parameter
    public static let alg: String = "alg"
    /// JSON Web Token Set URL Header Parameter
    public static let jku: String = "jku"
    /// JSON Web Key Header Parameter
    public static let jwk: String = "jwk"
    /// Key ID Header Parameter
    public static let kid: String = "kid"
    /// X.509 URL Header Parameter
    public static let x5u: String = "x5u"
    /// X.509 Certificate Chain Header Parameter
    public static let x5c: String = "x5c"
    /// X.509 Certificate SHA-256 Thumbprint Header Parameter
    public static let x5t: String = "x5t"
    /// X.509 Certificate SHA-256 Thumbprint Header Parameter
    public static let x5tS256: String = "x5tS256"
    /// Content Type Header Parameter
    public static let CONTENT_TYPE: String = "cty"
    /// Critical Header Parameter
    public static let crit: String = "crit"

    var headers: [String: Any] = [: ]

    public init() { }

    /// Set header 'typ'.
    /// - Parameter type: The type value.
    /// - Returns: Header instance.
    public func setType(_ type: String) -> Header {
        headers[Header.TYPE] = type
        return self
    }

    /// Get header type.
    /// - Returns: If has, return value string. Otherwise, return nil.
    public func getType() -> String? {
        return headers[Header.TYPE] as? String
    }

    /// Set header 'cty'.
    /// - Parameter contentType: The content type value.
    /// - Returns: Header instance.
    public func setContentType(_ contentType: String) -> Header {
        headers[Header.CONTENT_TYPE] = contentType
        return self
    }

    /// Get header content type.
    /// - Returns: If has, return value string. Otherwise, return nil.
    public func getContentType() -> String? {
        return headers[Header.CONTENT_TYPE] as? String
    }

    /// Set header key-value.
    /// - Parameters:
    ///   - key: The key string.
    ///   - value: The value string.
    /// - Returns: Header instance.
    public func setValue(key: String, value: Any) -> Header {
        headers[key] = value
        return self
    }

    /// Get header value by header key.
    /// - Parameter key: The key to header.
    /// - Returns: If has, return value. Otherwise, return nil.
    public func getValue(key: String) -> Any? {
        return headers[key] as Any
    }

    /// Get Header count.
    /// - Returns: Header count.
    public func size() -> Int {
        return headers.count
    }

    /// Check header is empty or not.
    /// - Returns: true if header is empty, otherwise false.
    public func isEmpty() -> Bool {
        return headers.isEmpty
    }

    /// Check key if headers key or not.
    /// - Parameter key: The key string.
    /// - Returns: True if has headers key, or false.
    public func containsKey(key: String) -> Bool {
        return headers[key] != nil
    }

    /// Check key if headers value or not.
    /// - Parameter value: The value string.
    /// - Returns: True if has headers value, or false.
    public func containsValue(value: Any) -> Bool {
        for v in headers.values {
            if v as AnyObject === value as AnyObject {
                return true
            }
        }
        return false
    }

    /// Get header value by header key.
    /// - Parameter key: The key string.
    /// - Returns: If has, return value. Otherwise, return nil.
    public func get(key: String) -> Any? {
        return headers[key]
    }

    /// Add header value by header key.
    /// - Parameters:
    ///   - key: The key string.
    ///   - value: The value string.
    public func put(key: String, value: String) {
        headers[key] = value
    }

    /// Remove header value by header key.
    /// - Parameter key: The key string.
    /// - Returns: If has, return value. Otherwise, return nil.
    public func remove(key: String) -> Any? {
        let value = headers[key]
        headers.removeValue(forKey: key)

        return value
    }

    /// Add header value by dictionary.
    /// - Parameter dic: The header key-value.
    public func putAll(dic: [String: Any]) {
        headers.merge(dict: dic)
    }

    /// Clear header
    public func clear() {
        headers.removeAll()
    }

    /// Get header values.
    /// - Returns: Array of header.
    public func values() -> [Any] {
        var values = [Any]()
        headers.forEach { k, v in
            values.append(v)
        }
        return values
    }

    func encode() throws -> String  {
        let data = try JSONSerialization.data(withJSONObject: headers, options: [])
//        print(String(data: data, encoding: .utf8))
        return JWTEncoder.base64urlEncodedString(data: data)
    }

    class func decode(_ data: Data) throws -> Header {
         let dic = try JSONSerialization.jsonObject(with: data, options: []) as? [String: Any]
         let hea = Header()
         if dic != nil {
             hea.headers = dic!
         }
         return hea
     }
}

