/*
* Copyright (c) 2020 Elastos Foundation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

import Foundation

private func mapToString(_ dict: OrderedDictionary<String, String>?, _ sep: String) -> String? {
    guard let _ = dict else {
        return nil
    }

    var first  = true
    var result = ""

    for (key, value) in dict! {
        result.append(!first ? sep : "")
        result.append(key)
        if value != "" {
            result.append("=")
        }
        result.append(value)

        if  first {
            first = false
        }
    }
    return result
}

/// DID URL defines by the did-url rule, refers to a URL that begins with a DID followed by one or more additional components.
/// A DID URL always identifies the resource to be located. DIDURL includes DID and Url fragment by user defined.
public class DIDURL {
    private static let TAG = "DIDURL"

    private var _did: DID?
    private var _fragment: String?

    private var _parameters: OrderedDictionary<String, String>?
    private var _path: String?
    private var _queryParameters: OrderedDictionary<String, String>?
    private var _metadata: CredentialMeta?

    /// Create a new DID URL according to DID and fragment.
    /// - Parameters:
    ///   - id: A valid didurl guaranteed containing valid did.
    ///   - url: A fragment string.
    /// - Throws: If error occurs, throw error.
    public init(_ id: DID, _ url: String) throws {
        guard !url.isEmpty else {
            throw DIDError.illegalArgument("empty didurl string")
        }

        var fragment = url
        if url.hasPrefix("did:") {
            do {
                try ParserHelper.parse(url, false, DIDURL.Listener(self))
            } catch {
                Log.e(DIDURL.TAG, "Parsing didurl error: malformed didurl string \(url)")
                throw DIDError.malformedDIDURL("malformed DIDURL \(url)")
            }

            guard did == id else {
                throw DIDError.illegalArgument("Mismatched arguments")
            }
            return
        }

        if url.hasPrefix("#") {
            let starIndex = fragment.index(fragment.startIndex, offsetBy: 1)
            let endIndex  = fragment.index(starIndex, offsetBy: fragment.count - 2)
            fragment  = String(fragment[starIndex...endIndex])
        }
        self._did = id
        self._fragment = fragment
    }

    /// Get DID URL from string.
    /// - Parameter url: A  string including id information. idstring support: 1. “did:elastos:xxxxxxx#xxxxx”
    /// - Throws: If error occurs, throw error.
    public init(_ url: String) throws {
        guard !url.isEmpty else {
            throw DIDError.illegalArgument()
        }

        do {
            try ParserHelper.parse(url, false, DIDURL.Listener(self))
        } catch {
            Log.e(DIDURL.TAG, "Parsing didurl error: malformed didurl string \(url)")
            throw DIDError.malformedDIDURL("malformed DIDURL \(url)")
        }
    }

    // A valid didurl guaranteed containing valid did.
    public var did: DID {
        return _did!
    }

    /// Set did
    /// - Parameter newValue: The new did
    public func setDid(_ newValue: DID) {
        self._did = newValue
    }

    // Regards to DIDs v1.0 specs:
    // "DID URL: A DID plus an optional DID path, optional ? character followed
    //  by a DID query, and optional # character followed by a DID fragment."
    public var fragment: String? {
        return _fragment
    }

    func setFragment(_ newValue: String) {
        self._fragment = newValue
    }

    /// <#Description#>
    /// - Returns: <#description#>
    public func parameters() -> String? {
        return mapToString(_parameters, ";")
    }

    public func parameter(ofKey: String) -> String? {
        return _parameters?[ofKey]
    }

    /// Check is contains parameter
    /// - Parameter forKey: <#forKey description#>
    /// - Returns: <#description#>
    public func containsParameter(forKey: String) -> Bool {
        return _parameters?.keys.contains(forKey) ?? false
    }

    func appendParameter(_ value: String?, forKey: String) {
        if  self._parameters == nil {
            self._parameters = OrderedDictionary()
        }
        self._parameters![forKey] = value
    }

    /// <#Description#>
    public var path: String? {
        return _path
    }

    func setPath(_ newValue: String) {
        self._path = newValue
    }

    /// <#Description#>
    /// - Returns: <#description#>
    public func queryParameters() -> String? {
        return mapToString(_queryParameters, "&")
    }

    /// <#Description#>
    /// - Parameter ofKey: <#ofKey description#>
    /// - Returns: <#description#>
    public func queryParameter(ofKey: String) -> String? {
        return _queryParameters?[ofKey]
    }

    /// <#Description#>
    /// - Parameter forKey: <#forKey description#>
    /// - Returns: <#description#>
    public func containsQueryParameter(forKey: String) -> Bool {
        return _queryParameters?.keys.contains(forKey) ?? false
    }

    /// <#Description#>
    /// - Parameters:
    ///   - value: <#value description#>
    ///   - forKey: <#forKey description#>
    public func appendQueryParameter(_ value: String?, forKey: String) {
        if  self._queryParameters == nil {
            self._queryParameters = OrderedDictionary()
        }
        self._queryParameters![forKey] = value
    }

    func setMetadata(_ metadata: CredentialMeta) {
        self._metadata = metadata
    }

    /// Get CredentialMetaData from Credential.
    /// - Returns: Return the handle to CredentialMetaData
    public func getMetadata() -> CredentialMeta {
        if  self._metadata == nil {
            self._metadata = CredentialMeta()
        }
        return self._metadata!
    }

    /// Save Credential(DIDURL) MetaData.
    /// - Throws: If error occurs, throw error.
    public func saveMetadata() throws {
        if (_metadata != nil && _metadata!.attachedStore) {
            try _metadata?.store?.storeCredentialMetadata(did, self, _metadata!)
        }
    }
}

extension DIDURL: CustomStringConvertible {
    func toString() -> String {
        var builder: String = ""

        builder.append(did.toString())
        if (parameters() != nil) {
            builder.append(";")
            builder.append(parameters()!)
        }
        if !(path?.isEmpty ?? true) {
            builder.append(path!)
        }
        if (queryParameters() != nil) {
            builder.append("?")
            builder.append(queryParameters()!)
        }

        if !(_fragment?.isEmpty ?? true) {
            builder.append("#")
            builder.append(fragment!)
        }
        return builder
    }

    /// Get id string from DID URL.
    public var description: String {
        return toString()
    }
}

extension DIDURL: Equatable {
    func equalsTo(_ other: DIDURL) -> Bool {
        return toString() == other.toString()
    }

    func equalsTo(_ other: String) -> Bool {
        return toString() == other
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
        hasher.combine(self.toString())
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

        override func exitMethodSpecificString(
                            _ ctx: DIDURLParser.MethodSpecificStringContext) {
            self.didURL?.did.setMethodSpecificId(ctx.getText())
        }

        override func enterParams(_ ctx: DIDURLParser.ParamsContext) {
            self.didURL?._parameters = OrderedDictionary()
        }

        override func exitParamMethod(_ ctx: DIDURLParser.ParamMethodContext) {
            let method = ctx.getText()
            if  method != Constants.METHOD {
                Log.e(DIDURL.TAG, "Unknown parameter method: \(method)")
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
            let value = self.value ?? ""
            self.didURL?.appendParameter(value, forKey: self.name!)
            self.name = nil
            self.value = nil
        }

        override func exitPath(_ ctx: DIDURLParser.PathContext) {
            self.didURL?.setPath("/" + ctx.getText())
        }

        override func enterQuery(_ ctx: DIDURLParser.QueryContext) {
            self.didURL?._queryParameters = OrderedDictionary()
        }

        override func exitQueryParamName(_ ctx: DIDURLParser.QueryParamNameContext) {
            self.name = ctx.getText()
        }

        override func exitQueryParamValue(_ ctx: DIDURLParser.QueryParamValueContext) {
            self.value = ctx.getText()
        }

        override func exitQueryParam(_ ctx: DIDURLParser.QueryParamContext) {
            let value = self.value ?? ""
            self.didURL?.appendQueryParameter(value, forKey: self.name!)
            self.name = nil
            self.value = nil
        }

        override func exitFrag(_ ctx: DIDURLParser.FragContext) {
            self.didURL?.setFragment(ctx.getText())
        }
    }
}
