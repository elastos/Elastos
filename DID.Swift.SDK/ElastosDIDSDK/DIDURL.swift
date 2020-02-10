import Foundation

public class DIDURL {
    // The fields below would be filled in the contructors.
    private var _did: DID?
    private var _fragment: String?

    private var _parameters: Dictionary<String, String>?
    private var _path: String?
    private var _queryParameters: Dictionary<String, String>?
    private var _meta: CredentialMeta?

    public init(_ id: DID, _ fragment: String) throws {
        self._did = id
        self._fragment = fragment

        guard !fragment.isEmpty else {
            throw DIDError.illegalArgument()
        }

        if fragment.hasPrefix("did:") {
            do {
                try ParserHelper.parse(fragment, false, DIDURL.Listener(self))
            } catch {
                throw DIDError.malformedDID("Malformed DIDURL \(fragment)")
            }

            guard self._did == id else {
                throw DIDError.illegalArgument("Mismatch arguments")
            }
        }

        /* TODO:
        if  fragment.hasPrefix("#") {
            // TODO:
        }
        */
    }
    
    public init(_ url: String) throws {
        guard !url.isEmpty else {
            throw DIDError.illegalArgument()
        }

        do {
            try ParserHelper.parse(url, false, DIDURL.Listener(self))
        } catch {
            throw DIDError.malformedDIDURL("Malformed DIDURL \(url)")
        }
    }

    public var did: DID {
        return self._did!
    }

    public func setDid(_ newValue: DID) {
        self._did = newValue
    }

    public var fragment: String {
        return self._fragment!
    }

    func setFragment(_ newValue: String) {
        self._fragment = newValue
    }

    private func mapToString(_ dict: Dictionary<String, String>?, _ sep: String) -> String? {
        guard let _ = dict else {
            return nil
        }

        var first  = true
        var result = ""

        for (key, value) in dict! {
            result.append(!first ? sep : "")
            result.append(key)
            result.append("=")
            result.append(value)

            if  first {
                first = false
            }
        }
        return result
    }

    public func parameters() -> String? {
        return mapToString(self._parameters, ";")
    }

    public func parameter(ofKey: String) -> String? {
        return self._parameters?[ofKey]
    }

    public func containsParameter(forKey: String) -> Bool {
        return self._parameters?.keys.contains(forKey) ?? false
    }

    public func appendParameter(_ value: String?, forKey: String) {
        if  self._parameters == nil {
            self._parameters = Dictionary<String, String>()
        }
        self._parameters![forKey] = value
    }

    // Path
    public var path: String? {
        return self._path
    }

    func setPath(_ newValue: String) {
        self._path = newValue
    }

    // QueryParameters
    public func queryParameters() -> String? {
        return mapToString(self._queryParameters!, "&")
    }

    public func queryParameter(ofKey: String) -> String? {
        return self._queryParameters?[ofKey]
    }

    public func containsQueryParameter(forKey: String) -> Bool {
        return self._queryParameters?.keys.contains(forKey) ?? false
    }
    
    public func appendQueryParameter(_ value: String?, forKey: String) {
        if  self._queryParameters == nil {
            self._queryParameters = Dictionary<String, String>()
        }
        self._queryParameters![forKey] = value
    }

    // meta
    func getMeta() -> CredentialMeta {
        if  self._meta == nil {
            self._meta = CredentialMeta()
        }
        return self._meta!
    }

    func setMeta(_ meta: CredentialMeta) {
        self._meta = meta
    }

    public func setExtra(value: String, forName: String) throws {
        guard !forName.isEmpty else {
            throw DIDError.illegalArgument()
        }

        getMeta().setExtra(value, forName)
        if getMeta().hasAttachedStore {
            try getMeta().store?.storeCredentialMeta(self.did, self, getMeta())
        }
    }

    public func getExtra(forName: String) -> String? {
        return getMeta().getExtra(forName)
    }

    public var aliasName: String {
        return getMeta().aliasName
    }

