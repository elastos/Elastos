
import XCTest
@testable import ElastosDIDSDK

class VerifiableCredentialTest: XCTestCase {
    
    func TestKycCredential() {
        do {
            let testData = TestData()
            
            // for integrity check
            _ = try testData.setupStore(true)
            let issuer:DIDDocument = try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            let vc: VerifiableCredential = try testData.loadEmailCredential()
            
            XCTAssertEqual(try DIDURL(test.subject, "email"), vc.getId())
            
            XCTAssertTrue((vc.getTypes()).contains("BasicProfileCredential"))
            XCTAssertTrue((vc.getTypes()).contains("InternetAccountCredential"))
            XCTAssertTrue((vc.getTypes()).contains("EmailCredential"))
            
            XCTAssertEqual(issuer.subject, vc.issuer)
            XCTAssertEqual(test.subject, vc.subject.did)
            XCTAssertEqual("john@example.com", vc.subject.getPropertyAsString(ofName: "email"))
            
            XCTAssertNotNil(vc.issuanceDate)
            XCTAssertNotNil(vc.expirationDate)
            
            XCTAssertFalse(vc.isExpired)
            XCTAssertTrue(vc.isGenuine)
            XCTAssertTrue(vc.isValid)
        }
        catch {
            XCTFail()
        }
    }
    
    func TestSelfProclaimedCredential() {
        do{
            let testData: TestData = TestData()
            
            // for integrity check
            _ = try testData.setupStore(true)
            let test: DIDDocument = try testData.loadTestDocument()
            
            let vc = try testData.loadProfileCredential()
            
            XCTAssertEqual(try DIDURL(test.subject, "profile"), vc!.getId())
            XCTAssertTrue((vc!.getTypes()).contains("BasicProfileCredential"))
            XCTAssertTrue((vc!.getTypes()).contains("SelfProclaimedCredential"))
            
            XCTAssertEqual(test.subject, vc!.issuer)
            XCTAssertEqual(test.subject, vc!.subject.did)
            
            XCTAssertEqual("John", vc!.subject.getPropertyAsString(ofName: "name"))
            XCTAssertEqual("Male", vc!.subject.getPropertyAsString(ofName: "gender"))
            XCTAssertEqual("Singapore", vc!.subject.getPropertyAsString(ofName: "nation"))
            XCTAssertEqual("English", vc!.subject.getPropertyAsString(ofName: "language"))
            XCTAssertEqual("john@example.com", vc!.subject.getPropertyAsString(ofName: "email"))
            XCTAssertEqual("@john", vc!.subject.getPropertyAsString(ofName: "twitter"))
            XCTAssertNotNil(vc!.issuanceDate)
            XCTAssertNotNil(vc!.expirationDate)
            
            XCTAssertFalse(vc!.isExpired)
            XCTAssertTrue(vc!.isGenuine)
            XCTAssertTrue(vc!.isValid)
        }
        catch {
            XCTFail()
        }
    }
    
    func testParseAndSerializeKycCredential() {
        do{
            let testData: TestData = TestData()
            
            var json: String = try testData.loadTwitterVcNormalizedJson()
            let normalized: VerifiableCredential = try VerifiableCredential.fromJson(json)
            
            json = try testData.loadTwitterVcCompactJson()
            let compact: VerifiableCredential = try VerifiableCredential.fromJson(json)
            
            let vc: VerifiableCredential = try testData.loadTwitterCredential()
            
            XCTAssertEqual(try testData.loadTwitterVcNormalizedJson(), normalized.toString(true))
            XCTAssertEqual(try testData.loadTwitterVcNormalizedJson(), compact.toString(true))
            XCTAssertEqual(try testData.loadTwitterVcNormalizedJson(), vc.toString(true))
            
            XCTAssertEqual(try testData.loadTwitterVcCompactJson(), normalized.toString(false))
            XCTAssertEqual(try testData.loadTwitterVcCompactJson(), compact.toString(false))
            XCTAssertEqual(try testData.loadTwitterVcCompactJson(), vc.toString(false))
        }
        catch {
         print(error)
            XCTFail()
        }
    }
    
    func testParseAndSerializeSelfProclaimedCredential() {
        do {
            let testData: TestData = TestData()
            
            var json: String = try testData.loadProfileVcNormalizedJson()
            let normalized: VerifiableCredential = try VerifiableCredential.fromJson(json)
            
            json = try testData.loadProfileVcCompactJson()
            let compact: VerifiableCredential = try VerifiableCredential.fromJson(json)
            
            let vc = try testData.loadProfileCredential()
            
            XCTAssertEqual(try testData.loadProfileVcNormalizedJson(), normalized.toString(true))
            XCTAssertEqual(try testData.loadProfileVcNormalizedJson(), compact.toString(true))
            XCTAssertEqual(try testData.loadProfileVcNormalizedJson(), vc!.toString(true))
            
            XCTAssertEqual(try testData.loadProfileVcCompactJson(), normalized.toString(false))
            XCTAssertEqual(try testData.loadProfileVcCompactJson(), compact.toString(false))
            XCTAssertEqual(try testData.loadProfileVcCompactJson(), vc!.toString(false))
        }
        catch {
            XCTFail()
        }
    }
    
    func testParseAndSerializeJsonCredential() {
        do {
            let testData = TestData()
            var json = try testData.loadJsonVcNormalizedJson()
            let normalized = try VerifiableCredential.fromJson(json)
            json = try testData.loadJsonVcCompactJson()
            let compact = try VerifiableCredential.fromJson(json)
            let vc = try testData.loadJsonCredential()
            XCTAssertEqual(try testData.loadJsonVcNormalizedJson(), normalized.toString(true))
            XCTAssertEqual(try testData.loadJsonVcNormalizedJson(), compact.toString(true))
            XCTAssertEqual(try testData.loadJsonVcNormalizedJson(), vc.toString(true))

            XCTAssertEqual(try testData.loadJsonVcCompactJson(), normalized.toString(false))
            XCTAssertEqual(try testData.loadJsonVcCompactJson(), compact.toString(false))
            XCTAssertEqual(try testData.loadJsonVcCompactJson(), vc.toString(false))
        } catch {
            XCTFail()
        }
    }
}
