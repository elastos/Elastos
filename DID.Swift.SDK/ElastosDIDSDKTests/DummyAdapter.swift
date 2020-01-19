
import Foundation
import ElastosDIDSDK
import SPVWrapper

class DummyAdapter: DIDAdapter {

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
            print(str)
        }

        return str
    }
    
    func createIdTransaction(_ payload: String, _ memo: String?) throws -> String {
        do {
            let request: IDChainRequest = try IDChainRequest.fromJson(payload)
            
            if (verbose) {
                print("ID Transaction: \(request.operation)\(request.did!)")
                print("     \(request.toJson(false))\(request.did!)" )
            }
            
            if try !request.isValid() {
                throw TestError.failue("Invalid ID transaction request.")
            }
            if request.operation != IDChainRequest.Operation.DEACTIVATE {
                if try !request.doc!.isValid() {
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
                
                guard ti!.operation != IDChainRequest.Operation.DEACTIVATE else {
                    throw TestError.failue("DID already dactivated.")
                }
                
                guard request.previousTxid == ti!.transactionId else {
                    throw TestError.failue("Previous transaction id missmatch.")
                }
                break
                }
            case .DEACTIVATE: do {
                guard ti != nil else {
                    throw TestError.failue("DID not exist.")
                }
                
                guard ti!.operation != IDChainRequest.Operation.DEACTIVATE else {
                    throw TestError.failue("DID already dactivated.")
                }
                break
                }
            default: break
                
            }
            
            ti = IDTransactionInfo(DummyAdapter.generateTxId(), DateFormater.currentDate(), request)
            idtxs.append(ti!)
            
            return ti!.transactionId
        } catch {
            print(error)
            throw error
        }
    }
    
    func resolve(_ requestId: String, _ did: String, _ all: Bool) throws -> String {
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
        
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        dic["id"] = requestId
        dic["jsonrpc"] = "2.0"

        var redic: OrderedDictionary<String, Any> = OrderedDictionary()
        redic["did"] = target.description
        var status:Int = 3
        let last = getLastTransaction(target)
        
        if last != nil {
            if last!.operation == IDChainRequest.Operation.DEACTIVATE {
                status = 2
            }
            else {
                if last!.request.doc!.isExpired() {
                    status = 1
                }
                else {
                    status = 0
                }
            }
            matched = true
        }

        redic["status"] = String("\(status)")
        if status != 3 {
            let reversedArr: Array = idtxs.reversed()
            var arr: Array<OrderedDictionary<String, Any>> = [ ]
            for ti in reversedArr {
                if ti.did == target {
                    let dic = ti.toJson()
                    arr.append(dic)
                    if (!all) {
                        break
                    }
                }
            }
            redic["transaction"] = arr
        }

        dic["result"] = redic
        if (verbose) {
            print(matched ? "success" : "failed")
        }
        let json = JsonHelper.creatJsonString(dic: dic)
        return json
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
        print(Int(y))
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
        self.timestamp = DateFormater.currentDate()
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
            print(str)
        }

        return str
    }
    
    public func getOperation() -> IDChainRequest.Operation {
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
            print(Int(y))
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
