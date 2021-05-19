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

public class VerifiablePresentationBuilder {
    private let _signer: DIDDocument
    private let _signKey: DIDURL
    private var _realm: String?
    private var _nonce: String?

    private var presentation: VerifiablePresentation?

    init(_ signer: DIDDocument, _ signKey: DIDURL) {
        self._signer = signer
        self._signKey = signKey

        self.presentation = VerifiablePresentation()
    }

    /// Set verifiable credentials for presentation.
    /// - Parameter credentials: Verifiable credentials
    /// - Throws: if an error occurred, throw error.
    /// - Returns: VerifiablePresentationBuilder instance.
    public func withCredentials(_ credentials: VerifiableCredential...) throws
        -> VerifiablePresentationBuilder {

        return try withCredentials(credentials)
    }

    /// Set verifiable credentials for presentation.
    /// - Parameter credentials: Verifiable credentials
    /// - Throws: if an error occurred, throw error.
    /// - Returns: VerifiablePresentationBuilder instance.
    public func withCredentials(_ credentials: Array<VerifiableCredential>) throws
        -> VerifiablePresentationBuilder {

        guard let _ = presentation else {
            throw DIDError.invalidState(Errors.PRESENTATION_ALREADY_SEALED)
        }

        for credential in credentials {
            // Presentation should be signed by the subject of Credentials
            guard credential.subject.did == self._signer.subject else {
                throw DIDError.illegalArgument(
                    "Credential \(credential.getId()) not match with requested id")
            }
            guard credential.checkIntegrity() else {
                throw DIDError.illegalArgument("incomplete credential \(credential.toString())")
            }

            presentation!.appendCredential(credential)
        }
        return self
    }

    /// Set realm for presentation.
    /// - Parameter realm: Target areas to which the expression applies, such as website domain names, application names, etc.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: VerifiablePresentationBuilder instance.
    public func withRealm(_ realm: String) throws -> VerifiablePresentationBuilder {
        guard let _ = presentation else {
            throw DIDError.invalidState(Errors.PRESENTATION_ALREADY_SEALED)
        }
        guard !realm.isEmpty else {
            throw DIDError.illegalArgument()
        }

        self._realm = realm
        return self
    }

    /// Set nonce for presentation.
    /// - Parameter nonce: Random value used for signature operation
    /// - Throws: if an error occurred, throw error.
    /// - Returns: VerifiablePresentationBuilder instance.
    public func withNonce(_ nonce: String) throws -> VerifiablePresentationBuilder {
        guard let _ = presentation else {
            throw DIDError.invalidState(Errors.PRESENTATION_ALREADY_SEALED)
        }
        guard !nonce.isEmpty else {
            throw DIDError.illegalArgument()
        }

        self._nonce = nonce
        return self
    }

    /// Finish modiy VerifiablePresentation.
    /// - Parameter storePassword: Pass word to sign.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: A handle to VerifiablePresentation.
    public func sealed(using storePassword: String) throws -> VerifiablePresentation {
        guard let _ = presentation else {
            throw DIDError.invalidState(Errors.PRESENTATION_ALREADY_SEALED)
        }
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard _realm != nil && _nonce != nil else {
            throw DIDError.invalidState("Missing realm and nonce")
        }

        var data: [Data] = []
        data.append(presentation!.toJson(true))
        if let realm = _realm {
            data.append(realm.data(using: .utf8)!)
        }
        if let nonce = _nonce {
            data.append(nonce.data(using: .utf8)!)
        }
        let signature = try _signer.sign(_signKey, storePassword, data)

        let proof = VerifiablePresentationProof(_signKey, _realm!, _nonce!, signature)
        presentation!.setProof(proof)

        // invalidate builder.
        let sealed = self.presentation!
        self.presentation = nil

        return sealed
    }
}
