
import XCTest
import ElastosDIDSDK
import SPVAdapter

class TestDataGenerator: XCTest {
    private var outputDir: String = ""
    private var adapter: DIDAdapter!
    private var issuer: DIDDocument!
    private var test: DIDDocument!
    private var store: DIDStore!

    func create() throws -> String {
        do{
            let cblock: PasswordCallback = ({(walletDir, walletId) -> String in return "test111111"})
            adapter = SPVAdaptor(walletDir, walletId, networkConfig, resolver, cblock)
            //        TestUtils.deleteFile(storePath)
            store = try DIDStore.open("filesystem", storePath)
            DIDBackend.creatInstance(adapter)
            TestData.deleteFile(storePath)
            store = try DIDStore.open("filesystem", storePath)
            
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store!.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
            outputDir = tempDir + "/" + "DIDTestFiles"
            
            return mnemonic
        }
        catch {
            print(error)
            return ""
        }
    }
    
    func createTestIssuer() throws {
        do{
            let doc: DIDDocument = try store.newDid(storePass)
            print("Generate issuer DID: \(doc.subject)...")
            
            let selfIssuer = try Issuer(doc)
            var props: Dictionary<String, String> = [: ]
            props["name"] = "Test Issuer"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "issuer@example.com"
            
            let vc: VerifiableCredential = try selfIssuer.seal(for: doc.subject!,"profile", ["BasicProfileCredential", "SelfProclaimedCredential"], props, storePass)
            doc.addCredential(vc)
            issuer = try doc.seal(store, storePass)
            try store.storeDid(issuer)
            
            
            let id: DIDURL = issuer.getDefaultPublicKey()
            let sk: String = try store.loadPrivateKey(issuer.subject!, id: id)
            let data: Data = try store.decryptFromBase64(storePass, sk)
            let binSk: [UInt8] = [UInt8](data)
            writeTo("issuer." + id.fragment + ".sk", Base58.base58FromBytes(binSk))
            
            var json = try issuer.description(true)
            writeTo("issuer.normalized.json", json);
            
            json = try formatJson(json)
            writeTo("issuer.json", json);
            
            json = try issuer.description(false)
            writeTo("issuer.compact.json", json);
            print(try issuer.isValid())
        }
        catch {
            print(error)
        }
    }
    
    
    func createTestDocument() throws {
        do {
            let doc = try store.newDid(storePass)
            
            // Test document with two embedded credentials
            print("Generate test DID: \(doc.subject)...")
            
            var temp = try TestData.generateKeypair()
            try doc.addAuthenticationKey("key2", temp.getPublicKeyBase58())
//            writeTo("document.key2.sk", Base58.encode(temp.serialize()))
            
            temp = try TestData.generateKeypair()
            try doc.addAuthenticationKey("key3", temp.getPublicKeyBase58())
//            writeTo("document.key3.sk", Base58.encode(temp.serialize()))
            
            temp = try TestData.generateKeypair()
            let controller = DerivedKey.getIdString(try temp.getPublicKeyBytes())
            try doc.addAuthorizationKey("recovery", controller, temp.getPublicKeyBase58())
            
            try doc.addService("openid", "OpenIdConnectVersion1.0Service", "https://openid.example.com/");
            try doc.addService("vcr", "CredentialRepositoryService", "https://did.example.com/credentials");
            try doc.addService("carrier", "CarrierAddress", "carrier://X2tDd1ZTErwnHNot8pTdhp7C7Y9FxMPGD8ppiasUT4UsHH2BpF1d");
            
            let selfIssuer = try Issuer(doc)
            var props: Dictionary<String, String> = [: ]
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            let vcProfile: VerifiableCredential = try selfIssuer.seal(for: doc.subject!, "profile", ["BasicProfileCredential", "SelfProclaimedCredential"], props, storePass)
            
            let kycIssuer = try Issuer(issuer)
            
            props = [: ]
            props["email"] = "john@example.com"
            let vcEmail: VerifiableCredential = try selfIssuer.seal(for: doc.subject!, "email", ["BasicProfileCredential", "InternetAccountCredential", "EmailCredential"], props, storePass)
            doc.addCredential(vcProfile)
            doc.addCredential(vcEmail)
            test = try doc.seal(store, storePass)
            try store.storeDid(test)
            
            var id = test.getDefaultPublicKey()
            let sk = try store.loadPrivateKey(test.subject!, id: id)
            let data: Data = try store.decryptFromBase64(storePass, sk)
            let binSk = [UInt8](data)
            writeTo("document." + id.fragment + ".sk", Base58.base58FromBytes(binSk))
            
            var json = try test.description(true)
            writeTo("document.normalized.json", json)
            
            json = try formatJson(json)
            writeTo("document.json", json);
            
            json = try test.description(false)
            writeTo("document.compact.json", json);
            print(try test.isValid())
            
            // Profile credential
            print("Generate credential:  \(vcProfile.id)...")
            json = vcProfile.description(true)
            writeTo("vc-profile.normalized.json", json);
            
            json = try formatJson(json)
            writeTo("vc-profile.json", json);
            
            json = vcProfile.description(false)
            writeTo("vc-profile.compact.json", json);
            print(try vcProfile.isValid())
            
            // email credential
            print("Generate credential:  \(vcEmail.id)...")
            json = vcEmail.description(true)
            writeTo("vc-email.normalized.json", json);
            
            json = try formatJson(json)
            writeTo("vc-email.json", json);
            
            json = vcEmail.description(false)
            writeTo("vc-email.compact.json", json);
            print(try vcEmail.isValid())
            
            // Passport credential
            id = try DIDURL(test.subject!, "passport")
            print("Generate credential:  \(id)...")
            props = [: ]
            props["nation"] = "Singapore"
            props["passport"] = "S653258Z07"
            
            let vcPassport = try selfIssuer.seal(for: doc.subject!, "passport", ["BasicProfileCredential", "SelfProclaimedCredential"], props, storePass)
            
            json = vcPassport.description(true)
            writeTo("vc-passport.normalized.json", json)
            
            json = try formatJson(json)
            writeTo("vc-passport.json", json)
            
            json = vcPassport.description(false)
            writeTo("vc-passport.compact.json", json)
            print(try vcPassport.isValid())
            
            // Twitter credential
            id = try DIDURL(test.subject!, "twitter")
            print("Generate credential:  \(id)...")
            
            props = [: ]
            props["twitter"] = "@john"
            let vcTwitter =  try selfIssuer.seal(for: doc.subject!, "twitter", ["InternetAccountCredential", "TwitterCredential"], props, storePass)
            
            json = vcTwitter.description(true)
            writeTo("vc-twitter.normalized.json", json)
            
            json = try formatJson(json)
            writeTo("vc-twitter.json", json)
            
            json = vcTwitter.description(false)
            writeTo("vc-twitter.compact.json", json)
            
            print(try vcPassport.isValid())
            
            // Presentation with above credentials
            print("Generate presentation...")
            
            let vp: VerifiablePresentation = try VerifiablePresentation.seal(for: test.subject!, [vcProfile, vcEmail, vcPassport, vcTwitter], "https://example.com/", "873172f58701a9ee686f0630204fee59", storePass)
            json = vp.description
            writeTo("vp.normalized.json", json)
            
            json = try formatJson(json)
            writeTo("vp.json", json)
            
            print(try vcPassport.isValid())
        }
        catch {
            
        }
    }
    
