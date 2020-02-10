import Foundation

public class VerifiableCredentialBuilder {
    private var _target: DID
    private var _signKey: DIDURL
    private var _credential: VerifiableCredential?

    init(_ target: DID, _ doc: DIDDocument, _ signKey: DIDURL) {
        self._target  = target
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

    private func getMaxExpires() -> Date {
        let date: Date = Date()

        // TODO
        // if credential.issuanceDate != nil {
        //    date = self.credential.issuanceDate!
        // }
        // return DateFormater.dateToWantDate(date, VerifiableCredentialBuilder.MAX_VALID_YEARS)
        return date
    }

    public func withDefaultExpirationDate() throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }

        self._credential!.setExpirationDate(getMaxExpires())
        return self
    }

    public func withExpirationDate(_ expirationDate: Date) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }

        var date: Date = expirationDate
        let maxExpirationDate = getMaxExpires()
        if DateFormater.comporsDate(expirationDate, maxExpirationDate) {
            date = maxExpirationDate
        }
        self._credential!.setExpirationDate(date)
        return self
    }

    public func withProperties(_ properties: Dictionary<String,String>) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard properties.count > 0 else {
            throw DIDError.illegalArgument()
        }

        /*
        TODO:
        var subject = VerifiableCredentialSubject(self.target)
        // subject.property(ofName: )
        credential!.subject = subject
        */
        return self
    }

    public func withProperties(_ json: String) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard !json.isEmpty else {
            throw DIDError.illegalArgument()
        }

        // TODO
        return self
    }

    public func withProperties(_ node: JsonNode) throws -> VerifiableCredentialBuilder {
        guard let _ = self._credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard node.size > 0 else {
            throw DIDError.illegalArgument()
        }

        let subject = VerifiableCredentialSubject(self._target)
        subject.setProperties(node)
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
        guard let _ = self._credential!.getSubject() else {
            throw DIDError.malformedCredential("Missing subject")
        }
        /*
        TODO:
        guard let _ = self._credential!.types else {
            throw DIDError.malformedCredentialError("Missing types")
        }
        guard let _ = self._credential!.expirationDate else {
            throw DIDError.malformedCredentialError("Missing expirated Date")
        }
        */

        let date = DateFormater.currentDate()
        self._credential!.setIssuanceDate(date)

        // let dict = _verifiableCredential?.toJson(true)

        /*

        if credential.expirationDate == nil {
            defaultExpirationDate()
        }

        let dic = self.credential.toJson(true, true)
        let json = JsonHelper.creatJsonString(dic: dic)
        let inputs: [CVarArg] = [json, json.count]
        let count: Int = inputs.count / 2
        let sig: String = try (self.document.sign(signKey, storepass, count, inputs))

        let proof = CredentialProof(DEFAULT_PUBLICKEY_TYPE, signKey, sig)
        self.credential.proof = proof

        return self.credential
        */
        /*
        let proof = VerifiableCredentialProof(DEFAULT_PUBLICKEY_TYPE, signKey, sig)
        credential.proof = proof
        */

        let vc = self._credential!
        self._credential = nil
        return vc
    }
}
