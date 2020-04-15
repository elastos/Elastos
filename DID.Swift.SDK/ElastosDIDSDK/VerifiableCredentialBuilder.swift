import Foundation

public class VerifiableCredentialBuilder {
    private var _target: DID
    private var _signKey: DIDURL
    private var _forDoc: DIDDocument

    private var credential: VerifiableCredential?

    init(_ target: DID, _ doc: DIDDocument, _ signKey: DIDURL) {
        self._target  = target
        self._forDoc  = doc
        self._signKey = signKey

        self.credential = VerifiableCredential()
        self.credential!.setIssuer(doc.subject)
    }

    public func withId(_ id: DIDURL) throws -> VerifiableCredentialBuilder {
        guard let _ = credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }

        credential!.setId(id)
        return self
    }

    public func withId(_ id: String) throws -> VerifiableCredentialBuilder {
        guard !id.isEmpty else {
            throw DIDError.illegalArgument()
        }

        return try withId(DIDURL(_target, id))
    }

    public func withTypes(_ types: String...) throws -> VerifiableCredentialBuilder {
        guard let _ = credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard types.count > 0 else {
            throw DIDError.illegalArgument()
        }

        credential!.setType(types)
        return self
    }
    
    public func withTypes(_ types: Array<String>) throws -> VerifiableCredentialBuilder {
         guard let _ = credential else {
             throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
         }
         guard types.count > 0 else {
             throw DIDError.illegalArgument()
         }

         credential!.setType(types)
         return self
     }

    public func withDefaultExpirationDate() throws -> VerifiableCredentialBuilder {
        guard let _ = credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }

        credential!.setExpirationDate(maxExpirationDate())
        return self
    }

    public func withExpirationDate(_ expirationDate: Date) throws -> VerifiableCredentialBuilder {
        guard let _ = credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }

        guard !DateHelper.isExpired(expirationDate, maxExpirationDate()) else {
            throw DIDError.illegalArgument()
        }

        // TODO: check
        credential!.setExpirationDate(expirationDate)
        return self
    }

    public func withProperties(_ properites: Dictionary<String, String>) throws -> VerifiableCredentialBuilder {
        guard let _ = credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard !properites.isEmpty else {
            throw DIDError.illegalArgument()
        }
        // TODO: CHECK
        let jsonNode = JsonNode(properites)
        let subject = VerifiableCredentialSubject(_target)
        subject.setProperties(jsonNode)
        credential!.setSubject(subject)
        
        return self
    }

    public func withProperties(_ json: String) throws -> VerifiableCredentialBuilder {
        guard let _ = credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard !json.isEmpty else {
            throw DIDError.illegalArgument()
        }
        // TODO: CHECK
        let dic = try (JSONSerialization.jsonObject(with: json.data(using: .utf8)!, options: [JSONSerialization.ReadingOptions.init(rawValue: 0)]) as? [String: Any])
        guard let _ = dic else {
            throw DIDError.malformedCredential("properties data formed error.")
        }
        let jsonNode = JsonNode(dic!)
        let subject = VerifiableCredentialSubject(_target)
        subject.setProperties(jsonNode)
        credential!.setSubject(subject)
        
        return self
    }

    public func withProperties(_ properties: JsonNode) throws -> VerifiableCredentialBuilder {
        guard let _ = credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard properties.count > 0 else {
            throw DIDError.illegalArgument()
        }

        let subject = VerifiableCredentialSubject(_target)
        subject.setProperties(properties)

        credential!.setSubject(subject)
        return self
    }

    public func sealed(using storePassword: String) throws -> VerifiableCredential {
        guard let _ = credential else {
            throw DIDError.invalidState(Errors.CREDENTIAL_ALREADY_SEALED)
        }
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard credential!.checkIntegrity() else {
            throw DIDError.malformedCredential("imcomplete credential")
        }

        credential!.setIssuanceDate(DateHelper.currentDate())
        if credential!.getExpirationDate() == nil {
            _ = try withDefaultExpirationDate()
        }

        guard let data = credential!.toJson(true, true).data(using: .utf8) else {
            throw DIDError.illegalArgument("credential is nil")
        }
        let signature = try _forDoc.sign(_signKey, storePassword, [data])
        let proof = VerifiableCredentialProof(Constants.DEFAULT_PUBLICKEY_TYPE, _signKey, signature)

        credential!.setProof(proof)

        // invalidate builder
        let sealed = self.credential!
        self.credential = nil

        return sealed
    }

    private func maxExpirationDate() -> Date {
        guard credential?.getIssuanceDate() == nil else {
            return DateFormatter.convertToWantDate(credential!.issuanceDate, Constants.MAX_VALID_YEARS)
        }
        return DateFormatter.convertToWantDate(Date(), Constants.MAX_VALID_YEARS)
    }
}
