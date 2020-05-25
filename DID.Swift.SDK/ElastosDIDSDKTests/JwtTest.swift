
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

            let h = JwtBuilder.createHeader()
            _ = h.setType("JWT")
                .setContentType("json")
                .setValue(key: "library", value: "Elastos DID")
                .setValue(key: "version", value: "1.0")

            let c = JwtBuilder.createClaims()
            _ = c.setSubject(subject: "JwtTest")
                .setId(id: "0")
                .setAudience(audience: "Test cases")
                .setExpiration(expiration: Date() + 1000)
                .setIssuedAt(issuedAt: Date())
                .setNotBefore(notBefore: Date())
                .setValue(key: "foo", value: "bar")

            let jwt = try doc.jwtBuilder()
                .setHeader(h)
                .setClaims(c)
            let token = try jwt.sign(using: storePass)
            print(token)

            let jp: JwtParser = try doc.jwtParserBuilder().build();
            let jwt1: JWT = try jp.parseClaimsJwt(token);
            print(jwt1.header)
            print(jwt1.claims)

        } catch {
            print(error)
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