    // when alias value is nil, mean to clean alias.
    public func setAlias(_ newValue: String?) throws {
        getMeta().setAlias(newValue)
        if getMeta().hasAttachedStore {
            try getMeta().store?.storeCredentialMeta(did, self, getMeta())
        }
    }
}

extension DIDURL: CustomStringConvertible {
    func toString() -> String {
        var builder: String = ""

        builder.append(did.toString())
        if !(self._parameters?.isEmpty ?? true) {
            builder.append(":")
            builder.append(parameters()!)
        }
        if !(self.path?.isEmpty ?? true) {
            builder.append(self.path!)
        }
        if !(self._queryParameters?.isEmpty ?? true) {
            builder.append("?")
            builder.append(queryParameters()!)
        }
        if !self.fragment.isEmpty {
            builder.append("#")
            builder.append(self.fragment)
        }
        return builder
    }

    public var description: String {
        return toString()
    }
}

extension DIDURL: Equatable {
    func equalsTo(_ other: DIDURL) -> Bool {
        return self == other || self.toString() == other.toString()
    }

    func equalsTo(_ other: String) -> Bool {
        return self.toString() == other
    }

    public static func == (lhs: DIDURL, rhs: DIDURL) -> Bool {
        return lhs.equalsTo(rhs)
    }

    public static func != (lhs: DIDURL, rhs: DIDURL) -> Bool {
        return !lhs.equalsTo(rhs)
    }
}

// DIDURL used as hash key.
extension DIDURL: Hashable {
    public func hash(into hasher: inout Hasher) {
        // TODO: ???
    }
}

// Parse Listener
extension DIDURL {
    class Listener: DIDURLBaseListener {
        private var name: String?
        private var value: String?
        private var didURL: DIDURL?

        init(_ didURL: DIDURL) {
            self.didURL = didURL
            super.init()
        }

        override func enterDid(_ ctx: DIDURLParser.DidContext) {
            self.didURL?.setDid(DID())
        }

        override func exitMethod(_ ctx: DIDURLParser.MethodContext) {
            let method = ctx.getText()
            if  method != Constants.METHOD {
                print("Unknown method: \(method)")
            }
            self.didURL?.did.setMethod(Constants.METHOD)
        }

        override func exitMethodSpecificString(_ ctx: DIDURLParser.MethodSpecificStringContext) {
            self.didURL?.did.setMethodSpecificId(ctx.getText())
        }

        override func enterParams(_ ctx: DIDURLParser.ParamsContext) {
            self.didURL?._parameters = Dictionary<String, String>()
        }

        override func exitParamMethod(_ ctx: DIDURLParser.ParamMethodContext) {
            let method = ctx.getText()
            if method != Constants.METHOD {
                print("Unknown parameter method: \(method)")
            }
            self.didURL?.did.setMethod(method)
        }

        override func exitParamQName(_ ctx: DIDURLParser.ParamQNameContext) {
            self.name = ctx.getText()
        }

        override func exitParamValue(_ ctx: DIDURLParser.ParamValueContext) {
            self.value = ctx.getText()
        }

        override func exitParam(_ ctx: DIDURLParser.ParamContext) {
            self.didURL?.appendParameter(self.value, forKey: self.name!)
            self.value = nil
            self.name = nil
        }

        override func exitPath(_ ctx: DIDURLParser.PathContext) {
            self.didURL?.setPath("/" + ctx.getText())
        }

        override func enterQuery(_ ctx: DIDURLParser.QueryContext) {
            self.didURL?._queryParameters = Dictionary<String, String>()
        }

        override func exitQueryParamName(_ ctx: DIDURLParser.QueryParamNameContext) {
            self.name = ctx.getText()
        }

        override func exitQueryParamValue(_ ctx: DIDURLParser.QueryParamValueContext) {
            self.value = ctx.getText()
        }

        override func exitQueryParam(_ ctx: DIDURLParser.QueryParamContext) {
            self.didURL?.appendQueryParameter(self.value, forKey: self.name!)
            self.name = nil
            self.value = nil
        }

        override func exitFrag(_ ctx: DIDURLParser.FragContext) {
            self.didURL?.setFragment(ctx.getText())
        }
    }
}
