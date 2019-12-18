
import XCTest
import ElastosDIDSDK

class TestData: XCTestCase {
    private static var dummyAdapter: DIDAdapter?
    private static var spvAdapter: DIDAdapter?
    private static var rootKey: HDKey?
    private static var index: Int?
    private static var testIssuer: DIDDocument?
    private static var issuerCompactJson: String?
    private static var issuerNormalizedJson: String?
    private static var testDocument: DIDDocument?
    private static var testCompactJson: String?
    private static var testNormalizedJson: String?
    private static var profileVc: VerifiableCredential?
    private static var profileVcCompactJson: String?
    private static var profileVcNormalizedJson: String?
    private static var emailVc: VerifiableCredential?
    private static var emailVcCompactJson: String?
    private static var emailVcNormalizedJson: String?
    private static var passportVc: VerifiableCredential?
    private static var passportVcCompactJson: String?
    private static var passportVcNormalizedJson: String?
    private static var twitterVc: VerifiableCredential?
    private static var twitterVcCompactJson: String?
    private static var twitterVcNormalizedJson: String?
    private static var testVp: VerifiablePresentation?
    private static var testVpNormalizedJson: String?
    private static var restoreMnemonic: String?

    public func setupStore(_ dummyBackend: Bool) throws {
        var adapter: DIDAdapter = DummyAdapter()
        if dummyBackend {
            if TestData.dummyAdapter == nil {
                TestData.dummyAdapter = DummyAdapter() as? DIDAdapter
                adapter = TestData.dummyAdapter!
            }
        }
        else {
            if TestData.spvAdapter == nil {
                let cblock: PasswordCallback = ({(walletDir, walletId) -> String in return "test111111"})
                TestData.spvAdapter = (SPVAdaptor(walletDir, walletId, networkConfig, resolver, cblock) as? DIDAdapter)
            }
            deleteFile(storePath)
            adapter = TestData.spvAdapter!
        }
        try DIDStore.creatInstance("filesystem", storePath, adapter)
    }
    
    public func initIdentity() throws -> String {
        let mnemonic: String = HDKey.generateMnemonic(0)
        try DIDStore.shareInstance()?.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
        return mnemonic
    }
    
    func loadDIDDocument(_ fileName: String) throws -> DIDDocument {
        let bundle = Bundle(for: type(of: self))
        let jsonPath = bundle.path(forResource: fileName, ofType: "json")
        let doc: DIDDocument = try DIDDocument.fromJson(jsonPath!)
        
        if DIDStore.isInitialized() {
            let store: DIDStore = try DIDStore.shareInstance()!
            try store.storeDid(doc)
        }
        return doc
    }
    
    func importPrivateKey(_ id: DIDURL, _ fileName: String, _ type: String) throws {
        let skBase58: String = try loadText(fileName, type)
        let data = skBase58.data(using: .utf8)
        let udata = [UInt8](data!)
        let keyBase58: String = Base58.base58FromBytes(udata)
        try DIDStore.shareInstance()?.storePrivateKey(id.did, id, keyBase58, storePass)
    }
    
    func loadTestIssuer() throws -> DIDDocument {
        if TestData.testIssuer == nil {
            TestData.testIssuer = try loadDIDDocument("issuer")
            try importPrivateKey((TestData.testIssuer?.getDefaultPublicKey())!, "issuer.primary", "sk")
        }
        return TestData.testIssuer!
    }
    
    func loadTestDocument() throws -> DIDDocument {
        _ = try loadTestIssuer()
        if TestData.testDocument == nil {
            TestData.testDocument = try loadDIDDocument("document")
        }
        try importPrivateKey((TestData.testDocument?.getDefaultPublicKey())!, "document.primary", "sk")
        try importPrivateKey(TestData.testDocument!.getPublicKey("key2")!.id, "document.key2", "sk")
        try importPrivateKey(TestData.testDocument!.getPublicKey("key3")!.id, "document.key3", "sk")
        return TestData.testDocument!
    }
    
