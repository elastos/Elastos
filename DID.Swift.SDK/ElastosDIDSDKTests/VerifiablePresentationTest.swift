
import XCTest
@testable import ElastosDIDSDK

class VerifiablePresentationTest: XCTestCase {
    
    func testReadPresentation() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            
            // For integrity check
            _ = try testData.loadTestIssuer()
            let testDoc: DIDDocument = try testData.loadTestDocument()
            
            let vp: VerifiablePresentation = try testData.loadPresentation()
            
            XCTAssertEqual("VerifiablePresentation", vp.type)
            XCTAssertEqual(testDoc.subject, vp.signer)
            XCTAssertEqual(4, vp.cedentialCount)
            
            let vcs: Array<VerifiableCredential> = vp.credentials
            for vc in vcs {
                XCTAssertEqual(testDoc.subject, vc.subject.did)
                let re = vc.getId().fragment == "profile" || vc.getId().fragment == "email" || vc.getId().fragment == "twitter" || vc.getId().fragment == "passport"
                XCTAssertTrue(re)
            }
            XCTAssertNotNil(try vp.credential(ofId: DIDURL(vp.signer, "profile")))
            XCTAssertNotNil(try vp.credential(ofId:DIDURL(vp.signer, "email")))
            XCTAssertNotNil(try vp.credential(ofId:DIDURL(vp.signer, "twitter")))
            XCTAssertNotNil(try vp.credential(ofId:DIDURL(vp.signer, "passport")))
            XCTAssertNil(try vp.credential(ofId:DIDURL(vp.signer, "notExist")))
            
            XCTAssertTrue(vp.isGenuine)
            XCTAssertTrue(vp.isValid)
            
        }
        catch {
            XCTFail()
        }
    }
    
    func testBuild() {
        do {
            let testData = TestData()
            let store = try! testData.setupStore(true)
            // For integrity check
            _ = try! testData.loadTestIssuer()
            let testDoc = try! testData.loadTestDocument()
            
            let pb = try! VerifiablePresentation.editingVerifiablePresentation(for: testDoc.subject, using: store)
            let vp = try! pb.withCredentials(testData.loadProfileCredential()!,
                                            testData.loadEmailCredential(),
                                            testData.loadTwitterCredential(),
                                            testData.loadPassportCredential()!)
                .withRealm("https://example.com/")
                .withNonce("873172f58701a9ee686f0630204fee59")
                .sealed(using: storePass)
            
            XCTAssertNotNil(vp)
            XCTAssertEqual("VerifiablePresentation", vp.type)
            XCTAssertEqual(testDoc.subject, vp.signer)

            XCTAssertEqual(4, vp.cedentialCount)
            let vcs = vp.credentials
            
            vcs.forEach { vc in
                let re = vc.getId().fragment == "profile" || vc.getId().fragment == "email" || vc.getId().fragment == "twitter" || vc.getId().fragment == "passport"
                XCTAssertTrue(re)
            }
            
            XCTAssertNotNil(try! vp.credential(ofId: DIDURL(vp.signer, "profile")))
            XCTAssertNotNil(try! vp.credential(ofId: DIDURL(vp.signer, "email")))
            XCTAssertNotNil(try! vp.credential(ofId: DIDURL(vp.signer, "twitter")))
            XCTAssertNotNil(try! vp.credential(ofId: DIDURL(vp.signer, "passport")))
            let re = try vp.credential(ofId: DIDURL(vp.signer, "notExist"))
            XCTAssertNil(re)

            XCTAssertTrue(vp.isGenuine)
            XCTAssertTrue(vp.isValid)
        }
        catch {
            XCTFail()
        }
    }
    
    func testParseAndSerialize() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            
            // For integrity check
            _ = try testData.loadTestIssuer()
            _ = try testData.loadTestDocument()
            let vp: VerifiablePresentation = try testData.loadPresentation()
            XCTAssertNotNil(vp)
            XCTAssertTrue(vp.isGenuine)
            XCTAssertTrue(vp.isValid)

            let normalized: VerifiablePresentation = try VerifiablePresentation.fromJson(
            try testData.loadPresentationNormalizedJson())
            XCTAssertNotNil(normalized)
            XCTAssertTrue(normalized.isGenuine)
            XCTAssertTrue(normalized.isValid)
            XCTAssertEqual(try testData.loadPresentationNormalizedJson(), normalized.description)
            XCTAssertEqual(try testData.loadPresentationNormalizedJson(), vp.description)
        }catch {
            XCTFail()
        }
    }
}
