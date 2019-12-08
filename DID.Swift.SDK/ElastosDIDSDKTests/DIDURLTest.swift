
import XCTest
import ElastosDIDSDK

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
        XCTAssertEqual(params, url.getParameters())
    }

    func testGetParameter() {
        XCTAssertEqual("testvalue", url.getParameter("elastos:foo"))
        XCTAssertNotNil(url.getParameter("foo"))
        XCTAssertEqual("123", url.getParameter("bar"))
        XCTAssertEqual("12345", url.getParameter("elastos:foobar"))
        XCTAssertNotNil(url.getParameter("foobar"))
        XCTAssertNotNil(url.getParameter("keyonly"))
    }

    func testHasParameter() {
        XCTAssertTrue(url.hasParameter("elastos:foo"))
        XCTAssertTrue(url.hasParameter("bar"))
        XCTAssertTrue(url.hasParameter("elastos:foobar"))
        XCTAssertTrue(url.hasParameter("keyonly"))

        XCTAssertFalse(url.hasParameter("notexist"))
        XCTAssertFalse(url.hasParameter("foo"))
        XCTAssertFalse(url.hasParameter("boobar"))
    }

   func testGetPath() {
        XCTAssertEqual(path, url.path)
    }

    func testGetQuery() {
        XCTAssertEqual(query, url.getQuery())
    }

    func testGetQueryParameter() {
        XCTAssertEqual("qvalue", url.getQueryParameter("qkey"))
        XCTAssertEqual("true", url.getQueryParameter("test"))
        XCTAssertNil(url.getQueryParameter("qkeyonly"))
    }
    
    func testHasQueryParameter() {
        XCTAssertTrue(url.hasQueryParameter("qkeyonly"))
        XCTAssertTrue(url.hasQueryParameter("qkey"))
        XCTAssertTrue(url.hasQueryParameter("test"))

        XCTAssertFalse(url.hasQueryParameter("notexist"));
    }

    func testGetFragment() {
        XCTAssertEqual(fragment, url.fragment)
    }

    func testToExternalForm() {
        XCTAssertEqual(testURL, url.toExternalForm())
    }

    func testHashCode() {
        var other: DIDURL = try! DIDURL(testURL)
        XCTAssertEqual(url.hash, other.hash)

        other = try! DIDURL("did:elastos:1234567890#test")
        XCTAssertEqual(url.hash, other.hash)
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
