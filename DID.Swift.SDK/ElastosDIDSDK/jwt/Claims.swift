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

// MARK: Claims
public class Claims {
    /**
     The "iss" (issuer) claim identifies the principal that issued the
     JWT.  The processing of this claim is generally application specific.
     The "iss" value is a case-sensitive.
     */
    public static let iss: String = "iss"

    /**
     The "sub" (subject) claim identifies the principal that is the
     subject of the JWT.  The claims in a JWT are normally statements
     about the subject.  The subject value MUST either be scoped to be
     locally unique in the context of the issuer or be globally unique.
     The processing of this claim is generally application specific.  The
     "sub" value is case-sensitive.
     */
    public static let sub: String = "sub"

    /**
     The "aud" (audience) claim identifies the recipients that the JWT is
     intended for.  Each principal intended to process the JWT MUST
     identify itself with a value in the audience claim.  If the principal
     processing the claim does not identify itself with a value in the
     "aud" claim when this claim is present, then the JWT MUST be
     rejected. The interpretation of audience values is generally application specific.
     The "aud" value is case-sensitive.
     */
    public static let aud: String = "aud"

    /**
     The "exp" (expiration time) claim identifies the expiration time on
     or after which the JWT MUST NOT be accepted for processing.  The
     processing of the "exp" claim requires that the current date/time
     MUST be before the expiration date/time listed in the "exp" claim.
     Implementers MAY provide for some small leeway, usually no more than
     a few minutes, to account for clock skew.
     */
    public static let exp: String = "exp"

    /**
     The "nbf" (not before) claim identifies the time before which the JWT
     MUST NOT be accepted for processing.  The processing of the "nbf"
     claim requires that the current date/time MUST be after or equal to
     the not-before date/time listed in the "nbf" claim.  Implementers MAY
     provide for some small leeway, usually no more than a few minutes, to
     account for clock skew.
     */
    public static let nbf: String = "nbf"

    /**
     The "iat" (issued at) claim identifies the time at which the JWT was
     issued.  This claim can be used to determine the age of the JWT.
     */
    public static let iat: String = "iat"

    /**
     The "jti" (JWT ID) claim provides a unique identifier for the JWT.
     The identifier value MUST be assigned in a manner that ensures that
     there is a negligible probability that the same value will be
     accidentally assigned to a different data object; if the application
     uses multiple issuers, collisions MUST be prevented among values
     produced by different issuers as well.  The "jti" claim can be used
     to prevent the JWT from being replayed.  The "jti" value is case-
     sensitive
     */
    public static let jti: String = "jti"

    var claims: [String: Any] = [: ]

    public init() { }

    /// Get jwt issuer.
    /// - Returns: If has, return issuer string. Otherwise, return nil.
    public func getIssuer() -> String? {
        return claims[Claims.iss] as? String
    }

    /// Set JWT issuer.
    /// - Parameter issuer: The issuer value.
    /// - Returns: Claims instance.
    public func setIssuer(issuer: String) -> Claims {
        claims[Claims.iss] = issuer
        return self
    }

    /// Set JWT subject.
    /// - Parameter subject: The subject value.
    /// - Returns: Claims instance.
    public func setSubject(subject: String) -> Claims {
        claims[Claims.sub] = subject
        return self
    }

    /// Get JWT subject.
    /// - Returns: If has, return subject string. Otherwise, return nil.
    public func getSubject() -> String? {
        return claims[Claims.sub] as? String
    }

    /// Get jwt audience.
    /// - Returns: If has, return audience string. Otherwise, return nil.
    public func getAudience() -> String? {
        return claims[Claims.aud] as? String
    }

    /// Set JWT audience.
    /// - Parameter audience: The audience value.
    /// - Returns: Claims instance.
    public func setAudience(audience: String) -> Claims {
        claims[Claims.aud] = audience
        return self
    }

    /// Get expirate date.
    /// - Returns: If has, return jwt expiration Date. Otherwise, return nil.
    public func getExpiration() -> Date? {
        return DateHelper.getDateFromTimeStamp(claims[Claims.exp] as? Int)
    }

    /// Set expirate date.
    /// - Parameter expiration: The date of jwt expiration.
    /// - Returns: JwtBuilder instance.
    public func setExpiration(expiration: Date) -> Claims {
        claims[Claims.exp] = DateHelper.getTimeStamp(expiration)
        return self
    }

    /// Get jwt 'nbf' time.
    /// - Returns: If has, return not before Date. Otherwise, return nil.
    public func getNotBefore() -> Date? {
        return DateHelper.getDateFromTimeStamp(claims[Claims.nbf] as? Int)
    }

    /// Set JWT ‘nbf’ value.
    /// - Parameter notBefore: The ‘nbf’ value.
    /// - Returns: Claims instance.
    public func setNotBefore(notBefore: Date) -> Claims {
        claims[Claims.nbf] = DateHelper.getTimeStamp(notBefore)
        return self
    }