    func writeTo(_ fileName: String, _ content: String) {
        let path = outputDir + "/" + fileName
        
        //        Writer out = new FileWriter(outputDir.getPath() + File.separator + fileName);
        //        out.write(content);
        //        out.close();
        
    }
    
    func formatJson(_ json: String) throws -> String {
        //        ObjectMapper mapper = new ObjectMapper();
        //        JsonNode node = mapper.readTree(json);
        //        json = mapper.writerWithDefaultPrettyPrinter().writeValueAsString(node);
        //        return json;
        return ""
    }
    
    public func createTestFiles() throws {
        try create()
        try createTestIssuer()
        try createTestDocument()
    }
    
    public func createTestDidsForRestore() throws {
        
        print("Generate mnemonic for restore...")
        let mnemonic = try create()
        writeTo("mnemonic.restore", mnemonic)
        print("OK")
        
        var dids: String = ""
        
        print("Generate DIDs for restore......")
        for _ in 0..<5 {
            print("******** Waiting for wallet available");
            while (true) {
                let a = adapter as! SPVAdaptor
                if try a.isAvailable() {
                    print(" OK")
                    break
                } else {
                    print(".")
                }
                
                // TODO: Waiting 30s
            }
            
            let doc = try store?.newDid(storePass)
            print("******** Publishing DID: \(doc?.subject)")
            try store?.publishDid(doc!, storePass)
            while (true) {
                // TODO: Waiting 30s
                do {
                    let d = try store?.resolveDid(doc!.subject!, true)
                    if (d != nil) {
                        print(" OK")
                        break
                    } else {
                        print(".")
                    }
                } catch {
                    print("x")
                }
            }
            dids = "\(dids)\n"
        }
        
        writeTo("dids.restore", dids.description)
        print("Generate DIDs for restore......OK")
    }
}

/*
     public static void main(String[] argc) throws Exception {
         TestDataGenerator bc = new TestDataGenerator();

         bc.createTestFiles();
         bc.createTestDidsForRestore();
     }
 }

 
 */