    func loadCredential(_ fileName: String, _ type_: String) throws -> VerifiableCredential {
        let buldle = Bundle(for: type(of: self))
        let jsonstr = buldle.path(forResource: fileName, ofType: type_)
        let vc: VerifiableCredential = try VerifiableCredential.fromJson(jsonstr!)
        if DIDStore.isInitialized() {
            try DIDStore.shareInstance()?.storeCredential(vc)
        }
        return vc
    }
    
    public func loadProfileCredential() throws -> VerifiableCredential {
        if TestData.profileVc == nil {
            TestData.profileVc = try loadCredential("vc-profile", "json")
        }
        return TestData.profileVc!
    }
    
    public func loadEmailCredential() throws -> VerifiableCredential {
        if TestData.emailVc == nil {
            TestData.emailVc = try loadCredential("vc-email", "json")
        }
        return TestData.emailVc!
    }
    
    public func loadPassportCredential() throws -> VerifiableCredential {
        if TestData.passportVc == nil {
            TestData.passportVc = try loadCredential("vc-passport", "json")
        }
        return TestData.passportVc!
    }
    
    public func loadTwitterCredential() throws -> VerifiableCredential {
        if TestData.twitterVc == nil {
            TestData.twitterVc = try loadCredential("vc-twitter", "json")
        }
        return TestData.twitterVc!
    }
    
    public func loadPresentation() throws -> VerifiablePresentation {
        if TestData.testVp == nil {
            let bl = Bundle(for: type(of: self))
            let jsonstr = bl.path(forResource: "vp", ofType: "json")
            TestData.testVp = try VerifiablePresentation.fromJson(jsonstr!)
        }
        return TestData.testVp!
    }
    
    func loadText(_ fileName: String, _ type_: String) throws -> String {
        let bl = Bundle(for: type(of: self))
        let jsonstr = bl.path(forResource: fileName, ofType: type_)
        return jsonstr!
    }
    
    public func loadIssuerCompactJson() throws -> String {
        if TestData.issuerCompactJson == nil {
            TestData.issuerCompactJson = try loadText("issuer.compact", "json")
        }
        return TestData.issuerCompactJson!
    }
    
    public func loadIssuerNormalizedJson() throws -> String {
        if TestData.issuerNormalizedJson == nil {
            TestData.issuerNormalizedJson = try loadText("issuer.normalized", "json")
        }
        return TestData.issuerNormalizedJson!
    }
    
    public func loadTestCompactJson() throws -> String {
        if TestData.testCompactJson == nil {
            TestData.testCompactJson = try loadText("document.compact", "json")
        }
        return TestData.testCompactJson!
    }
    
    public func loadTestNormalizedJson() throws -> String {
        if TestData.testNormalizedJson == nil {
            TestData.testNormalizedJson = try loadText("document.normalized", "json")
        }
        return TestData.testNormalizedJson!
    }
    
    public func loadProfileVcCompactJson() throws -> String {
        if TestData.profileVcCompactJson == nil {
            TestData.profileVcCompactJson = try loadText("vc-profile.compact", "json")
        }
        return TestData.profileVcCompactJson!
    }
    
    public func loadProfileVcNormalizedJson() throws -> String {
        if TestData.profileVcNormalizedJson == nil {
            TestData.profileVcNormalizedJson = try loadText("vc-profile.normalized", "json")
        }
        return TestData.profileVcNormalizedJson!
    }
    
    public func loadEmailVcCompactJson() throws -> String {
        if TestData.emailVcCompactJson == nil {
            TestData.emailVcCompactJson = try loadText("vc-email.compact", "json")
        }
        return TestData.emailVcCompactJson!
    }
    
    public func loadEmailVcNormalizedJson() throws -> String {
        if TestData.emailVcNormalizedJson == nil {
            TestData.emailVcNormalizedJson = try loadText("vc-email.normalized", "json")
        }
        return TestData.emailVcNormalizedJson!
    }
    
    public func loadPassportVcCompactJson() throws -> String {
        if TestData.passportVcCompactJson == nil {
            TestData.passportVcCompactJson = try loadText("vc-passport.compact", "json")
        }
        return TestData.passportVcCompactJson!
    }
    
