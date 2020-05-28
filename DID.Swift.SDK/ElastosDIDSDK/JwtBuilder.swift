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

    public class func createClaims() -> Claims {
        return Claims()
    }

    public func setHeader(_ header: Header) -> JwtBuilder {
        h = header
        return self
    }

    public func setType(_ type: String) -> JwtBuilder {
        if h == nil {
            h = Header()
        }
        h?.headers[Header.TYPE] = type
        return self
    }

    public func setContentType(_ cty: String) -> JwtBuilder {
        if h == nil {
            h = Header()
        }
        h?.headers[Header.CONTENT_TYPE] = cty
        return self
    }

    public func setClaims(_ claim: Claims) -> JwtBuilder {
        c = claim
        if !claim.containsKey(key: Claims.iss) {
          c = claim.setIssuer(issuer: issuer)
        }
        return self
    }

    public func appendHeader(header: [String: Any]) -> JwtBuilder {
        if h == nil {
            h = Header()
        }
        header.forEach { key, value in
            h?.headers[key] = value
        }
        return self
    }

    public func appendHeader(key: String, value: Any) -> JwtBuilder {
        if h == nil {
            h = Header()
        }
        h?.headers[key] = value
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

    public func setId(id: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[Claims.jti] = id
        return self
    }

    public func setAudience(audience: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[Claims.aud] = audience
        return self
    }

    public func setIssuedAt(issuedAt: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c = c?.setIssuedAt(issuedAt: issuedAt)
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

    public func claims(key: String, value: Any) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[key] = value
        return self
    }

    public func claimWithJson(key: String, value: String) throws -> JwtBuilder {
        let dic = try JSONSerialization.jsonObject(with: value.data(using: .utf8)!, options: [])
        return claims(key: key, value: dic)
    }

    public func claimWithJson(value: String) throws -> JwtBuilder {
        let dic = try JSONSerialization.jsonObject(with: value.data(using: .utf8)!, options: []) as? [String : Any]
        guard dic != nil else {
            throw DIDError.illegalArgument("TODO")
        }
        return claim(claim: dic! )
    }

    public func claim(claim: [String: Any]) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        claim.forEach { key, value in
            c?.claims[key] = value
        }
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

// test
    func read(fileName: String) -> Data {
        do {
            var pathToTests = #file
            if pathToTests.hasSuffix("JwtBuilder.swift") {
                pathToTests = pathToTests.replacingOccurrences(of: "JwtBuilder.swift", with: "")
            }
            let fileData = try Data(contentsOf: URL(fileURLWithPath: "\(pathToTests)\(fileName)"))
            return fileData
        } catch {
            exit(1)
        }
    }
}

