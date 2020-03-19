
import XCTest
@testable import ElastosDIDSDK

class DIDTest: XCTestCase {
    let testMethodSpecificID = "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
    let testDID = "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
    var did: DID!
    override func setUp() {
        // Put setup code here. This method is called before the invocation of each test method in the class.
        did = try! DID(testDID)
    }

    func testConstructor() {
        do {
            var did: DID = try DID(testDID)
            XCTAssertEqual(testDID, did.description)
            
            did = try DID("did:elastos:1234567890")
            XCTAssertEqual("did:elastos:1234567890", did.description)
        } catch  {
        }
    }

    func testConstructorError1() {
        do {
            let _ = try DID("id:elastos:1234567890")
        } catch {
            print(error)
        }
    }

    func testConstructorError2() {
        do {
            let _ = try DID("did:example:1234567890")
        } catch {
            print(error)
        }
    }

    func testConstructorError3() {
        do {
            let _ = try DID("did:elastos:")
        } catch {
            print(error)
        }
    }

    func testGetMethod()  {
        XCTAssertEqual(DID.METHOD, did.method)
    }

    func testGetMethodSpecificId() {
        XCTAssertEqual(testMethodSpecificID, did.methodSpecificId)
    }

    func testToExtermalForm() {
        XCTAssertEqual(testDID, did.description)
    }
    
    func testHashCode() {
        do {
            var other: DID = try DID(testDID)
//            XCTAssertEqual(did.hash, other.hash) // TODO:
            
            other = try DID("did:elastos:1234567890")
//            XCTAssertNotEqual(did.hash, other.hash) // TODO:
        } catch {
            
        }
    }

    func testEquals() {
        do {
            var other = try DID(testDID)
            XCTAssertTrue(did == other)
            XCTAssertTrue(did.description == testDID)

            other = try DID("did:elastos:1234567890")
            XCTAssertFalse(did == other)
            XCTAssertFalse(did.description == "did:elastos:1234567890")
        } catch {
        }
    }
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    
}
