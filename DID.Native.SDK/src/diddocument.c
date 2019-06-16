#include <stdio.h>
#include <openssl/opensslv.h>
#include <cjson/cJSON.h>

#include "ela_did.h"

//#define DID_MAX_LEN   512
//static char* did_descriper = "did:elastos:";

int DIDDocument_Create(char* output)
{
    cJSON* id_part, pk_part;
    cJSON* doc_root = cJSON_CreateObject();
    if(!doc_root)
        return -1;

    id_part = cJSON_AddStringToObject(doc_root, "id", get_DID(did, Fragement_NULL, 0));
    if(!json)
        goto error_exit;

    //todo:

    putout = cJSON_Print(doc_root);
    cJSON_Delete(doc_root);
    return 0;

error_exit:
    if(doc_root){
        cJSON_Delete(doc_root);
        doc_root = NULL;
    }

    return -1;
}

