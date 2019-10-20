

import XCTest
import ElastosDIDSDK


class DIDDoucumentTests: XCTestCase {

    override func setUp() {
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }

    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    func testParseDocument() {
        let document: DIDDocument = try! DIDDocument.fromJson("/Users/liaihong/Desktop/testdiddoc.json")
        XCTAssertEqual(3, document.getPublicKeyCount())
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
        let document: DIDDocument = try! DIDDocument.fromJson("/Users/liaihong/Desktop/testdiddoc.json")
        let jsonString: String = try! document.toExternalForm(true)
        let url = URL(fileURLWithPath: "/Users/liaihong/Desktop/compact.json")
        let jsonStr = try! String(contentsOf: url)
        XCTAssertEqual(jsonString, jsonStr)
    }
    
    func  testNormalizedJson() {
        let document: DIDDocument = try! DIDDocument.fromJson("/Users/liaihong/Desktop/testdiddoc.json")
        let str: String = try! document.toExternalForm(false)
        let url = URL(fileURLWithPath: "/Users/liaihong/Desktop/normalized.json")
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
