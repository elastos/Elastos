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

protocol DIDStorage {
    // Root private identity
    func containsPrivateIdentity() -> Bool
    func storePrivateIdentity(_ key: String) throws
    func loadPrivateIdentity() throws -> String

    func containsPublicIdentity() -> Bool
    func storePublicIdentity(_ key: String) throws
    func loadPublicIdentity() throws -> String

    func storePrivateIdentityIndex(_ index: Int) throws
    func loadPrivateIdentityIndex() throws -> Int

    func containsMnemonic() -> Bool
    func storeMnemonic(_ mnemonic: String) throws
    func loadMnemonic() throws -> String
    
    // DIDs
    func storeDidMetadata(_ did: DID, _ meta: DIDMeta) throws
    func loadDidMetadata(_ did: DID) throws -> DIDMeta

    func storeDid(_ doc: DIDDocument) throws
    func loadDid(_ did: DID) throws -> DIDDocument?
    func containsDid(_ did: DID) -> Bool
    func deleteDid(_ did: DID) -> Bool
    func listDids(_ filter: Int) throws -> Array<DID>

    // Credentials
    func storeCredentialMetadata(_ did: DID, _ id: DIDURL, _ meta: CredentialMeta) throws
    func loadCredentialMetadata(_ did: DID, _ id: DIDURL) throws -> CredentialMeta

    func storeCredential(_ credential: VerifiableCredential) throws
    func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential
    func containsCredentials(_ did: DID) -> Bool
    func containsCredential(_ did: DID, _ id: DIDURL) -> Bool
    func deleteCredential(_ did: DID, _ id: DIDURL) -> Bool
    func listCredentials(_ did: DID) throws -> Array<DIDURL>
    func selectCredentials(_ did: DID, _ id: DIDURL?, _ type: Array<Any>?) throws -> Array<DIDURL>

    // Private keys
    func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws
    func loadPrivateKey(_ did: DID, _ id: DIDURL) throws -> String
    func containsPrivateKeys(_ did: DID) -> Bool
    func containsPrivateKey(_ did: DID, _ id: DIDURL) -> Bool
    func deletePrivateKey(_ did: DID, _ id: DIDURL) -> Bool

    func changePassword(_  callback: (String) throws -> String) throws
}
