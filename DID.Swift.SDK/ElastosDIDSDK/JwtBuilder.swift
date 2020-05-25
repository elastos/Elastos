import Foundation

public class JwtBuilder {
    var h: Header?
    var c: Claims?
    var publicKeyClosure: ((_ id: String?) throws -> Data)?
    var privateKeyClosure: ((_ id: String?, _ storepass: String) throws -> Data)?

    init(publicKey: @escaping (_ id: String?) throws -> Data, privateKey: @escaping (_ id: String?, _ storepass: String) throws -> Data) {
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
        h?.headers[h!.typ] = type
        return self
    }

    public func setContentType(_ cty: String) -> JwtBuilder {
        if h == nil {
            h = Header()
        }
        h?.headers[h!.cty] = cty
        return self
    }

    public func setClaims(_ claim: Claims) -> JwtBuilder {
        c = claim
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

    public func appdendSubject(sub: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[c!.sub] = sub
        return self
    }

    public func appdendId(id: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[c!.jti] = id
        return self
    }

    public func appdendAudience(audience: String) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[c!.aud] = audience
        return self
    }

    public func appdendIssuedAt(issuedAt: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[c!.iat] = issuedAt
        return self
    }

    public func appdendExpiration(expiration: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[c!.exp] = expiration
        return self
    }

    public func appdendNotBefore(nbf: Date) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[c!.nbf] = nbf
        return self
    }

    public func claims(key: String, value: Any) -> JwtBuilder {
        if c == nil {
            c = Claims()
        }
        c?.claims[key] = value
        return self
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

    public func sign(using password: String) throws -> String {
        var jwt = JWT(header: self.h!, claims: self.c!)
        let privateKey = try self.privateKeyClosure!(nil, password)
        let publicKey = try self.publicKeyClosure!(nil)
        let jwtSigner = JWTSigner.es256(privateKey: privateKey)
        let jwtVerify = JWTVerifier.es256(publicKey: publicKey)

        let signedJWT = try jwt.sign(using: jwtSigner)

        // test
        let ok = JWT.verify(signedJWT, using: jwtVerify)
        print(ok)
//        // test
//        let j = JWT<Claims>(jwtString: signedJWT)
//        j.header.headers
        return signedJWT
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

