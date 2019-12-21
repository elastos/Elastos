
import Foundation
import ElastosDIDSDK
import SPVAdapter

class DummyAdapter: DIDAdapter {
    private var verbose: Bool = false
    private var txs: Array<IDTx> = []
    
    public init(_ verbose: Bool) {
        self.verbose = verbose
    }

    public init() {
        self.verbose = false
    }

    func createIdTransaction(_ payload: String, _ memo: String?) throws -> Bool {
        do {
            let request: IDChainRequest = try IDChainRequest.fromJson(payload)

            if (verbose) {
                print("ID Transaction: \(request.operation)\(request.did)")
                print("     \(request.toJson(false))\(request.did)" )
            }
            
            txs.append(IDTx(request))
            
            return true
        } catch {
            print(error)
        }
        
        return false
    }
    
    func resolve(_ did: String) throws -> String? {
        var d = did
        let hashStr = did.prefix(12)

        if hashStr != "did:elastos:" {
            d = "did:elastos:\(did)"
        }
        
        if (verbose) {
            print("Resolve: " + d + "...")
        }
        
        let target: DID = try DID(d)
        for tx in txs {
            if (tx.getDid() == target) {
                if (verbose) {
                    print("success")
                }
                return try tx.toJson()
            }
        }
        
        if (verbose) {
            print("failed")
        }
        return nil
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

    public func getDid() -> DID {
        return request.did!
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
