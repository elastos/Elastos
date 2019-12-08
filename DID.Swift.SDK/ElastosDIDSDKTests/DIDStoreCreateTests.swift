

import XCTest
import ElastosDIDSDK

class DIDStoreCreateTests: XCTestCase {
    var adapter: SPVAdaptor!

    override func setUp() {
        // Put setup code here. This method is called before the invocation of each test method in the class.
        let cblock: PasswordCallback = ({(walletDir, walletId) -> String in return "test111111"})
        adapter = SPVAdaptor(walletDir, walletId, networkConfig, resolver, cblock)
    }
    
    func test00CreateEmptyStore0() {
        TestUtils.deleteFile(storePath)
        do {
            let cblock: PasswordCallback = ({(walletDir, walletId) -> String in return "test111111"})
            adapter = SPVAdaptor(walletDir, walletId, networkConfig, resolver, cblock)
            try DIDStore.creatInstance("filesystem", storePath, adapter)
            let tempStore: DIDStore = try DIDStore.shareInstance()!
            XCTAssertFalse(try! tempStore.hasPrivateIdentity())
            XCTAssertTrue(TestUtils.exists(storePath))
            let path: String = storePath + "/" + ".DIDStore"
            let filemanage: FileManager = FileManager.default
            filemanage.fileExists(atPath: path)
        } catch {
            print("test00CreateEmptyStore0 error: \(error)")
        }
    }
    
    func test00Restore() {
        do {
            TestUtils.deleteFile(storePath)
            try DIDStore.creatInstance("filesystem", storePath, adapter)

            let store: DIDStore = try DIDStore.shareInstance()!
            let mnemonic: String = "harvest goddess absorb secret drift rail smooth eight boy fresh faculty spawn"
            try store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
            try store.synchronize(storePass)

            let dids = try store.listDids(DIDStore.DID_HAS_PRIVATEKEY)

            XCTAssertEqual(10, dids.count)
            // TODO: improve
        } catch {
            print("test00Restore error: \(error)")
        }
    }
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

}
