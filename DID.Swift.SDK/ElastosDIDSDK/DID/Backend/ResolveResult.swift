
import Foundation

public class ResolveResult {
    private static let DID = "did"
    private static let STATUS = "status"
    private static let TRANSACTION = "transaction"

    public static let STATUS_VALID: Int = 0
    public static let STATUS_EXPIRED = 1
    public static let STATUS_DEACTIVATED = 2
    public static let STATUS_NOT_FOUND = 3

    public var did: DID!
    public var status: Int!
    public var idtxs: Array<IDTransactionInfo> = []
    
    init(_ did: DID, _ status: Int) {
        self.did = did
        self.status = status
    }

    public var transactionCount: Int {
        get {
            return idtxs.count
        }
    }

    public func transactionInfo(atIndex: Int) -> IDTransactionInfo? {
        guard idtxs.count > atIndex else {
            return nil
        }
        return idtxs[atIndex]
    }

    func append(newTransactionInfo: IDTransactionInfo) {
        objc_sync_enter(self)
        defer { objc_sync_exit(self) }

        idtxs.append(newTransactionInfo)
    }
    
    public func toJson() -> OrderedDictionary<String, Any> {
        var dict: OrderedDictionary<String, Any> = OrderedDictionary()
        dict[ResolveResult.DID] = did.description
        dict[ResolveResult.STATUS] = status

        if (status != ResolveResult.STATUS_NOT_FOUND) {
            var arr: Array<Any> = []
            for ti in idtxs {
                arr.append(ti.toJson())
            }
            dict[ResolveResult.TRANSACTION] = arr
        }

        return dict
    }
    
    public func toJson() -> String {
        let dict: OrderedDictionary<String, Any> = toJson()
        return JsonHelper.creatJsonString(dic: dict)
    }
    
    public class func fromJson(_ result: OrderedDictionary<String, Any>) throws -> ResolveResult {
        
        let did: DID = try JsonHelper.getDid(result, DID, false, nil, "Resolved result DID")!
        
        let status: Int = try JsonHelper.getInteger(result, STATUS, false, -1, "Resolved status")
        
        let rr: ResolveResult = ResolveResult(did, status)
        
        if (status != ResolveResult.STATUS_NOT_FOUND) {
            let txs: Array<Any> = result[TRANSACTION] as! Array<Any>
            if (txs.count == 0)
            {
                throw DIDError.didResolveError(_desc: "Invalid resolve result, missing transaction.")
            }
            for i in 0..<txs.count {
                let ti: IDTransactionInfo = try IDTransactionInfo.fromJson(txs[i] as! OrderedDictionary<String, Any>)
                rr.append(newTransactionInfo: ti)
            }
        }
        
        return rr
    }
    
    public class func fromJson(_ json: String) throws -> ResolveResult? {
        let string = JsonHelper.preHandleString(json)
        let ordDic = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        let result = ordDic["result"] as! Array<Any>
        
        if (result.count == 0) {
            return nil
        }
        let re: OrderedDictionary<String, Any> = result[0] as! OrderedDictionary<String, Any>
        return try fromJson(re)
    }
}

extension ResolveResult: CustomStringConvertible {
   @objc public var description: String {
        return toJson()
    }
}
