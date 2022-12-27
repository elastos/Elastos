
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
/*
    func testResolveLocal() {
        do {
            let json = "{\"id\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab\",\"publicKey\":[{\"id\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#primary\",\"type\":\"ECDSAsecp256r1\",\"controller\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab\",\"publicKeyBase58\":\"21YM84C9hbap4GfFSB3QbjauUfhAN4ETKg2mn4bSqx4Kp\"}],\"authentication\":[\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#primary\"],\"verifiableCredential\":[{\"id\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#name\",\"type\":[\"BasicProfileCredential\",\"SelfProclaimedCredential\"],\"issuer\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab\",\"issuanceDate\":\"2020-07-01T00:46:40Z\",\"expirationDate\":\"2025-06-30T00:46:40Z\",\"credentialSubject\":{\"id\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab\",\"name\":\"KP Test\"},\"proof\":{\"type\":\"ECDSAsecp256r1\",\"verificationMethod\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#primary\",\"signature\":\"jQ1OGwpkYqjxooyaPseqyr_1MncOZDrMS_SvwYzqkCHVrRfjv_b7qfGCjxy7Gbx-LS3bvxZKeMxU1B-k3Ysb3A\"}}],\"expires\":\"2025-07-01T00:46:40Z\",\"proof\":{\"type\":\"ECDSAsecp256r1\",\"created\":\"2020-07-01T00:47:20Z\",\"creator\":\"did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab#primary\",\"signatureValue\":\"TOpNt-pWeQDJFaS5EkpMOuCqnZKhPCizf7LYQQDBrNLVIZ_7AR73m-KJk7Aja0wmZWXd7S4n7SC2W4ZQayJlMA\"}}"
            let did = try DID("did:elastos:idFKwBpj3Buq3XbLAFqTy8LMAW8K7kp3Ab")

            try DIDBackend.initializeInstance(resolver, TestData.getResolverCacheDir())
            try ResolverCache.reset()

            var doc = try did.resolve()
            XCTAssertNotNil(doc)

            DIDBackend.self.setResolveHandle { d -> DIDDocument? in
                if d == did {
                    return try DIDDocument.convertToDIDDocument(fromJson: json)
                }
                return nil
            }

            doc = try did.resolve()
            XCTAssertNotNil(doc)
            XCTAssertEqual(did, doc?.subject)

            DIDBackend.setResolveHandle(nil)
            doc = try did.resolve()
            XCTAssertNil(doc)
        } catch {
            XCTFail()
        }
    }
 */
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    
}
