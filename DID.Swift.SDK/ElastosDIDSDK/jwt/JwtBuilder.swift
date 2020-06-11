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

    public class func createHeader() -> Header {
        return Header()
    }

    public class func createHeader(header: [String: Any]) -> Header {
        let he = Header()
        he.putAll(dic: header)
        return he
    }

    public func setHeader(_ header: Header) -> JwtBuilder {
        h = header
        return self
    }

    public func setHeader(_ header: [String: Any]) -> JwtBuilder {
        h?.clear()
        if h == nil {
            h = Header()
        }
        h?.putAll(dic: header)
        return self
    }

    public func addHeaders(header: [String: Any]) -> JwtBuilder {
        if h == nil {
            h = Header()
        }
        header.forEach { key, value in
            h?.headers[key] = value
        }
        return self
    }

    public func addHeader(key: String, value: Any) -> JwtBuilder {
        if h == nil {
            h = Header()
        }
        h?.headers[key] = value
        return self
    }

    // TODO: check
    public func setPayload(payload: String) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        try c?.putAllWithJson(json: payload)
        return self
    }

    public class func createClaims() -> Claims {
        return Claims()
    }

    public class func createClaims(claims: [String: Any]) -> Claims {
        let cl = Claims()
        cl.putAll(dic: claims)
        return cl
    }

    public func setClaims(_ claim: Claims) -> JwtBuilder {
        c = claim
        if !claim.containsKey(key: Claims.iss) {
            c = claim.setIssuer(issuer: issuer)
        }
        return self
    }

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

    public func setClaims(claims: JsonNode) throws -> JwtBuilder {
        let claimStr = claims.toString()

        return try setClaimsWithJson(value: claimStr)
    }

    public func setClaimsWithJson(value: String) throws -> JwtBuilder {
        let dic = try JSONSerialization.jsonObject(with: value.data(using: .utf8)!, options: []) as? [String : Any]
        guard dic != nil else {
            throw DIDError.illegalArgument("TODO")
        }
        return setClaims(claim: dic! )
    }

    public func addClaims(claims: [String: Any]) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.putAll(dic: claims)
        return self
    }

    public func addClaims(claims: JsonNode) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        let claimStr = claims.toString()
        try c?.putAllWithJson(json: claimStr)
        return self
    }

    public func addClaimsWithJson(jsonClaims: String) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        try c?.putAllWithJson(json: jsonClaims)

        return self
    }

    public func setIssuer(iss: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c = c?.setIssuer(issuer: iss)
        return self
    }

    public func setSubject(sub: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[Claims.sub] = sub
        return self
    }

    public func setAudience(audience: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[Claims.aud] = audience
        return self
    }

    public func setExpiration(expiration: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c = c?.setExpiration(expiration: expiration)
        return self
    }

    public func setNotBefore(nbf: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c = c?.setNotBefore(notBefore: nbf)
        return self
    }

    public func setIssuedAt(issuedAt: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c = c?.setIssuedAt(issuedAt: issuedAt)
        return self
    }

    public func setId(id: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[Claims.jti] = id
        return self
    }

    public func claim(name: String, value: Any) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.put(key: name, value: value)

        return self
    }

    public func claim(name: String, value: JsonNode) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        let claimStr = value.toString()
        try c?.putWithJson(key: name, value: claimStr)

        return self
    }

    public func claimWithJson(name: String, jsonValue: String) throws -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        try c?.putWithJson(key: name, value: jsonValue)

        return self
    }

    public func sign(using password: String) throws -> JwtBuilder {
        jwt = JWT(header: self.h!, claims: self.c!)
        let privateKey = try self.privateKeyClosure!(nil, password)
        let jwtSigner = JWTSigner.es256(privateKey: privateKey)
        signature = try jwt!.sign(using: jwtSigner)

        return self
    }

    public func sign(withKey: String, using password: String) throws -> JwtBuilder {
        jwt = JWT(header: self.h!, claims: self.c!)
        let privateKey = try self.privateKeyClosure!(withKey, password)
        let jwtSigner = JWTSigner.es256(privateKey: privateKey)
        signature = try jwt!.sign(using: jwtSigner)

        return self
    }

    public func compact() throws -> String {
        if jwt == nil {
            jwt = JWT(header: self.h!, claims: self.c!)
        }

        // test verify
//        let publicKey = try self.publicKeyClosure!(nil)
//        let jwtVerify = JWTVerifier.es256(publicKey: publicKey)
//        let ok = JWT.verify(signedJWT, using: jwtVerify)
//        print(ok)
//        let j = JWT<Claims>(jwtString: signedJWT)
//        j.header.headers
        return try jwt!.compact(sign: signature)
    }
}

