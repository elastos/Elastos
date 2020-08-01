/*
* Copyright (c) 2019 Elastos Foundation
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

import UIKit

public class JwtParserBuilder: NSObject {

    var getPublicKey : ((_ id: String?) throws -> Data)?

    var getPrivateKey : ((_ id: String?, _ storepass: String?) throws -> Data)?

    var claimsJwt : String?

    /// Parse jwt token.
    /// - Parameter claimsJwt: Jwt token.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: The handle of JWT.
    public func parseClaimsJwt(_ claimsJwt: String) throws -> JWT {
        return try JWT(jwtString: claimsJwt)
    }

    /// Create JwtParser
    /// - Returns: JwtParser instance.
    public func build() -> JwtParser {
        return JwtParser()
    }
}

public class JwtParser: NSObject {

    var jwt: JWT?

    /// Parse jwt token.
    /// - Parameter claimsJwt: Jwt token.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: The handle of JWT.
    public func parseClaimsJwt(_ claimsJwt: String) throws -> JWT {
        return try JWT(jwtString: claimsJwt)
    }

    /// Get jwt header.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: Jwt Header.
    public func getHeader() throws -> Header {
        return jwt!.header;
    }

    /// Get jwt claims.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: Jwt Claims.
    public func getBody() throws -> Claims {
        return jwt!.claims;
    }
}
