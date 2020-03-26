
import XCTest
@testable import ElastosDIDSDK

class IssuerTest: XCTestCase {
    
    func testnewIssuerTestWithSignKey() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let signKey: DIDURL = issuerDoc.defaultPublicKey
            let issuer = try VerifiableCredentialIssuer(issuerDoc.subject, signKey, store)
            XCTAssertEqual(issuerDoc.subject, issuer.did)
            XCTAssertEqual(signKey, issuer.signKey)
        } catch {
            XCTFail()
        }
    }
    
    func testnewIssuerTestWithoutSignKey() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let issuer = try VerifiableCredentialIssuer(issuerDoc.subject, store)
            XCTAssertEqual(issuerDoc.subject, issuer.did)
            XCTAssertEqual(issuerDoc.defaultPublicKey, issuer.signKey)
        } catch  {
            XCTFail()
        }
    }
    
    func testnewIssuerTestWithInvalidKey() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            var issuerDoc: DIDDocument = try testData.loadTestIssuer()
            
            let key: HDKey.DerivedKey = try TestData.generateKeypair()
            let signKey: DIDURL = try DIDURL(issuerDoc.subject, "testKey")
            let db: DIDDocumentBuilder = issuerDoc.editing()
            _ = try db.appendAuthenticationKey(with: signKey, keyBase58: key.getPublicKeyBase58())
            
            issuerDoc = try db.sealed(using: storePass)
            XCTAssertTrue(issuerDoc.isValid)

            let doc = issuerDoc
            XCTAssertThrowsError(try VerifiableCredentialIssuer(doc, signKey)) { (error) in
                switch error {
                case DIDError.illegalArgument:
                //everything is fine
                    XCTAssertTrue(true)
                default:
                    XCTFail("Unexpected error thrown")
                }
            }
        } catch {
            XCTFail()
        }
    }
    
    func testnewIssuerTestWithInvalidKey2() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let signKey: DIDURL = try DIDURL(issuerDoc.subject, "recovery")
            let doc = issuerDoc
            XCTAssertThrowsError(try VerifiableCredentialIssuer(doc, signKey)) { (error) in
                switch error {
                case DIDError.illegalArgument:
                //everything is fine
                    XCTAssertTrue(true)
                default:
                    XCTFail("Unexpected error thrown")
                }
            }
        }
        catch {
            XCTFail()
        }
    }
    
    func testIssueKycCredentialTest() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            let testDoc: DIDDocument = try testData.loadTestDocument()
            
            var props: Dictionary<String, String> = [: ]
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            
            let issuer = try VerifiableCredentialIssuer(issuerDoc)
            let cb = issuer.editingVerifiableCredentialFor(did: testDoc.subject)
            let vc: VerifiableCredential = try cb.withId("testCredential")
                .withTypes("BasicProfileCredential", "InternetAccountCredential")
                .withProperties(props)
                .sealed(using: storePass)

            let vcId: DIDURL = try DIDURL(testDoc.subject, "testCredential")

            XCTAssertEqual(vcId, vc.getId())
            XCTAssertTrue(vc.getTypes().contains("BasicProfileCredential"))
            XCTAssertTrue(vc.getTypes().contains("InternetAccountCredential"))
            XCTAssertFalse(vc.getTypes().contains("SelfProclaimedCredential"))
            
            XCTAssertEqual(issuerDoc.subject, vc.issuer)
            XCTAssertEqual(testDoc.subject, vc.subject.did)
            
            XCTAssertEqual("John", vc.subject.getPropertyAsString(ofName: "name"))
            XCTAssertEqual("Male", vc.subject.getPropertyAsString(ofName:"gender"))
            XCTAssertEqual("Singapore", vc.subject.getPropertyAsString(ofName:"nation"))
            XCTAssertEqual("English", vc.subject.getPropertyAsString(ofName:"language"))
            XCTAssertEqual("john@example.com", vc.subject.getPropertyAsString(ofName:"email"))
            XCTAssertEqual("@john", vc.subject.getPropertyAsString(ofName:"twitter"))
            
            XCTAssertFalse(vc.isExpired)
            XCTAssertTrue(vc.isGenuine)
            XCTAssertTrue(vc.isValid)
        }
        catch {
            XCTFail()
        }
    }
    
    func testIssueSelfProclaimedCredentialTest() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            
            let issuerDoc: DIDDocument = try testData.loadTestIssuer()
            
            var props: Dictionary<String, String> = [: ]
            props["name"] = "Testing Issuer"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "issuer@example.com"
            let issuer =  try VerifiableCredentialIssuer(issuerDoc)
            let cb = issuer.editingVerifiableCredentialFor(did: issuerDoc.subject)
            let vc: VerifiableCredential = try cb.withId("myCredential")
                .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
                .withProperties(props)
                .sealed(using: storePass)
            
            let vcId: DIDURL = try DIDURL(issuerDoc.subject, "myCredential")
            XCTAssertEqual(vcId, vc.getId())
            XCTAssertTrue(vc.getTypes().contains("BasicProfileCredential"))
            XCTAssertTrue(vc.getTypes().contains("SelfProclaimedCredential"))
            XCTAssertFalse(vc.getTypes().contains("InternetAccountCredential"))
            
            XCTAssertEqual(issuerDoc.subject, vc.issuer)
            XCTAssertEqual(issuerDoc.subject, vc.subject.did)
            
            XCTAssertEqual("Testing Issuer", vc.subject.getPropertyAsString(ofName: "name"))
            XCTAssertEqual("Singapore", vc.subject.getPropertyAsString(ofName:"nation"))
            XCTAssertEqual("English", vc.subject.getPropertyAsString(ofName:"language"))
            XCTAssertEqual("issuer@example.com", vc.subject.getPropertyAsString(ofName:"email"))
            
            XCTAssertFalse(vc.isExpired)
            XCTAssertTrue(vc.isGenuine)
            XCTAssertTrue(vc.isValid)
        }
        catch {
            XCTFail()
        }
    }
    
    func testIssueJsonPropsCredential() {
        do {
            let testData = TestData()
            _ = try testData.setupStore(true)
            let issuerDoc = try testData.loadTestIssuer()
            let props: String = "{\"name\":\"Jay Holtslander\",\"alternateName\":\"Jason Holtslander\",\"booleanValue\":true,\"numberValue\":1234,\"doubleValue\":9.5,\"nationality\":\"Canadian\",\"birthPlace\":{\"type\":\"Place\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"}},\"affiliation\":[{\"type\":\"Organization\",\"name\":\"Futurpreneur\",\"sameAs\":[\"https://twitter.com/futurpreneur\",\"https://www.facebook.com/futurpreneur/\",\"https://www.linkedin.com/company-beta/100369/\",\"https://www.youtube.com/user/CYBF\"]}],\"alumniOf\":[{\"type\":\"CollegeOrUniversity\",\"name\":\"Vancouver Film School\",\"sameAs\":\"https://en.wikipedia.org/wiki/Vancouver_Film_School\",\"year\":2000},{\"type\":\"CollegeOrUniversity\",\"name\":\"CodeCore Bootcamp\"}],\"gender\":\"Male\",\"Description\":\"Technologist\",\"disambiguatingDescription\":\"Co-founder of CodeCore Bootcamp\",\"jobTitle\":\"Technical Director\",\"worksFor\":[{\"type\":\"Organization\",\"name\":\"Skunkworks Creative Group Inc.\",\"sameAs\":[\"https://twitter.com/skunkworks_ca\",\"https://www.facebook.com/skunkworks.ca\",\"https://www.linkedin.com/company/skunkworks-creative-group-inc-\",\"https://plus.google.com/+SkunkworksCa\"]}],\"url\":\"https://jay.holtslander.ca\",\"image\":\"https://s.gravatar.com/avatar/961997eb7fd5c22b3e12fb3c8ca14e11?s=512&r=g\",\"address\":{\"type\":\"PostalAddress\",\"addressLocality\":\"Vancouver\",\"addressRegion\":\"BC\",\"addressCountry\":\"Canada\"},\"sameAs\":[\"https://twitter.com/j_holtslander\",\"https://pinterest.com/j_holtslander\",\"https://instagram.com/j_holtslander\",\"https://www.facebook.com/jay.holtslander\",\"https://ca.linkedin.com/in/holtslander/en\",\"https://plus.google.com/+JayHoltslander\",\"https://www.youtube.com/user/jasonh1234\",\"https://github.com/JayHoltslander\",\"https://profiles.wordpress.org/jasonh1234\",\"https://angel.co/j_holtslander\",\"https://www.foursquare.com/user/184843\",\"https://jholtslander.yelp.ca\",\"https://codepen.io/j_holtslander/\",\"https://stackoverflow.com/users/751570/jay\",\"https://dribbble.com/j_holtslander\",\"http://jasonh1234.deviantart.com/\",\"https://www.behance.net/j_holtslander\",\"https://www.flickr.com/people/jasonh1234/\",\"https://medium.com/@j_holtslander\"]}"
            let issuer = try VerifiableCredentialIssuer(issuerDoc)
            let cb = issuer.editingVerifiableCredentialFor(did: issuerDoc.subject)
            let vc = try cb.withId("myCredential")
                .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
                .withProperties(props)
                .sealed(using: storePass)
            
            let vcId = try DIDURL(issuerDoc.subject, "myCredential")
            XCTAssertEqual(vcId, vc.getId())
            XCTAssertTrue(vc.getTypes().contains("BasicProfileCredential"))
            XCTAssertTrue(vc.getTypes().contains("SelfProclaimedCredential"))
            XCTAssertFalse(vc.getTypes().contains("InternetAccountCredential"))
            XCTAssertEqual(issuerDoc.subject, vc.issuer)
            XCTAssertEqual(issuerDoc.subject, vc.subject.did)

            XCTAssertEqual("Technologist", vc.subject.getPropertyAsString(ofName: "Description"))
            XCTAssertEqual("Jason Holtslander", vc.subject.getPropertyAsString(ofName:"alternateName"))
            XCTAssertEqual("1234", vc.subject.getPropertyAsString(ofName:"numberValue"))
            XCTAssertEqual("9.5", vc.subject.getPropertyAsString(ofName:"doubleValue"))

            XCTAssertNotNil(vc.subject.getProperties())
            XCTAssertFalse(vc.isExpired)
            XCTAssertTrue(vc.isGenuine)
            XCTAssertTrue(vc.isValid)
        } catch {
            print(error)
            XCTFail()
        }
    }
}