    public func loadPassportVcNormalizedJson() throws -> String {
        if TestData.passportVcNormalizedJson == nil {
            TestData.passportVcNormalizedJson = try loadText("vc-passport.normalized", "json")
        }
        return TestData.passportVcNormalizedJson!
    }
    
    public func loadTwitterVcCompactJson() throws -> String {
        if TestData.twitterVcCompactJson == nil {
            TestData.twitterVcCompactJson = try loadText("vc-twitter.compact", "json")
        }
        return TestData.twitterVcCompactJson!
    }
    
    public func loadTwitterVcNormalizedJson() throws -> String {
        if TestData.twitterVcNormalizedJson == nil {
            TestData.twitterVcNormalizedJson = try loadText("vc-twitter.normalized", "json")
        }
        return TestData.twitterVcNormalizedJson!
    }
    
    public func loadPresentationNormalizedJson() throws -> String {
        if TestData.testVpNormalizedJson == nil {
            TestData.testVpNormalizedJson = try loadText("vp.normalized", "json")
        }
        return TestData.testVpNormalizedJson!
    }
    
    public func loadRestoreMnemonic() throws -> String {
        if TestData.restoreMnemonic == nil {
            // TODO: load test
            TestData.restoreMnemonic = try loadText("mnemonic", "restore")
        }
        return TestData.restoreMnemonic!
    }
    
    public func deleteFile() throws -> String {
        if TestData.restoreMnemonic == nil {
            // TODO: load test
            TestData.restoreMnemonic = try loadText("mnemonic", "restore")
        }
        return TestData.restoreMnemonic!
    }
    
    public class func generateKeypair() throws -> DerivedKey {
        if TestData.rootKey == nil {
            let mnemonic: String = HDKey.generateMnemonic(0)
            TestData.rootKey = try HDKey.fromMnemonic(mnemonic, "")
            TestData.index = 0
        }
        TestData.index = TestData.index! + 1
        return try TestData.rootKey!.derive(TestData.index!)
    }

    func deleteFile(_ path: String) {
        do {
            let filemanager: FileManager = FileManager.default
            var isdir = ObjCBool.init(false)
            let fileExists = filemanager.fileExists(atPath: path, isDirectory: &isdir)
            if fileExists && isdir.boolValue {
                if let dircontents = filemanager.enumerator(atPath: path) {
                    for case let url as URL in dircontents {
                        deleteFile(url.absoluteString)
                    }
                }
            }
            guard fileExists else {
                return
            }
            try filemanager.removeItem(atPath: path)
        } catch {
            print("deleteFile error: \(error)")
        }
    }
    
    func exists(_ dirPath: String) -> Bool {
        let fileManager = FileManager.default
        var isDir : ObjCBool = false
        if fileManager.fileExists(atPath: dirPath, isDirectory:&isDir) {
            if isDir.boolValue {
                return true
            }
        }
        return false
    }
    
    func existsFile(_ path: String) -> Bool {
        var isDirectory = ObjCBool.init(false)
        let fileExists = FileManager.default.fileExists(atPath: path, isDirectory: &isDirectory)
        return !isDirectory.boolValue && fileExists
    }
    
    func currentDateToWantDate(_ year: Int)-> Date {
        let current = Date()
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone(abbreviation: "UTC")!
        var comps:DateComponents?
        
        comps = calendar.dateComponents([.year, .month, .day, .hour, .minute, .second], from: current)
        comps?.year = Constants.MAX_VALID_YEARS
        comps?.month = 0
        comps?.day = 0
        comps?.hour = 0
        comps?.minute = 0
        comps?.second = 0
        comps?.nanosecond = 0
        let realDate = calendar.date(byAdding: comps!, to: current) ?? Date()
        let hour = calendar.component(.hour, from: realDate)
        let useDate = calendar.date(bySettingHour: hour, minute: 00, second: 00, of: realDate) ?? Date()
        
        return useDate
    }
}
