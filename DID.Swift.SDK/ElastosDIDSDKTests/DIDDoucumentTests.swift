

import XCTest
import ElastosDIDSDK


class DIDDoucumentTests: XCTestCase {
    
    var compactPath: String!
    var documentPath: String!
    var normalizedPath: String!

    override func setUp() {
        super.setUp()
        
        let bundle = Bundle(for: type(of: self))
        compactPath = bundle.path(forResource: "compact", ofType: "json")!
        documentPath = bundle.path(forResource: "testdiddoc", ofType: "json")!
        normalizedPath = bundle.path(forResource: "normalized", ofType: "json")!
    }

    override func tearDown() {
        compactPath = nil
        documentPath = nil
        normalizedPath = nil
        super.tearDown()
    }

    func testParseDocument() {
        let document: DIDDocument = try! DIDDocument.fromJson(documentPath)
        XCTAssertEqual(4, document.getPublicKeyCount())
        let pks: Array<DIDPublicKey> = document.getPublicKeys()
        pks.forEach { pk in
            let result: Bool = pk.id.fragment == "default" || pk.id.fragment == "key2" || pk.id.fragment == "keys3" || pk.id.fragment == "recovery"
            XCTAssert(result)
            print(pk.id.fragment as Any)
            if pk.id.fragment == "recovery"{
                XCTAssertNotEqual(document.subject, pk.controller)
            }
            else {
                XCTAssertEqual(document.subject, pk.controller)
            }
        }
        XCTAssertEqual(3, document.getAuthenticationKeyCount())
        XCTAssertEqual(1, document.getAuthorizationKeyCount())
        XCTAssertEqual(2, document.getCredentialCount())
        XCTAssertEqual(3, document.getServiceCount())
    }
    
    func testCompactJson() {

        let document: DIDDocument = try! DIDDocument.fromJson(documentPath)
        let jsonString: String = try! document.toExternalForm(true)
        let url = URL(fileURLWithPath:compactPath)
        let jsonStr = try! String(contentsOf: url)
        XCTAssertEqual(jsonString, jsonStr)
    }
    
    func  testNormalizedJson() {
        
        let document: DIDDocument = try! DIDDocument.fromJson(documentPath)
        let str: String = try! document.toExternalForm(false)
        let url = URL(fileURLWithPath: normalizedPath)
        let jsonStr = try! String(contentsOf: url)
        XCTAssertEqual(str, jsonStr)
    }

    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }

}
