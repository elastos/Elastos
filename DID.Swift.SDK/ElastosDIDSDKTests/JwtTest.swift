
import XCTest
import ElastosDIDSDK

class JwtTest: XCTestCase {

    func testJWT() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            var h = JwtBuilder.createHeader()
            _ = h.setType("JWT")
                .setContentType("json")
                .setValue(key: "library", value: "Elastos DID")
                .setValue(key: "version", value: "1.0")

            var c = JwtBuilder.createClaims()
            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10
            _ = c.setSubject(subject: "JwtTest")
                .setId(id: "0")
                .setAudience(audience: "Test cases")
                .setExpiration(expiration: exp)
                .setIssuedAt(issuedAt: iat!)
                .setNotBefore(notBefore: nbf)
                .setValue(key: "foo", value: "bar")

            let token = try doc.jwtBuilder()
                .setHeader(h)
                .setClaims(c)
                .compact()
            print(token)

            let jp: JwtParser = doc.jwtParserBuilder().build()
            let jwt: JWT = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)
            h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual("JWT", h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as! String)

            c = jwt.claims

            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as! String)

        } catch {
            print(error)
            XCTFail()
        }
    }

    func testSignWithDefaultKey() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            var h = JwtBuilder.createHeader()
            h = h.setType(Header.JWT_TYPE)
                .setContentType("json")
                .setValue(key: "library", value: "Elastos DID")
                .setValue(key: "version", value: "1.0")
            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10

            var c = JwtBuilder.createClaims()
            c = c.setSubject(subject: "JwtTest")
                .setId(id: "0")
                .setAudience(audience: "Test cases")
                .setIssuedAt(issuedAt: iat!)
                .setExpiration(expiration: exp)
                .setNotBefore(notBefore: nbf)
                .setValue(key: "foo", value: "bar")

            let token = try doc.jwtBuilder()
                            .setHeader(h)
                            .setClaims(c)
                            .sign(using: storePass)
                            .compact()
            XCTAssertNotNil(token)

            let jp = doc.jwtParserBuilder().build()
            let jwt = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)

            h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual(Header.JWT_TYPE, h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as? String)
            XCTAssertEqual("1.0", h.getValue(key: "version") as? String)

            c = jwt.claims
            XCTAssertNotNil(c)
            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as! String)

            let s = jwt.signature
            XCTAssertNotNil(s)
        } catch {
            XCTFail()
        }
    }

    func testSignWithSpecificKey() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10

            let token = try doc.jwtBuilder()
                .appendHeader(key: Header.TYPE, value: Header.JWT_TYPE)
                .appendHeader(key: Header.CONTENT_TYPE, value: "json")
                .appendHeader(key: "library", value: "Elastos DID")
                .appendHeader(key: "version", value: "1.0")
                .setSubject(sub: "JwtTest")
                .setId(id: "0")
                .setAudience(audience: "Test cases")
                .setIssuedAt(issuedAt: iat!)
                .setExpiration(expiration: exp)
                .setNotBefore(nbf: nbf)
                .claims(key: "foo", value: "bar")
                .sign(withKey: "#key2", using: storePass)
                .compact()
            XCTAssertNotNil(token)

            let jp = doc.jwtParserBuilder().build()
            let jwt = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)

            let h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual(Header.JWT_TYPE, h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as? String)
            XCTAssertEqual("1.0", h.getValue(key: "version") as? String)

            let c = jwt.claims
            XCTAssertNotNil(c)
            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as? String)
            let s = jwt.signature
            XCTAssertNotNil(s)
        } catch {
            XCTFail()
        }
    }

    func testAutoVerify() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10

            let token = try doc.jwtBuilder()
                    .appendHeader(key: Header.TYPE, value: Header.JWT_TYPE)
                    .appendHeader(key: Header.CONTENT_TYPE, value: "json")
                    .appendHeader(key: "library", value: "Elastos DID")
                    .appendHeader(key: "version", value: "1.0")
                    .setSubject(sub: "JwtTest")
                    .setId(id: "0")
                    .setAudience(audience: "Test cases")
                    .setIssuedAt(issuedAt: iat!)
                    .setExpiration(expiration: exp)
                    .setNotBefore(nbf: nbf)
                    .claims(key: "foo", value: "bar")
                    .sign(withKey: "#key2", using: storePass)
                    .compact()

            XCTAssertNotNil(token)

            // The JWT parser not related with a DID document
            let jp = JwtParserBuilder().build()
            let jwt = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)

            let h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual(Header.JWT_TYPE, h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as? String)
            XCTAssertEqual("1.0", h.getValue(key: "version") as? String)

            let c = jwt.claims
            XCTAssertNotNil(c)
            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as? String)
            let s = jwt.signature
            XCTAssertNotNil(s)
        } catch {
            XCTFail()
        }
    }

    func testClaimJsonNode() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10
            let node = try testData.loadEmailVcNormalizedJson()
            let dic: [String: Any]? = try JSONSerialization.jsonObject(with: node.data(using: .utf8)!, options: []) as? [String: Any]

            let token = try doc.jwtBuilder()
                .appendHeader(key: Header.TYPE, value: Header.JWT_TYPE)
                .appendHeader(key: Header.CONTENT_TYPE, value: "json")
                .appendHeader(key: "library", value: "Elastos DID")
                .appendHeader(key: "version", value: "1.0")
                .setSubject(sub: "JwtTest")
                .setId(id: "0")
                .setAudience(audience: "Test cases")
                .setIssuedAt(issuedAt: iat!)
                .setExpiration(expiration: exp)
                .setNotBefore(nbf: nbf)
                .claims(key: "foo", value: "bar")
                .claims(key: "vc", value: dic as Any)
                .sign(withKey: "#key2", using: storePass)
                .compact()
            XCTAssertNotNil(token)

            // The JWT parser not related with a DID document
            let jp = JwtParserBuilder().build()
            let jwt = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)

            let h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual(Header.JWT_TYPE, h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as? String)
            XCTAssertEqual("1.0", h.getValue(key: "version") as? String)

            let c = jwt.claims
            XCTAssertNotNil(c)
            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as? String)
            let s = jwt.signature
            XCTAssertNotNil(s)
            let d = c.getValue(key: "vc") as? [String: Any]
            XCTAssertNotNil(d)
            XCTAssertEqual(try testData.loadEmailCredential().getId().description, d!["id"] as? String)
        } catch {
            XCTFail()
        }
    }

    func testClaimJsonText() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10

            let jsonValue = try testData.loadEmailVcNormalizedJson()
            let token = try doc.jwtBuilder()
                    .appendHeader(key: Header.TYPE, value: Header.JWT_TYPE)
                    .appendHeader(key: Header.CONTENT_TYPE, value: "json")
                    .appendHeader(key: "library", value: "Elastos DID")
                    .appendHeader(key: "version", value: "1.0")
                    .setSubject(sub: "JwtTest")
                    .setId(id: "0")
                    .setAudience(audience: "Test cases")
                    .setIssuedAt(issuedAt: iat!)
                    .setExpiration(expiration: exp)
                    .setNotBefore(nbf: nbf)
                    .claims(key: "foo", value: "bar")
                    .claimWithJson(key: "vc", value: jsonValue)
                    .sign(withKey: "#key2", using: storePass)
                    .compact()
            XCTAssertNotNil(token)

            let jp = JwtParserBuilder().build()
            let jwt = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)

            let h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual(Header.JWT_TYPE, h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as? String)
            XCTAssertEqual("1.0", h.getValue(key: "version") as? String)

            let c = jwt.claims
            XCTAssertNotNil(c)
            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as? String)
            let s = jwt.signature
            XCTAssertNotNil(s)
            let d = c.getValue(key: "vc") as? [String: Any]
            XCTAssertNotNil(d)
            XCTAssertEqual(try testData.loadEmailCredential().getId().description, d!["id"] as? String)
        } catch {
            XCTFail()
        }
    }

    func testSignSetClaimWithJsonNode() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10

            let json = "{\n" +
                    "  \"sub\":\"JwtTest\",\n" +
                    "  \"jti\":\"0\",\n" +
                    "  \"aud\":\"Test cases\",\n" +
                    "  \"foo\":\"bar\",\n" +
                    "  \"object\":{\n" +
                    "    \"hello\":\"world\",\n" +
                    "    \"test\":true\n" +
                    "  }\n" +
                    "}"
            let dic: [String: Any]? = try JSONSerialization.jsonObject(with: json.data(using: .utf8)!, options: []) as? [String: Any]
            let token = try doc.jwtBuilder()
                    .appendHeader(key: Header.TYPE, value: Header.JWT_TYPE)
                    .appendHeader(key: Header.CONTENT_TYPE, value: "json")
                    .appendHeader(key: "library", value: "Elastos DID")
                    .appendHeader(key: "version", value: "1.0")
                    .claim(claim: dic!)
                    .setIssuedAt(issuedAt: iat!)
                    .setExpiration(expiration: exp)
                    .setNotBefore(nbf: nbf)
                    .sign(withKey: "#key2", using: storePass)
                    .compact()
            XCTAssertNotNil(token)

            // The JWT parser not related with a DID document
            let jp = JwtParserBuilder().build()
            let jwt = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)

            let h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual(Header.JWT_TYPE, h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as? String)
            XCTAssertEqual("1.0", h.getValue(key: "version") as? String)

            let c = jwt.claims
            XCTAssertNotNil(c)
            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as? String)
            let s = jwt.signature
            XCTAssertNotNil(s)
        } catch {
            XCTFail()
        }
    }

    func testSetClaimWithJsonText() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10

            let json = "{\n" +
                    "  \"sub\":\"JwtTest\",\n" +
                    "  \"jti\":\"0\",\n" +
                    "  \"aud\":\"Test cases\",\n" +
                    "  \"foo\":\"bar\",\n" +
                    "  \"object\":{\n" +
                    "    \"hello\":\"world\",\n" +
                    "    \"test\":true\n" +
                    "  }\n" +
                    "}"
            let token = try doc.jwtBuilder()
                    .appendHeader(key: Header.TYPE, value: Header.JWT_TYPE)
                    .appendHeader(key: Header.CONTENT_TYPE, value: "json")
                    .appendHeader(key: "library", value: "Elastos DID")
                    .appendHeader(key: "version", value: "1.0")
                    .claimWithJson(value: json)
                    .setIssuedAt(issuedAt: iat!)
                    .setExpiration(expiration: exp)
                    .setNotBefore(nbf: nbf)
                    .sign(withKey: "#key2", using: storePass)
                    .compact()
            XCTAssertNotNil(token)

            // The JWT parser not related with a DID document
            let jp = JwtParserBuilder().build()
            let jwt = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)

            let h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual(Header.JWT_TYPE, h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as? String)
            XCTAssertEqual("1.0", h.getValue(key: "version") as? String)

            let c = jwt.claims
            XCTAssertNotNil(c)
            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as? String)
            let s = jwt.signature
            XCTAssertNotNil(s)

        } catch {
            XCTFail()
        }
    }

    func testAddClaimWithJsonNode() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10
            let json = "{\n" +
                    "  \"sub\":\"JwtTest\",\n" +
                    "  \"jti\":\"0\",\n" +
                    "  \"aud\":\"Test cases\",\n" +
                    "  \"foo\":\"bar\",\n" +
                    "  \"object\":{\n" +
                    "    \"hello\":\"world\",\n" +
                    "    \"test\":true\n" +
                    "  }\n" +
                    "}"
            let dic: [String: Any]? = try JSONSerialization.jsonObject(with: json.data(using: .utf8)!, options: []) as? [String: Any]
            let token = try doc.jwtBuilder()
                    .appendHeader(key: Header.TYPE, value: Header.JWT_TYPE)
                    .appendHeader(key: Header.CONTENT_TYPE, value: "json")
                    .appendHeader(key: "library", value: "Elastos DID")
                    .appendHeader(key: "version", value: "1.0")
                    .claim(claim: dic!)
                    .setIssuedAt(issuedAt: iat!)
                    .setExpiration(expiration: exp)
                    .setNotBefore(nbf: nbf)
                    .sign(withKey: "#key2", using: storePass)
                    .compact()
            XCTAssertNotNil(token)

            // The JWT parser not related with a DID document
            let jp = JwtParserBuilder().build()
            let jwt = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)

            let h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual(Header.JWT_TYPE, h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as? String)
            XCTAssertEqual("1.0", h.getValue(key: "version") as? String)

            let c = jwt.claims
            XCTAssertNotNil(c)
            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as? String)
            let s = jwt.signature
            XCTAssertNotNil(s)
        } catch {
            XCTFail()
        }
    }

    func testAddClaimWithJsonText() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let doc = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid)

            let userCalendar = Calendar.current
            var components = DateComponents()
            components.year = 2020
            components.month = 5
            components.day = 27
            components.minute = 21
            components.hour = 21
            components.second = 41
            let iat = userCalendar.date(from: components)

            let exp = iat! + 1000
            let nbf = iat! - 10
            let json = "{\n" +
                    "  \"sub\":\"JwtTest\",\n" +
                    "  \"jti\":\"0\",\n" +
                    "  \"aud\":\"Test cases\",\n" +
                    "  \"foo\":\"bar\",\n" +
                    "  \"object\":{\n" +
                    "    \"hello\":\"world\",\n" +
                    "    \"test\":true\n" +
                    "  }\n" +
                    "}"
            let token = try doc.jwtBuilder()
                    .appendHeader(key: Header.TYPE, value: Header.JWT_TYPE)
                    .appendHeader(key: Header.CONTENT_TYPE, value: "json")
                    .appendHeader(key: "library", value: "Elastos DID")
                    .appendHeader(key: "version", value: "1.0")
                    .claimWithJson(value: json)
                    .setIssuedAt(issuedAt: iat!)
                    .setExpiration(expiration: exp)
                    .setNotBefore(nbf: nbf)
                    .sign(withKey: "#key2", using: storePass)
                    .compact()
            XCTAssertNotNil(token)

            // The JWT parser not related with a DID document
            let jp = JwtParserBuilder().build()
            let jwt = try jp.parseClaimsJwt(token)
            XCTAssertNotNil(jwt)

            let h = jwt.header
            XCTAssertNotNil(h)
            XCTAssertEqual("json", h.getContentType())
            XCTAssertEqual(Header.JWT_TYPE, h.getType())
            XCTAssertEqual("Elastos DID", h.getValue(key: "library") as? String)
            XCTAssertEqual("1.0", h.getValue(key: "version") as? String)

            let c = jwt.claims
            XCTAssertNotNil(c)
            XCTAssertEqual("JwtTest", c.getSubject())
            XCTAssertEqual("0", c.getId())
            XCTAssertEqual(doc.subject.description, c.getIssuer())
            XCTAssertEqual("Test cases", c.getAudience())
            XCTAssertEqual(iat, c.getIssuedAt())
            XCTAssertEqual(exp, c.getExpiration())
            XCTAssertEqual(nbf, c.getNotBefore())
            XCTAssertEqual("bar", c.getValue(key: "foo") as? String)
            let s = jwt.signature
            XCTAssertNotNil(s)
        } catch {
            XCTFail()
        }
    }

    override func setUpWithError() throws {
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }

    override func tearDownWithError() throws {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    func testExample() throws {
        // This is an example of a functional test case.
        // Use XCTAssert and related functions to verify your tests produce the correct results.
    }

    func testPerformanceExample() throws {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }

}
