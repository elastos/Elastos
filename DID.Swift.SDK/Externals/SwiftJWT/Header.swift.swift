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

    public func setType(_ type: String) -> Header {
        headers[Header.TYPE] = type
        return self
    }

    public func getType() -> String? {
        return headers[Header.TYPE] as? String
    }

    public func setContentType(_ contentType: String) -> Header {
        headers[Header.CONTENT_TYPE] = contentType
        return self
    }

    public func getContentType() -> String? {
        return headers[Header.CONTENT_TYPE] as? String
    }

    public func setValue(key: String, value: Any) -> Header {
        headers[key] = value
        return self
    }

    public func getValue(key: String) -> Any? {
        return headers[key] as Any
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

