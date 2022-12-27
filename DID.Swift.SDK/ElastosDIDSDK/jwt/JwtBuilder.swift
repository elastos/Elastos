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

public class JwtBuilder {
    var h: Header?
    var c: Claims?
    var signature: String?
    var issuer: String
    var jwt: JWT?
    var publicKeyClosure: ((_ id: String?) throws -> Data)?
    var privateKeyClosure: ((_ id: String?, _ storepass: String) throws -> Data)?

    init(issuer: String, publicKey: @escaping (_ id: String?) throws -> Data, privateKey: @escaping (_ id: String?, _ storepass: String) throws -> Data) {
        self.issuer = issuer
        publicKeyClosure = publicKey
        privateKeyClosure = privateKey
    }

    /// Create header for jwt.
    /// - Returns: Header instance.
    public class func createHeader() -> Header {
        return Header()
    }

    /// Create header for jwt.
    /// - Parameter header: The param of header.
    /// - Returns: Header instance.
    public class func createHeader(header: [String: Any]) -> Header {
        let he = Header()
        he.putAll(dic: header)
        return he
    }

    /// Set header for jwt.
    /// - Parameter header: The param of header.
    /// - Returns: JwtBuilder instance.
    public func setHeader(_ header: Header) -> JwtBuilder {
        h = header
        return self
    }

    /// Set header for jwt.
    /// - Parameter header: The param of header.
    /// - Returns: JwtBuilder instance.
    public func setHeader(_ header: [String: Any]) -> JwtBuilder {
        h?.clear()
        if h == nil {
            h = Header()
        }
        h?.putAll(dic: header)
        return self
    }

    /// Add header for jwt.
    /// - Parameter header: The param of header.
    /// - Returns: JwtBuilder instance.
    public func addHeaders(header: [String: Any]) -> JwtBuilder {
        if h == nil {
            h = Header()
        }
        header.forEach { key, value in
            h?.headers[key] = value
        }
        return self
    }

    /// Add header for jwt.
    /// - Parameters:
    ///   - key: The key of header.
    ///   - value: The value of header.
    /// - Returns: JwtBuilder instance.
    public func addHeader(key: String, value: Any) -> JwtBuilder {
        if h == nil {
            h = Header()
        }
        h?.headers[key] = value
        return self
    }

