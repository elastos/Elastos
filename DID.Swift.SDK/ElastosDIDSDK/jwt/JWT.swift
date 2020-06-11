/**
 * Copyright IBM Corporation 2017
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

// MARK: JWT

/**
 
 A struct representing the `Header` and `Claims` of a JSON Web Token.
 
 ### Usage Example: ###
 ```swift
 struct MyClaims: Claims {
    var name: String
 }
 let jwt = JWT(claims: MyClaims(name: "Kitura"))
 let key = "<PrivateKey>".data(using: .utf8)!
 let signedJWT: String? = try? jwt.sign(using: .rs256(key: key, keyType: .privateKey))
 ```
 */

public struct JWT {
    
    /// The JWT header.
    public var header: Header
    
    /// The JWT claims
    public var claims: Claims

    /// The JWT signature
    public var signature: String?

    /// Initialize a `JWT` instance from a `Header` and `Claims`.
    ///
    /// - Parameter header: A JSON Web Token header object.
    /// - Parameter claims: A JSON Web Token claims object.
    /// - Returns: A new instance of `JWT`.
    public init(header: Header = Header(), claims: Claims) {
        self.header = header
        self.claims = claims
    }
    
    /// Initialize a `JWT` instance from a JWT String.
    /// The signature will be verified using the provided JWTVerifier.
    /// The time based standard JWT claims will be verified with `validateClaims()`.
    /// If the string is not a valid JWT, or the verification fails, the initializer returns nil.
    ///
    /// - Parameter jwt: A String with the encoded and signed JWT.
    /// - Parameter verifier: The `JWTVerifier` used to verify the JWT.
    /// - Returns: An instance of `JWT` if the decoding succeeds.
    /// - Throws: `JWTError.invalidJWTString` if the provided String is not in the form mandated by the JWT specification.
    /// - Throws: `JWTError.failedVerification` if the verifier fails to verify the jwtString.
    /// - Throws: A DecodingError if the JSONDecoder throws an error while decoding the JWT.
    public init(jwtString: String, verifier: JWTVerifier = .none ) throws {
        let components = jwtString.components(separatedBy: ".")
        guard components.count == 2 || components.count == 3,
            let headerData = JWTDecoder.data(base64urlEncoded: components[0]),
            let claimsData = JWTDecoder.data(base64urlEncoded: components[1])
        else {
            throw JWTError.invalidJWTString
        }
        if components.count == 3 {
            signature = components[2]
        }
        guard JWT.verify(jwtString, using: verifier) else {
            throw JWTError.failedVerification
        }
        let jsonDecoder = JSONDecoder()
        jsonDecoder.dateDecodingStrategy = .secondsSince1970
        let header = try Header.decode(headerData) // try jsonDecoder.decode(Header.self, from: headerData)
        let claims = try Claims.decode(claimsData) // try jsonDecoder.decode(T.self, from: claimsData)
        self.header = header
        self.claims = claims
    }
    
    /// Sign the JWT using the given algorithm and encode the header, claims and signature as a JWT String.
    ///
    /// - Note: This function will set header.alg field to the name of the signing algorithm.
    ///
    /// - Parameter using algorithm: The algorithm to sign with.
    /// - Returns: A String with the encoded and signed JWT.
    /// - Throws: An EncodingError if the JSONEncoder throws an error while encoding the JWT.
    /// - Throws: `JWTError.osVersionToLow` if not using macOS 10.12.0 (Sierra) or iOS 10.0 or higher.
    /// - Throws: A Signing error if the jwtSigner is unable to sign the JWT with the provided key.
    public mutating func sign(using jwtSigner: JWTSigner) throws -> String {
        let tempHeader = header
        tempHeader.headers[Header.alg] = jwtSigner.name
        let headerString = try tempHeader.encode()
        let claimsString = try claims.encode()
        header.headers[Header.alg] = tempHeader.headers[Header.alg]
        return try jwtSigner.sign(header: headerString, claims: claimsString)
    }

    public func compact(sign: String?) throws -> String {
        let tempHeader = header
        let headerString = try tempHeader.encode()
        let claimsString = try claims.encode()
        header.headers[Header.alg] = tempHeader.headers[Header.alg]

        return headerString + "." + claimsString + "." + (sign ?? "")
    }

    /// Verify the signature of the encoded JWT using the given algorithm.
    ///
    /// - Parameter jwt: A String with the encoded and signed JWT.
    /// - Parameter using algorithm: The algorithm to verify with.
    /// - Returns: A Bool indicating whether the verification was successful.
    public static func verify(_ jwt: String, using jwtVerifier: JWTVerifier) -> Bool {
        return jwtVerifier.verify(jwt: jwt)
    }

    /// Validate the time based standard JWT claims.
    /// This function checks that the "exp" (expiration time) is in the future
    /// and the "iat" (issued at) and "nbf" (not before) headers are in the past,
    ///
    /// - Parameter leeway: The time in seconds that the JWT can be invalid but still accepted to account for clock differences.
    /// - Returns: A value of `ValidateClaimsResult`.
    public func validateClaims(leeway: TimeInterval = 0) -> ValidateClaimsResult {        
        if let expirationDate = claims.claims[Claims.exp] {
                if expirationDate as! Date + leeway < Date() {
                    return .expired
                }
            }

            if let notBeforeDate = claims.claims[Claims.nbf] {
                if notBeforeDate as! Date > Date() + leeway {
                    return .notBefore
                }
            }

            if let issuedAtDate = claims.claims[Claims.iat] {
                if issuedAtDate as! Date > Date() + leeway {
                    return .issuedAt
                }
            }

            return .success
        }
}