    /// Get jwt issued time.
    /// - Returns: If has, return 'iat' Date. Otherwise, return nil.
    public func getIssuedAt() -> Date? {
        
        return DateHelper.getDateFromTimeStamp(claims[Claims.iat] as? Int)
    }

    /// Set JWT issued time.
    /// - Parameter issuedAt: The ‘iat’ value.
    /// - Returns: Claims instance.
    public func setIssuedAt(issuedAt: Date) -> Claims {
        claims[Claims.iat] = DateHelper.getTimeStamp(issuedAt)
        return self
    }

    /// Get jwt id.
    /// - Returns: If has, return id string. Otherwise, return nil.
    public func getId() -> String? {
        return claims[Claims.jti] as? String
    }

    /// Set JWT id.
    /// - Parameter id: The Id value
    /// - Returns: Claims instance.
    public func setId(id: String) -> Claims {
        claims[Claims.jti] = id
        return self
    }

    /// Get claims count.
    /// - Returns: The count of claim.
    public func size() -> Int {
        return claims.count
    }

    /// Check claim is empty or not.
    /// - Returns: true if claim is empty, otherwise false.
    public func isEmpty() -> Bool {
        return claims.isEmpty
    }

    ///  Check key if claims key or not.
    /// - Parameter key: The key string.
    /// - Returns: True if has claims key, or false.
    public func containsKey(key: String) -> Bool {
        return claims[key] != nil
    }

    /// Check key if claims value or not.
    /// - Parameter value: The value string.
    /// - Returns: True if has claims value, or false.
    public func containsValue(value: Any) -> Bool {
        for v in claims.values {
            if v as AnyObject === value as AnyObject {
                return true
            }
        }
        return false
    }

    /// Get claim value by claim key.
    /// - Parameter key: The key string.
    /// - Returns: If has, return value. Otherwise, return nil.
    public func get(key: String) -> Any {
        return claims[key] as Any
    }

    /// Get value string by claim key
    /// - Parameter key: The key string.
    /// - Throws: If error occurs,throw error.
    /// - Returns: Claim value string
    public func getAsJson(key: String) throws -> String {
        let v = claims[key]
        if !(v is String) && v != nil {
            let data = try JSONSerialization.data(withJSONObject: v as Any, options: [])
            return (String(data: data, encoding: .utf8)!)
        }
        throw DIDError.illegalArgument("Data parsing error in method in getAsJson().")
    }

    /// Add claim value by key-value.
    /// - Parameters:
    ///   - key: The key string.
    ///   - value: The value string.
    public func put(key: String, value: Any) {
        claims[key] = value
    }

    /// Add claim value by key-value.
    /// - Parameters:
    ///   - key: The key string.
    ///   - value: The value string.
    /// - Throws: If error occurs,throw error.
    public func putWithJson(key: String, value: String) throws {
        let dic = try JSONSerialization.jsonObject(with: value.data(using: .utf8)!, options: [])
        claims[key] = dic
    }

    /// Remove claim value by claim key.
    /// - Parameter key: The key string.
    /// - Returns: If has, return value. Otherwise, return nil.
    public func remove(key: String) -> Any? {
        let value = claims[key]
        claims.removeValue(forKey: key)

        return value
    }

    /// Add claim value by dictionary.
    /// - Parameter dic: The header key-value.
    public func putAll(dic: [String: Any]) {
        claims.merge(dict: dic)
    }

    /// Add claim value by json string.
    /// - Parameter json: The header json string.
    /// - Throws: If error occurs,throw error.
    public func putAllWithJson(json: String) throws {
        let dic = try JSONSerialization.jsonObject(with: json.data(using: .utf8)!, options: []) as? [String : Any]
        guard dic != nil else {
            throw DIDError.illegalArgument("Data parsing error in method in putAllWithJson().")
        }
        putAll(dic: dic!)
    }

    /// Clear claim
    public func clear() {
        claims.removeAll()
    }

    /// Get claim values.
    /// - Returns: Array of claim.
    public func values() -> [Any] {
        var values = [Any]()
        claims.forEach { k, v in
            values.append(v)
        }
        return values
    }

    /// Add claim value by key-value.
    /// - Parameters:
    ///   - key: The key string.
    ///   - value: The value string
    /// - Returns: Claims instnce.
    public func setValue(key: String, value: Any) -> Claims {
        claims[key] = value
        return self
    }
}

public extension Claims {
    
    func encode() throws -> String {
        let data = try JSONSerialization.data(withJSONObject: claims, options: [])
//        print(String(data: data, encoding: .utf8))
        return JWTEncoder.base64urlEncodedString(data: data)
    }

   class func decode(_ data: Data) throws -> Claims {
        let dic = try JSONSerialization.jsonObject(with: data, options: []) as? [String: Any]
        let cla = Claims()
        if dic != nil {
            cla.claims = dic!
        }
        return cla
    }
}
