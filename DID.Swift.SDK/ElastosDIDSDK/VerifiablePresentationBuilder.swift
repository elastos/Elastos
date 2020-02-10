import Foundation

public class VerifiablePresentationBuilder {
    private let _signer: DIDDocument
    private let _signKey: DIDURL
    private var _realm: String?
    private var _nonce: String?
    private var _presentation: VerifiablePresentation?

    init(_ signer: DIDDocument, _ signKey: DIDURL) {
        self._signer = signer
        self._signKey = signKey
        self._presentation = VerifiablePresentation()
    }

    public func withCredentials(_ credentials: VerifiableCredential...) throws
        -> VerifiablePresentationBuilder {

        guard let _ = self._presentation else {
            throw DIDError.invalidState(Errors.PRESENTATION_ALREADY_SEALED)
        }

        for credential in credentials {
            // Presentation should be signed by the subject of Credentials
            guard credential.subject.did != self._signer.subject else {
                throw DIDError.illegalArgument(
                    "Credential \(credential.getId()) not match with requested id")
            }

            // TODO: integrity check.
            self._presentation!.appendCredential(credential)
        }
        return self
    }

    public func withRealm(_ realm: String) throws -> VerifiablePresentationBuilder {
        guard let _ = self._presentation else {
            throw DIDError.invalidState(Errors.PRESENTATION_ALREADY_SEALED)
        }
        guard !realm.isEmpty else {
            throw DIDError.illegalArgument()
        }

        self._realm = realm
        return self
    }

    public func withNonce(_ nonce: String) throws -> VerifiablePresentationBuilder {
        guard let _ = self._presentation else {
            throw DIDError.invalidState(Errors.PRESENTATION_ALREADY_SEALED)
        }
        guard !nonce.isEmpty else {
            throw DIDError.illegalArgument()
        }

        self._nonce = nonce
        return self
    }

    public func seal(using storePass: String) throws -> VerifiablePresentation {
        guard let _ = self._presentation else {
            throw DIDError.invalidState(Errors.PRESENTATION_ALREADY_SEALED)
        }
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard self._presentation!.cedentialCount > 0 else {
            throw DIDError.illegalArgument()
        }
        guard self._realm != nil && self._nonce != nil else {
            throw DIDError.invalidState("Missing realm and nonce")
        }

        let json = self._presentation!.toJson(true)
        let signature = try self._signer.sign(using: self._signKey,
                                          storePass: storePass,
                                          json.data(using: .utf8)!)

        let proof = VerifiablePresentationProof(self._signKey, self._realm!, self._nonce!, signature)
        self._presentation!.setProof(proof)

        // invalidate builder.
        let vp = self._presentation!
        self._presentation = nil
        return vp
    }
}
