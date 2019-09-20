

import Foundation

public class DIDURL: NSObject {

    public var did: DID!
    public var parameters: Dictionary<String, String>!
    public var path: String! // TODO: readonly
    public var query: Dictionary<String, String>!
    public var fragment: String!

    public init(_ id: DID, _ fragment: String) throws {
        did = id
        self.fragment = fragment
    }

    public init(_ url: String) throws {
        ParserHelper.parase(url, false, Listener())
    }

    public func getParameters() -> String {
        // TODO: parameters is nil
        return parameters!.queryString
    }

    public func getParameter(_ name: String) -> String {
        // TODO: parameters is nil
        return parameters![name]!
    }

    public func hasParameter(_ name: String) -> Bool {
        return parameters.keys.contains(name)
    }

    public func getQuery() -> String {
        return query.queryString
    }

    public func getQueryParameter(_ name: String) -> String {
        // TODO: query is nil
        return query[name]!
    }

    public func hasQueryParameter(_ name: String) -> Bool {
        // TODO: query is nil
        return query.keys.contains(name)
    }

    public func toExternalForm() -> String {
        var str: String = "\(did!)"
        if !parameters.isEmpty {
            str = "\(str);\(getParameters())"
        }

        if !path.isEmpty {
            str = "\(str)\(path!)"
        }

        if !query.isEmpty {
            str = "\(str)?\(getQuery())"
        }

        if !fragment.isEmpty {
            str = "#\(fragment!)"
        }
        return str
    }

    // TODO: public String toString() { return toExternalForm(); }
}

class Listener: DIDURLBaseListener {

    override func enterDid(_ ctx: DIDURLParser.DidContext) {
        // TODO:
    }

    override func exitMethod(_ ctx: DIDURLParser.MethodContext) {
        let method: String = ctx.getText()
        if (method != DID.METHOD){
            // TODO: throw error
            //            let error = DIDError.failue("Unknown method: \(method)")
        }
        // TODO set method

    }

    override func exitMethodSpecificString(_ ctx: DIDURLParser.MethodSpecificStringContext) {
        // TODO setMethodSpecificId
    }

    override func enterParams(_ ctx: DIDURLParser.ParamsContext) {
        // TODO
    }
}

extension Dictionary {
    var queryString: String {
        var output: String = ""
        for (key,value) in self {
            output +=  "\(key)=\(value)&"
        }
        output = String(output.dropLast())
        return output
    }
}
