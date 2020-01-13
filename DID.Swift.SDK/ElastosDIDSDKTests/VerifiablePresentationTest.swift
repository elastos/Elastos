
import XCTest
import ElastosDIDSDK

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
            XCTAssertEqual(testDoc.subject, vp.getSigner())
            XCTAssertEqual(4, vp.getCredentialCount())
            let vcs: Array<VerifiableCredential> = vp.getCredentials()
            for vc in vcs {
                XCTAssertEqual(testDoc.subject, vc.subject.id)
                let re = vc.id.fragment == "profile" || vc.id.fragment == "email" || vc.id.fragment == "twitter" || vc.id.fragment == "passport"
                XCTAssertTrue(re)
            }
            XCTAssertNotNil(try vp.getCredential(DIDURL(vp.getSigner(), "profile")))
            XCTAssertNotNil(try vp.getCredential(DIDURL(vp.getSigner(), "email")))
            XCTAssertNotNil(try vp.getCredential(DIDURL(vp.getSigner(), "twitter")))
            XCTAssertNotNil(try vp.getCredential(DIDURL(vp.getSigner(), "passport")))
            XCTAssertNil(try vp.getCredential(DIDURL(vp.getSigner(), "notExist")))
            
            XCTAssertTrue(try vp.isGenuine())
            XCTAssertTrue(try vp.isValid())
            
        }
        catch {
            XCTFail()
        }
    }
    
    func testBuild() {
        do {
            // TODO
            let testData = TestData()
            let store = try testData.setupStore(true)
            // For integrity check
            _ = try testData.loadTestIssuer()
            let testDoc = try testData.loadTestDocument()
            
            let arr: Array<VerifiableCredential> = [
                try testData.loadProfileCredential()!,
                try testData.loadEmailCredential(),
                try testData.loadTwitterCredential(),
                try testData.loadPassportCredential()!]
            let vp = try VerifiablePresentation.seal(for: testDoc.subject!, store, arr, "https://example.com/", "873172f58701a9ee686f0630204fee59", storePass)
            XCTAssertNotNil(vp)
            XCTAssertEqual("VerifiablePresentation", vp.type)
            XCTAssertEqual(testDoc.subject!, vp.getSigner())

            XCTAssertEqual(4, vp.getCredentialCount())
            let vcs = vp.getCredentials()
            
            vcs.forEach { vc in
                let re = vc.id.fragment == "profile" || vc.id.fragment == "email" || vc.id.fragment == "twitter" || vc.id.fragment == "passport"
                XCTAssertTrue(re)
            }
            
            XCTAssertNotNil(try vp.getCredential(DIDURL(vp.getSigner(), "profile")))
            XCTAssertNotNil(try vp.getCredential(DIDURL(vp.getSigner(), "email")))
            XCTAssertNotNil(try vp.getCredential(DIDURL(vp.getSigner(), "twitter")))
            XCTAssertNotNil(try vp.getCredential(DIDURL(vp.getSigner(), "passport")))
            XCTAssertNil(try vp.getCredential(DIDURL(vp.getSigner(), "notExist")))

            XCTAssertTrue(try vp.isGenuine())
            XCTAssertTrue(try vp.isValid())
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
            XCTAssertTrue(try vp.isGenuine())
            XCTAssertTrue(try vp.isValid())

            let normalized: VerifiablePresentation = try VerifiablePresentation.fromJson(
            try testData.loadPresentationNormalizedJson())
            XCTAssertNotNil(normalized)
            XCTAssertTrue(try normalized.isGenuine())
            XCTAssertTrue(try normalized.isValid())
            XCTAssertEqual(try testData.loadPresentationNormalizedJson(), normalized.description)
            XCTAssertEqual(try testData.loadPresentationNormalizedJson(), vp.description)
        }catch {
            XCTFail()
        }
    }
}
