
public class VerifiablePresentationBuilder {
    var signer: DIDDocument
    var signKey: DIDURL
    var realm: String?
    var nonce: String?
    var presentation: VerifiablePresentation?
    
    init(_ signer: DIDDocument, _ signKey: DIDURL, _ store: DIDStore) {
        self.signer = signer
        self.signKey = signKey
        self.presentation = VerifiablePresentation()
    }
    
    
    public func credentials(_ credentials: Array<VerifiableCredential>) throws -> VerifiablePresentationBuilder {
        
        for vc in credentials {
            guard vc.subject.id == signer.subject else {
                throw DIDError.illegalArgument("Credential '\(vc.id!)' not match with requested did")
            }
            // TODO: integrity check?
            presentation!.addCredential(vc)
        }
        return self
    }
    
    public func realm(_ realm: String) -> VerifiablePresentationBuilder {
        self.realm = realm
        return self
    }
    
    public func nonce(_ nonce: String) -> VerifiablePresentationBuilder {
        self.nonce = nonce
        return self
    }
    
    public func seal(_ storepass: String) throws -> VerifiablePresentation {

        let dic = presentation!.toJson(true)
        let json = JsonHelper.creatJsonString(dic: dic)
        var inputs: [CVarArg] = []
        if json.count > 0 {
            inputs.append(json)
            inputs.append(json.count)
        }
        if realm != nil && !realm!.isEmpty {
            inputs.append(realm!)
            inputs.append(realm!.count)
        }
        if nonce != nil && !nonce!.isEmpty {
            inputs.append(nonce!)
            inputs.append(nonce!.count)
        }
        
        let count = inputs.count / 2
        let sig = try signer.sign(signKey, storepass, count, inputs)
        let proof = PresentationProof(signKey, realm!, nonce!, sig)
        presentation!.proof = proof
        let vp: VerifiablePresentation = presentation!
        presentation = nil
        
        return vp
    }
}

