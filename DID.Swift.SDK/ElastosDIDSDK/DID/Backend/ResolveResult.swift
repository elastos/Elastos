
import Foundation

public class ResolveResult {
    private static let DID: String = "did"
    private static let STATUS: String = "status"
    private static let TRANSACTION: String = "transaction"

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
    
    public func getTransactionCount() -> Int {
        return idtxs.count
    }
    
    public func getTransactionInfo(_ index: Int) -> IDTransactionInfo? {
        if (idtxs.count == 0) {
            return nil
        }

        return idtxs[index]
    }

    func addTransactionInfo(_ ti: IDTransactionInfo) {
        objc_sync_enter(self)
        
        defer {
            objc_sync_exit(self)
        }
        idtxs.append(ti)
    }
    
    public func toJson() throws -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        dic[ResolveResult.DID] = did.description
        dic[ResolveResult.STATUS] = status
        

        if (status != ResolveResult.STATUS_NOT_FOUND) {
            var arr: Array<Any> = []
            for ti in idtxs {
                let d = try ti.toJson()
                arr.append(d)
            }
            dic[ResolveResult.TRANSACTION] = arr
        }
        return dic
    }
    
    public func toJson() throws -> String {
        let dic: OrderedDictionary<String, Any> = try toJson()
        return JsonHelper.creatJsonString(dic: dic)
    }
    
    public class func fromJson(_ result: OrderedDictionary<String, Any>) throws -> ResolveResult {
        
        let did: DID = try JsonHelper.getDid(result, DID, false, nil, "Resolved result DID")!
        
        let status: Int = try JsonHelper.getInteger(result, STATUS, false, -1, "Resolved status")
        
        let rr: ResolveResult = ResolveResult(did, status)
        
        if (status != ResolveResult.STATUS_NOT_FOUND) {
            let txs: Array<Any> = result[TRANSACTION] as! Array<Any>
            if (txs.count == 0)
            {
                throw DIDResolveError.failue("Invalid resolve result, missing transaction.")
            }
            for i in 0..<txs.count {
                let ti: IDTransactionInfo = try IDTransactionInfo.fromJson(txs[i] as! OrderedDictionary<String, Any>)
                rr.addTransactionInfo(ti)
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
    
    public func description() throws -> String {
        return try toJson()
    }

}

