
import XCTest
@testable import ElastosDIDSDK

let testDID: String = "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
let params: String = "elastos:foo=testvalue;bar=123;keyonly;elastos:foobar=12345"
let path: String = "/path/to/the/resource"
let query: String = "qkey=qvalue&qkeyonly&test=true"
let fragment: String = "testfragment"
let testURL: String = testDID + ";" + params + path + "?" + query + "#" + fragment

class DIDURLTest: XCTestCase {

    var url: DIDURL!
    
    override func setUp() {
        // Put setup code here. This method is called before the invocation of each test method in the class.
        url = try! DIDURL(testURL)
    }

    func testConstructor(){
        do {            
            var testURL: String = testDID
            var url: DIDURL = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + ";" + params
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + path
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + "?" + query
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + "#" + fragment
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + ";" + params + path
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + ";" + params + path + "?" + query
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + ";" + params + path + "?" + query + "#" + fragment
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID  + path + "?" + query + "#" + fragment
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + ";" + params + "?" + query + "#" + fragment
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + ";" + params + path + "#" + fragment
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
            
            testURL = testDID + ";" + params + path + "?" + query
            url = try DIDURL(testURL)
            XCTAssertEqual(testURL, url.description)
        } catch {
            print(error)
        }
    }
    
    func testGetDid() {
        XCTAssertEqual(testDID, url.did.description)
    }

   func testGetParameters() {
        XCTAssertEqual(params, url.parameters())
    }

    func testGetParameter() {
        XCTAssertEqual("testvalue", url.parameter(ofKey: "elastos:foo"))
        XCTAssertNil(url.parameter(ofKey: "foo"))
        XCTAssertEqual("123", url.parameter(ofKey: "bar"))
        XCTAssertEqual("12345", url.parameter(ofKey: "elastos:foobar"))
        XCTAssertNil(url.parameter(ofKey: "foobar"))
        let re = url.parameter(ofKey: "keyonly") == nil || url.parameter(ofKey: "keyonly") == ""
        XCTAssertTrue(re)
    }

    func testHasParameter() {
        XCTAssertTrue(url.containsParameter(forKey: "elastos:foo"))
        XCTAssertTrue(url.containsParameter(forKey: "bar"))
        XCTAssertTrue(url.containsParameter(forKey: "elastos:foobar"))
        XCTAssertTrue(url.containsParameter(forKey: "keyonly"))

        XCTAssertFalse(url.containsParameter(forKey: "notexist"))
        XCTAssertFalse(url.containsParameter(forKey: "foo"))
        XCTAssertFalse(url.containsParameter(forKey: "boobar"))
    }

   func testGetPath() {
        XCTAssertEqual(path, url.path)
    }

    func testGetQuery() {
        XCTAssertEqual(query, url.queryParameters())
    }

    func testGetQueryParameter() {
        XCTAssertEqual("qvalue", url.queryParameter(ofKey: "qkey"))
        XCTAssertEqual("true", url.queryParameter(ofKey: "test"))
        let re = url.queryParameter(ofKey: "qkeyonly") == nil || url.queryParameter(ofKey: "qkeyonly") == ""
        XCTAssertTrue(re)
    }
    
    func testHasQueryParameter() {
        XCTAssertTrue(url.containsQueryParameter(forKey: "qkeyonly"))
        XCTAssertTrue(url.containsQueryParameter(forKey: "qkey"))
        XCTAssertTrue(url.containsQueryParameter(forKey: "test"))

        XCTAssertFalse(url.containsQueryParameter(forKey: "notexist"));
    }

    func testGetFragment() {
        XCTAssertEqual(fragment, url.fragment)
    }

    func testToExternalForm() {
        XCTAssertEqual(testURL, url.description)
    }

    func testHashCode() {
        var other: DIDURL = try! DIDURL(testURL)
//        XCTAssertEqual(url.hash, other.hash) // TODO:

        other = try! DIDURL("did:elastos:1234567890#test")
//        XCTAssertNotEqual(url.hash, other.hash) // TODO:
    }

    func testEquals() {
        var other: DIDURL = try! DIDURL(testURL)
        XCTAssertTrue(url == other)
        XCTAssertTrue(url.description == testURL)

        other = try! DIDURL("did:elastos:1234567890#test")
        XCTAssertFalse(url == other)
        XCTAssertFalse(url.description == "did:elastos:1234567890#test")
    }
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }
}