    /// Set payload.
    /// - Parameter payload: The payload string.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func setPayload(payload: String) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        try c?.putAllWithJson(json: payload)
        return self
    }

    /// Create Claims for jwt.
    /// - Returns: Claims instance.
    public class func createClaims() -> Claims {
        return Claims()
    }

    /// Create Claims for jwt.
    /// - Parameter claims: The param of claims.
    /// - Returns: Claims instance.
    public class func createClaims(claims: [String: Any]) -> Claims {
        let cl = Claims()
        cl.putAll(dic: claims)
        return cl
    }

    /// Set Claims for jwt.
    /// - Parameter claim: The claim.
    /// - Returns: JwtBuilder instance.
    public func setClaims(_ claim: Claims) -> JwtBuilder {
        c = claim
        if !claim.containsKey(key: Claims.iss) {
            c = claim.setIssuer(issuer: issuer)
        }
        return self
    }

    /// Set Claims for jwt.
    /// - Parameter claim: The param of claims.
    /// - Returns: JwtBuilder instance.
    public func setClaims(claim: [String: Any]) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.clear()
        claim.forEach { key, value in
            c?.claims[key] = value
        }
        if !c!.containsKey(key: Claims.iss) {
           c = c!.setIssuer(issuer: issuer)
        }
        return self
    }

    /// Set Claims for jwt.
    /// - Parameter claims: The param of claims.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func setClaims(claims: JsonNode) throws -> JwtBuilder {
        let claimStr = claims.toString()

        return try setClaimsWithJson(value: claimStr)
    }

    /// Set claims with json for jwt.
    /// - Parameter value: The param of claims.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func setClaimsWithJson(value: String) throws -> JwtBuilder {
        let dic = try JSONSerialization.jsonObject(with: value.data(using: .utf8)!, options: []) as? [String : Any]
        guard dic != nil else {
            throw DIDError.illegalArgument("TODO")
        }
        return setClaims(claim: dic! )
    }

    /// Add claims for jwt.
    /// - Parameter claims: The param of claims.
    /// - Returns: JwtBuilder instance.
    public func addClaims(claims: [String: Any]) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.putAll(dic: claims)
        return self
    }

    /// Add claims for jwt.
    /// - Parameter claims: The claim
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func addClaims(claims: JsonNode) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        let claimStr = claims.toString()
        try c?.putAllWithJson(json: claimStr)
        return self
    }

    /// Add claims with json for jwt.
    /// - Parameter jsonClaims: The param of claims.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func addClaimsWithJson(jsonClaims: String) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        try c?.putAllWithJson(json: jsonClaims)

        return self
    }

    /// Set JWT issuer.
    /// - Parameter iss: The issuer value.
    /// - Returns: JwtBuilder instance.
    public func setIssuer(iss: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c = c?.setIssuer(issuer: iss)
        return self
    }

    /// Set JWT subject.
    /// - Parameter sub: The subject value.
    /// - Returns: JwtBuilder instance.
    public func setSubject(sub: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[Claims.sub] = sub
        return self
    }

    /// Set JWT audience.
    /// - Parameter audience: The audience value.
    /// - Returns: JwtBuilder instance.
    public func setAudience(audience: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[Claims.aud] = audience
        return self
    }

    /// Set expirate date.
    /// - Parameter expiration: The date of jwt expiration.
    /// - Returns: JwtBuilder instance.
    public func setExpiration(expiration: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c = c?.setExpiration(expiration: expiration)
        return self
    }

    /// Set JWT ‘nbf’ value.
    /// - Parameter nbf: The ‘nbf’ value.
    /// - Returns: JwtBuilder instance.
    public func setNotBefore(nbf: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c = c?.setNotBefore(notBefore: nbf)
        return self
    }

    /// Set JWT issued time.
    /// - Parameter issuedAt: The ‘iat’ value.
    /// - Returns: JwtBuilder instance.
    public func setIssuedAt(issuedAt: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c = c?.setIssuedAt(issuedAt: issuedAt)
        return self
    }

    /// Set JWT id.
    /// - Parameter id: The Id value.
    /// - Returns: JwtBuilder instance.
    public func setId(id: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[Claims.jti] = id
        return self
    }

    /// Add claim for jwt.
    /// - Parameters:
    ///   - name: The key for claim.
    ///   - value: The value for claim.
    /// - Returns: JwtBuilder instance.
    public func claim(name: String, value: Any) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.put(key: name, value: value)

        return self
    }

    /// Add claim for jwt.
    /// - Parameters:
    ///   - name: The key for claim.
    ///   - value: The value for claim.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func claim(name: String, value: JsonNode) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        let claimStr = value.toString()
        try c?.putWithJson(key: name, value: claimStr)

        return self
    }

    /// Add claim with json for jwt.
    /// - Parameters:
    ///   - name: The key for claim.
    ///   - jsonValue: The value for claim.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func claimWithJson(name: String, jsonValue: String) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        try c?.putWithJson(key: name, value: jsonValue)

        return self
    }

    /// Sign the jwtbuilder header and body.
    /// - Parameter password: Pass word to sign.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func sign(using password: String) throws -> JwtBuilder {
        jwt = JWT(header: self.h!, claims: self.c!)
        let privateKey = try self.privateKeyClosure!(nil, password)
        let jwtSigner = JWTSigner.es256(privateKey: privateKey)
        signature = try jwt!.sign(using: jwtSigner)

        return self
    }

    /// Sign the jwtbuilder header and body.
    /// - Parameters:
    ///   - withKey: The sign key.
    ///   - password: Pass word to sign.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func sign(withKey: String, using password: String) throws -> JwtBuilder {
        jwt = JWT(header: self.h!, claims: self.c!)
        let privateKey = try self.privateKeyClosure!(withKey, password)
        let jwtSigner = JWTSigner.es256(privateKey: privateKey)
        signature = try jwt!.sign(using: jwtSigner)

        return self
    }

    /// Get token from compacting JWTBuilder.
    /// - Throws: If error occurs, throw error.
    /// - Returns: Token string.
    public func compact() throws -> String {
        if jwt == nil {
            jwt = JWT(header: self.h!, claims: self.c!)
        }

        return try jwt!.compact(sign: signature)
    }
}

