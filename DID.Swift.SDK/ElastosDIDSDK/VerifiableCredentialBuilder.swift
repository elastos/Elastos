import Foundation

public class VerifiableCredentialBuilder {
    private var _target: DID
    private var _signKey: DIDURL
    private var _forDoc: DIDDocument
    private var _credential: VerifiableCredential?

    init(_ target: DID, _ doc: DIDDocument, _ signKey: DIDURL) {
        self._target  = target
        self._forDoc  = doc
        self._signKey = signKey

        self._credential = VerifiableCredential()
        self._credential!.setIssuer(doc.subject)
    }

    public func withId(_ id: DIDURL) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }

        self._credential!.setId(id)
        return self
    }

    public func withId(_ id: String) throws -> VerifiableCredentialBuilder {
        guard !id.isEmpty else {
            throw DIDError.illegalArgument()
        }

        return try withId(try DIDURL(self._target, id))
    }

    public func withTypes(_ types: String...) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard types.count > 0 else {
            throw DIDError.illegalArgument()
        }

        self._credential!.setType(types)
        return self
    }

    public func withDefaultExpirationDate() throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }

        self._credential!.setExpirationDate(DateHelper.maxExpirationDate())
        return self
    }

    public func withExpirationDate(_ expirationDate: Date) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }

        let maxExpirationDate = DateHelper.maxExpirationDate()
        guard !DateHelper.isExpired(expirationDate, maxExpirationDate) else {
            throw DIDError.illegalArgument()
        }

        self._credential!.setExpirationDate(expirationDate)
        return self
    }

    public func withProperties(_ properties: Dictionary<String,String>) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard properties.count > 0 else {
            throw DIDError.illegalArgument()
        }

        // TODO:
        return self
    }

    public func withProperties(_ json: String) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard !json.isEmpty else {
            throw DIDError.illegalArgument()
        }

        // TODO:
        return self
    }

    public func withProperties(_ data: Dictionary<String, Any>) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }

        // TODO:

        let subject = VerifiableCredentialSubject(self._target)
        subject.setProperties(JsonNode(data))
        self._credential!.setSubject(subject)

        return self
    }

    public func seal(using storePass: String) throws -> VerifiableCredential {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard self._credential!.checkIntegrity() else {
            throw DIDError.malformedCredential("imcomplete credential")
        }

        let date = DateHelper.currentDate()
        self._credential!.setIssuanceDate(date)

        if self._credential!.getExpirationDate() == nil {
            _ = try withDefaultExpirationDate()
        }

        let json = self._credential!.toJson(true, true)
        let signature = try self._forDoc.sign(using: self._signKey, storePass: storePass, json.data(using: .utf8)!)
        let proof = VerifiableCredentialProof(Constants.DEFAULT_PUBLICKEY_TYPE, self._signKey, signature)

        self._credential!.setProof(proof)

        // invalidate builder
        let credential = self._credential!
        self._credential = nil

        return credential
    }
}
