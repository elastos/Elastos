import Foundation
@testable import ElastosDIDSDK
import SPVWrapper

class DummyAdapter: DIDAdapter, DIDResolver {
    
    private var verbose: Bool = false
    private var idtxs: Array<IDTransactionInfo> = []
    
    public init(_ verbose: Bool) {
        self.verbose = verbose
    }

    public init() {
        self.verbose = false
    }

    private static func generateTxId() -> String {
        var str = ""
        while(str.count < 32){
            let re = randomCustom(min: 0, max: 9)
            let r = decTohex(number: re)
            str = "\(str)\(r)"
        }

        return str
    }
    
    func createIdTransaction(_ payload: String, _ memo: String?, _ confirms: Int, _ callback: @escaping (String, Int, String?) -> Void) {
        
        do {
            let txid = try createIdTransaction(payload, memo)
            callback(txid, 0, nil);
        }
        catch {
            // TODO: callback("")
            callback("", -1, error.localizedDescription);
        }
    }
    
    func createIdTransaction(_ payload: String, _ memo: String?) throws -> String {
        
        do {
            let request: IDChainRequest = try IDChainRequest.fromJson(payload)
            
            if (verbose) {
                print("ID Transaction: \(request.operation)\(request.did!)")
                print("     \(request.toJson(false))\(request.did!)" )
                
                if request.operation != IDChainRequestOperation.DEACTIVATE {
                    print("     \(request.document!.toString(true))" )
                }
            }
            
            if !request.isValid {
                throw TestError.failue("Invalid ID transaction request.")
            }
            if request.operation != IDChainRequestOperation.DEACTIVATE {
                if !request.document!.isValid {
                    throw TestError.failue("Invalid DID Document.")
                }
            }
            var ti = getLastTransaction(request.did!)
            
            switch request.operation {
            case .CREATE: do {
                guard ti == nil else {
                    throw TestError.failue("DID already exist.")
                }
                break
                }
            case .UPDATE: do {
                guard ti != nil else {
                    throw TestError.failue("DID not exist.")
                }
                
                guard ti!.operation != IDChainRequestOperation.DEACTIVATE else {
                    throw TestError.failue("DID already dactivated.")
                }
                
                guard request.previousTransactionId == ti!.transactionId else {
                    throw TestError.failue("Previous transaction id missmatch.")
                }
                break
                }
            case .DEACTIVATE: do {
                guard ti != nil else {
                    throw TestError.failue("DID not exist.")
                }
                
                guard ti!.operation != IDChainRequestOperation.DEACTIVATE else {
                    throw TestError.failue("DID already dactivated.")
                }
                break
                }
            }
            //TODO: Date()
            ti = IDTransactionInfo(DummyAdapter.generateTxId(), Date(), request)
            idtxs.append(ti!)
            
            return ti!.transactionId
        } catch {
            print(error)
            throw error
        }
    }

    func resolve(_ requestId: String, _ did: String, _ all: Bool) throws -> Data {
        
        if (verbose) {
            print("Resolve: " + did + "...")
        }
        
        var d = did
        let hashStr = did.prefix(12)
        
        if hashStr != "did:elastos:" {
            d = "did:elastos:\(did)"
        }
        let target: DID = try DID(d)
        var matched: Bool = false
        let generator = JsonGenerator()
        generator.writeStartObject()
        generator.writeStringField("id", requestId)
        generator.writeStringField("jsonrpc", "2.0")
        generator.writeFieldName("result")
        generator.writeStartObject()
        generator.writeStringField("did", target.toString())

        var status = 3
        let last = getLastTransaction(target)
        if last != nil {
            if last!.operation == IDChainRequestOperation.DEACTIVATE {
                status = 2
            }
            else {
                if last!.request.document!.isExpired {
                    status = 1
                }
                else {
                    status = 0
                }
            }
            matched = true
        }
        generator.writeNumberField("status", status)

        if status != 3 {
            generator.writeFieldName("transaction")
            generator.writeStartArray()
            let reversedArr: Array = idtxs.reversed()
            for ti in reversedArr {
                if ti.did == target {
                    ti.toJson(generator)
                    if (!all) {
                        break
                    }
                }
            }
            generator.writeEndArray()
        }
        generator.writeEndObject()
        generator.writeEndObject()

        if (verbose) {
            print(matched ? "success" : "failed")
        }
        return generator.toString().data(using: .utf8)!
    }
    
    public func reset() {
        idtxs.removeAll()
    }
    
    private func getLastTransaction(_ did: DID) -> IDTransactionInfo? {
        let reversedArr = idtxs.reversed()
        for ti in reversedArr {
            if ti.did == did {
                return ti
            }
        }
        return nil
    }
    
    static func randomCustom(min: Int, max: Int) -> Int {
        //  [min, max)  [0, 100)
        //        var x = arc4random() % UInt32(max);
        //        return Int(x)
        // [min, max）
        let y = arc4random() % UInt32(max) + UInt32(min)
        return Int(y)
    }
    
    static func decTohex(number:Int) -> String {
        return String(format: "%0X", number)
    }
}

class IDTx {
    
    private var txId: String
    private var timestamp: Date
    private var request: IDChainRequest
    
    init(_ request: IDChainRequest) {
        self.txId = IDTx.generateTxId()
        self.timestamp = Date() // TODO:
        self.request = request
    }
    
    public func getDid() -> DID {
        return request.did!
    }
    
    private static func generateTxId() -> String {
        var str = ""
        while(str.count < 32){
            let re = randomCustom(min: 0, max: 9)
            let r = decTohex(number: re)
            str = "\(str)\(r)"
        }

        return str
    }
    
    public func getOperation() -> IDChainRequestOperation {
        return request.operation
    }

    public func getPayload() -> String {
        return request.toJson(false)
    }
    
    static func randomCustom(min: Int, max: Int) -> Int {
            //  [min, max)  [0, 100)
            //        var x = arc4random() % UInt32(max);
            //        return Int(x)
            // [min, max）
            let y = arc4random() % UInt32(max) + UInt32(min)
            return Int(y)
    }
    
    static func decTohex(number:Int) -> String {
         return String(format: "%0X", number)
     }
    
    public func toJson() throws -> String {
        let json = request.toJson(false)
        return "{\"id\": \"1\",\"jsonrpc\":\"2.0\",\"result\":[\(json)]}"
    }
}
