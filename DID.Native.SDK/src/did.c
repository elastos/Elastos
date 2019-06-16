#include <stdio.h>
#include <openssl/opensslv.h>
#include <cjson/cJSON.h>

#include "ela_did.h"

#define DID_MAX_LEN   512
static const char* did_scheme = "did";
static const char* did_method = "elastos";
static const char* did_publickey_type = "ECDSAsecp256r1";

//todo: to get pk and sk
static char* pk;
static char* sk;

enum Fragment_Type{
    Fragement_NULL,
    Fragement_Default,
    Fragement_Master,
    Fragement_Second_Key,
    Fragement_Third_Key
};

static char* fragment_str[] =
{
    "",
    "#default",
    "#master-keys",
    "#key-2",
    "#key-3",
};

static char* get_did_scheme_method()
{
    static char scheme_method_string[DID_MAX_LEN];
    strcpy(scheme_method_string, did_scheme);
    strcat(scheme_method_string, ":");
    strcat(scheme_method_string, did_method);
    strcat(scheme_method_string, ":");

    return scheme_method_string;
}

//to remember free(output)
//default: if 1, need to write idstring; if 0, can't write idstring
static char* get_did_descriper(DID *did, int fragment_type, int is_default)
{
  staitc char did_descriper[DID_MAX_LEN] = "";

  if(is_default && !did)
    return NULL;

  if(is_default) {
    strcpy(did_descriper, get_did_scheme_method());
    strcat(did_descriper, did.sepecific_idstring);
  }

  if(fragment_type)
    strcat(did_descriper, fragment_str[fragment_type]);

  return did_descriper;
}

DID_API DID* DID_From_String(const char* idstring)
{
    static DID* did;

    if(!idstring)
        return NULL;

    did = malloc(sizeof(DID));
    if(!did)
        return NULL;

    did->sepecific_idstring = malloc(strlen(idstring) + 1);
    if(!did->sepecific_idstring)
        return NULL;

    strcpy(did->sepecific_idstring, idstring);
    did->id_fragment = NULL;

    did->document.did = did;
    did->pubulic_keys = malloc(sizeof(PublicKey));
    if(!did->pubulic_keys)
        return NULL;

    did->document.pubulic_keys.did = did;
    //todo: type is default?
    did->document.pubulic_keys.type = did_publickey_type;
    did->document.pubulic_keys.controller = get_did_descriper(did, Fragement_Master, 0);

    //todo: ECDAsecp256r1 makes pk and sk

    did->pubulic_keys.PublicKeyBase = pk;

    //todo: store sk
    did->authorization = NULL;
    did->credential = NULL;
    did->service = NULL;
    did->extime = NULL;

    return did;
}

DID_API DID* DID_From_Json(cJSon *json);

DID_API DIDDocument* DID_Resolve(DID *did, in update);

DID_API const char* DID_Get_Method(DID* did)
{
    if(!did)
        return NULL;

    return did_method;
}

DID_API const char* DID_Get_SpecificId(DID* did)
{
    if(!did)
        return NULL;

    return did->sepecific_idstring;
}

DID_API const char* DID_Get_DidString(DID* did)
{
    if(!id)
        return NULL;

    int len = strlen(get_did_scheme_method()) + strlen(did->sepecific_idstring) + 1；

    static char did_string[len];
    strcpy(did_string, get_did_scheme_method());
    strcat(did_string, did->sepecific_idstring);

    return did_string;
}

DID_API void DID_Close(DID* did)
{
    if(!did)
        return;

    if(did->sepecific_idstring) {
        free(did->sepecific_idstring);
        did->sepecific_idstring = NULL;
    }

    if(did->document.pubulic_keys) {
        free(did->document.pubulic_keys);
        did->document.pubulic_keys = NULL;
    }

    free(did);
    did = NULL;

    return;
}
