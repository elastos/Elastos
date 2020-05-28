#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <limits.h>
#include <CUnit/Basic.h>
#include <time.h>

#include "ela_did.h"
#include "ela_jwt.h"
#include "loader.h"
#include "constant.h"
#include "did.h"

static DIDDocument *doc;
static DIDStore *store;

static void get_time(time_t *date, int n)
{
    struct tm *tm = NULL;

    *date = time(NULL);
    tm = gmtime(date);
    tm->tm_year += n;
    *date = mktime(tm);
}

static void test_jwt(void)
{
    DID *did;
    DIDURL *keyid;
    JWTBuilder *builder;
    JWS *jws;
    time_t iat, nbf, exp;
    const char *token, *data;
    char idstring[ELA_MAX_DIDURL_LEN];
    int rc;

    did = DIDDocument_GetSubject(doc);
    CU_ASSERT_PTR_NOT_NULL(did);

    builder = DIDDocument_GetJwtBuilder(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "ctyp", "json"));
    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "library", "Elastos DID"));
    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "typ", "JWT"));
    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "version", "1.0"));

    iat = time(NULL);
    get_time(&nbf, -1);
    get_time(&exp, 2);

    const char *json = "{\"hello\": \"world\", \"test\": \"true\"}";

    CU_ASSERT_TRUE(JWTBuilder_SetSubject(builder, "JwtTest"));
    CU_ASSERT_TRUE(JWTBuilder_SetId(builder, "0"));
    CU_ASSERT_TRUE(JWTBuilder_SetAudience(builder, "Test cases"));
    CU_ASSERT_TRUE(JWTBuilder_SetIssuedAt(builder, iat));
    CU_ASSERT_TRUE(JWTBuilder_SetExpiration(builder, exp));
    CU_ASSERT_TRUE(JWTBuilder_SetNotBefore(builder, nbf));
    CU_ASSERT_TRUE(JWTBuilder_SetClaim(builder, "foo", "bar"));
    CU_ASSERT_TRUE(JWTBuilder_SetClaimWithJson(builder, "object", json));
    CU_ASSERT_TRUE(JWTBuilder_SetClaimWithBoolean(builder, "finished", false));

    token = JWTBuilder_Compact(builder);
    CU_ASSERT_PTR_NOT_NULL(token);

    jws = JWTParser_Parse(token);
    CU_ASSERT_PTR_NOT_NULL(jws);
    free((char*)token);

    CU_ASSERT_STRING_EQUAL("json", JWS_GetHeader(jws, "ctyp"));
    CU_ASSERT_STRING_EQUAL("Elastos DID", JWS_GetHeader(jws, "library"));
    CU_ASSERT_STRING_EQUAL("JWT", JWS_GetHeader(jws, "typ"));
    CU_ASSERT_STRING_EQUAL("1.0", JWS_GetHeader(jws, "version"));

    CU_ASSERT_STRING_EQUAL("JwtTest", JWS_GetSubject(jws));
    CU_ASSERT_STRING_EQUAL("0", JWS_GetId(jws));
    CU_ASSERT_STRING_EQUAL(DID_ToString(did, idstring, sizeof(idstring)), JWS_GetIssuer(jws));
    CU_ASSERT_STRING_EQUAL("Test cases", JWS_GetAudience(jws));
    CU_ASSERT_STRING_EQUAL("bar", JWS_GetClaim(jws, "foo"));

    data = JWS_GetClaimAsJson(jws, "object");
    CU_ASSERT_STRING_EQUAL(json, data);
    free((char*)data);
    CU_ASSERT_EQUAL(false, JWS_GetClaimAsBoolean(jws, "finished"));
    CU_ASSERT_EQUAL(iat, JWS_GetIssuedAt(jws));
    CU_ASSERT_EQUAL(nbf, JWS_GetNotBefore(jws));
    CU_ASSERT_EQUAL(exp, JWS_GetExpiration(jws));
    JWS_Destroy(jws);

    //reset jwt builder
    rc = JWTBuilder_Reset(builder);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    keyid = DIDURL_NewByDid(did, "key2");
    CU_ASSERT_PTR_NOT_NULL(keyid);

    rc = JWTBuilder_Sign(builder, keyid, storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    token = JWTBuilder_Compact(builder);
    CU_ASSERT_PTR_NOT_NULL(token);

    jws = JWTParser_Parse(token);
    CU_ASSERT_PTR_NOT_NULL(jws);
    free((char*)token);

    CU_ASSERT_PTR_NULL(JWS_GetHeader(jws, "ctyp"));
    CU_ASSERT_PTR_NULL(JWS_GetHeader(jws, "library"));
    CU_ASSERT_STRING_EQUAL(DIDURL_ToString(keyid, idstring, sizeof(idstring), false), JWS_GetKeyId(jws));

    CU_ASSERT_PTR_NULL(JWS_GetSubject(jws));
    CU_ASSERT_PTR_NULL(JWS_GetAudience(jws));

    DIDURL_Destroy(keyid);
    JWTBuilder_Destroy(builder);
    JWS_Destroy(jws);
}

static void test_jwt_compatible(void)
{
    JWS *jws;

    const char *token = "eyJ0eXAiOiJKV1QiLCJjdHkiOiJqc29uIiwibGlicmFyeSI6IkVsYXN0b3MgRElEIiwidmVyc2lvbiI6IjEuMCIsImFsZyI6Im5vbmUifQ.eyJzdWIiOiJKd3RUZXN0IiwianRpIjoiMCIsImF1ZCI6IlRlc3QgY2FzZXMiLCJpYXQiOjE1OTA1NjE1MDQsImV4cCI6MTU5ODUxMDMwNCwibmJmIjoxNTg3OTY5NTA0LCJmb28iOiJiYXIiLCJpc3MiOiJkaWQ6ZWxhc3RvczppV0ZBVVloVGEzNWMxZlBlM2lDSnZpaFpIeDZxdXVtbnltIn0.";
    jws = JWTParser_Parse(token);
    CU_ASSERT_PTR_NOT_NULL(jws);

    CU_ASSERT_STRING_EQUAL("1.0", JWS_GetHeader(jws, "version"));
    CU_ASSERT_STRING_EQUAL("Elastos DID", JWS_GetHeader(jws, "library"));

    CU_ASSERT_STRING_EQUAL("JwtTest", JWS_GetSubject(jws));
    CU_ASSERT_STRING_EQUAL("0", JWS_GetId(jws));
    CU_ASSERT_STRING_EQUAL("Test cases", JWS_GetAudience(jws));
    CU_ASSERT_STRING_EQUAL("bar", JWS_GetClaim(jws, "foo"));
}

static void test_jws(void)
{
    DID *did;
    DIDURL *keyid;
    JWTBuilder *builder;
    JWS *jws;
    time_t iat, nbf, exp;
    const char *token, *data;
    char idstring[ELA_MAX_DIDURL_LEN];
    int rc;

    did = DIDDocument_GetSubject(doc);
    CU_ASSERT_PTR_NOT_NULL(did);

    builder = DIDDocument_GetJwtBuilder(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "ctyp", "json"));
    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "library", "Elastos DID"));
    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "typ", "JWT"));
    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "version", "1.0"));

    iat = time(NULL);
    get_time(&nbf, -1);
    get_time(&exp, 2);

    const char *json = "{\"hello\": \"world\", \"test\": \"true\"}";

    CU_ASSERT_TRUE(JWTBuilder_SetSubject(builder, "JwtTest"));
    CU_ASSERT_TRUE(JWTBuilder_SetId(builder, "0"));
    CU_ASSERT_TRUE(JWTBuilder_SetAudience(builder, "Test cases"));
    CU_ASSERT_TRUE(JWTBuilder_SetIssuedAt(builder, iat));
    CU_ASSERT_TRUE(JWTBuilder_SetExpiration(builder, exp));
    CU_ASSERT_TRUE(JWTBuilder_SetNotBefore(builder, nbf));
    CU_ASSERT_TRUE(JWTBuilder_SetClaim(builder, "foo", "bar"));
    CU_ASSERT_TRUE(JWTBuilder_SetClaimWithJson(builder, "object", json));
    CU_ASSERT_TRUE(JWTBuilder_SetClaimWithBoolean(builder, "finished", false));

    keyid = DIDURL_NewByDid(did, "key2");
    CU_ASSERT_PTR_NOT_NULL(keyid);

    rc = JWTBuilder_Sign(builder, keyid, storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    token = JWTBuilder_Compact(builder);
    CU_ASSERT_PTR_NOT_NULL(token);

    jws = JWTParser_Parse(token);
    CU_ASSERT_PTR_NOT_NULL(jws);
    free((char*)token);

    CU_ASSERT_STRING_EQUAL("json", JWS_GetHeader(jws, "ctyp"));
    CU_ASSERT_STRING_EQUAL("Elastos DID", JWS_GetHeader(jws, "library"));
    CU_ASSERT_STRING_EQUAL("JWT", JWS_GetHeader(jws, "typ"));
    CU_ASSERT_STRING_EQUAL("1.0", JWS_GetHeader(jws, "version"));
    CU_ASSERT_STRING_EQUAL(DIDURL_ToString(keyid, idstring, sizeof(idstring), false), JWS_GetKeyId(jws));

    CU_ASSERT_STRING_EQUAL("JwtTest", JWS_GetSubject(jws));
    CU_ASSERT_STRING_EQUAL("0", JWS_GetId(jws));
    CU_ASSERT_STRING_EQUAL(DID_ToString(did, idstring, sizeof(idstring)), JWS_GetIssuer(jws));
    CU_ASSERT_STRING_EQUAL("Test cases", JWS_GetAudience(jws));
    CU_ASSERT_STRING_EQUAL("bar", JWS_GetClaim(jws, "foo"));

    data = JWS_GetClaimAsJson(jws, "object");
    CU_ASSERT_STRING_EQUAL(json, data);
    free((char*)data);
    CU_ASSERT_EQUAL(false, JWS_GetClaimAsBoolean(jws, "finished"));
    CU_ASSERT_EQUAL(iat, JWS_GetIssuedAt(jws));
    CU_ASSERT_EQUAL(nbf, JWS_GetNotBefore(jws));
    CU_ASSERT_EQUAL(exp, JWS_GetExpiration(jws));
    JWS_Destroy(jws);

    //reset jwt builder
    rc = JWTBuilder_Reset(builder);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = JWTBuilder_Sign(builder, keyid, storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    token = JWTBuilder_Compact(builder);
    CU_ASSERT_PTR_NOT_NULL(token);

    jws = JWTParser_Parse(token);
    CU_ASSERT_PTR_NOT_NULL(jws);
    free((char*)token);

    CU_ASSERT_PTR_NULL(JWS_GetHeader(jws, "ctyp"));
    CU_ASSERT_PTR_NULL(JWS_GetHeader(jws, "library"));
    CU_ASSERT_STRING_EQUAL(DIDURL_ToString(keyid, idstring, sizeof(idstring), false), JWS_GetKeyId(jws));

    CU_ASSERT_PTR_NULL(JWS_GetSubject(jws));
    CU_ASSERT_PTR_NULL(JWS_GetAudience(jws));

    DIDURL_Destroy(keyid);
    JWTBuilder_Destroy(builder);
    JWS_Destroy(jws);
}

static void test_jws_withdefaultkey(void)
{
    DID *did;
    DIDURL *keyid;
    JWTBuilder *builder;
    JWS *jws;
    time_t iat, nbf, exp;
    const char *token, *data;
    char idstring[ELA_MAX_DIDURL_LEN];
    int rc;

    did = DIDDocument_GetSubject(doc);
    CU_ASSERT_PTR_NOT_NULL(did);

    builder = DIDDocument_GetJwtBuilder(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "ctyp", "json"));
    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "library", "Elastos DID"));
    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "typ", "JWT"));
    CU_ASSERT_TRUE(JWTBuilder_SetHeader(builder, "version", "1.0"));

    iat = time(NULL);
    get_time(&nbf, -1);
    get_time(&exp, 2);

    CU_ASSERT_TRUE(JWTBuilder_SetSubject(builder, "JwtTest"));
    CU_ASSERT_TRUE(JWTBuilder_SetId(builder, "0"));
    CU_ASSERT_TRUE(JWTBuilder_SetAudience(builder, "Test cases"));
    CU_ASSERT_TRUE(JWTBuilder_SetIssuedAt(builder, iat));
    CU_ASSERT_TRUE(JWTBuilder_SetExpiration(builder, exp));
    CU_ASSERT_TRUE(JWTBuilder_SetNotBefore(builder, nbf));
    CU_ASSERT_TRUE(JWTBuilder_SetClaim(builder, "foo", "bar"));

    rc = JWTBuilder_Sign(builder, NULL, storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    token = JWTBuilder_Compact(builder);
    CU_ASSERT_PTR_NOT_NULL(token);
    JWTBuilder_Destroy(builder);

    jws = JWTParser_Parse(token);
    CU_ASSERT_PTR_NOT_NULL(jws);
    free((char*)token);

    CU_ASSERT_STRING_EQUAL("json", JWS_GetHeader(jws, "ctyp"));
    CU_ASSERT_STRING_EQUAL("Elastos DID", JWS_GetHeader(jws, "library"));
    CU_ASSERT_STRING_EQUAL("JWT", JWS_GetHeader(jws, "typ"));
    CU_ASSERT_STRING_EQUAL("1.0", JWS_GetHeader(jws, "version"));

    keyid = DIDURL_NewByDid(did, "primary");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    CU_ASSERT_STRING_EQUAL(DIDURL_ToString(keyid, idstring, sizeof(idstring), false), JWS_GetKeyId(jws));

    CU_ASSERT_STRING_EQUAL("JwtTest", JWS_GetSubject(jws));
    CU_ASSERT_STRING_EQUAL("0", JWS_GetId(jws));
    CU_ASSERT_STRING_EQUAL(DID_ToString(did, idstring, sizeof(idstring)), JWS_GetIssuer(jws));
    CU_ASSERT_STRING_EQUAL("Test cases", JWS_GetAudience(jws));
    CU_ASSERT_STRING_EQUAL("bar", JWS_GetClaim(jws, "foo"));
    CU_ASSERT_EQUAL(iat, JWS_GetIssuedAt(jws));
    CU_ASSERT_EQUAL(nbf, JWS_GetNotBefore(jws));
    CU_ASSERT_EQUAL(exp, JWS_GetExpiration(jws));

    DIDURL_Destroy(keyid);
    JWS_Destroy(jws);
}

static void test_jws_compatible_withdefaultkey(void)
{
    JWS *jws;

    const char *token = "eyJ0eXAiOiJKV1QiLCJjdHkiOiJqc29uIiwibGlicmFyeSI6IkVsYXN0b3MgRElEIiwidmVyc2lvbiI6IjEuMCIsImFsZyI6IkVTMjU2In0.eyJzdWIiOiJKd3RUZXN0IiwianRpIjoiMCIsImF1ZCI6IlRlc3QgY2FzZXMiLCJpYXQiOjE1OTA1Njk4NTEsImV4cCI6MTU5ODUxODY1MSwibmJmIjoxNTg3OTc3ODUxLCJmb28iOiJiYXIiLCJpc3MiOiJkaWQ6ZWxhc3RvczppV0ZBVVloVGEzNWMxZlBlM2lDSnZpaFpIeDZxdXVtbnltIn0.OJKyhS4MA4_VA24l2ZMYRywRpZj0QWkNyB--niL7qOrsJ5NlxDfItn0cW9SQF81xYi4rhTTnzujwY3qnIPnmnw";
    jws = JWTParser_Parse(token);
    CU_ASSERT_PTR_NOT_NULL(jws);

    CU_ASSERT_STRING_EQUAL("1.0", JWS_GetHeader(jws, "version"));
    CU_ASSERT_STRING_EQUAL("Elastos DID", JWS_GetHeader(jws, "library"));

    CU_ASSERT_STRING_EQUAL("JwtTest", JWS_GetSubject(jws));
    CU_ASSERT_STRING_EQUAL("0", JWS_GetId(jws));
    CU_ASSERT_STRING_EQUAL("Test cases", JWS_GetAudience(jws));
    CU_ASSERT_STRING_EQUAL("bar", JWS_GetClaim(jws, "foo"));
}

static void test_jws_compatible(void)
{
    JWS *jws;

    const char *token = "eyJ0eXAiOiJKV1QiLCJjdHkiOiJqc29uIiwibGlicmFyeSI6IkVsYXN0b3MgRElEIiwidmVyc2lvbiI6IjEuMCIsImtpZCI6IiNrZXkyIiwiYWxnIjoiRVMyNTYifQ.eyJpc3MiOiJkaWQ6ZWxhc3RvczppV0ZBVVloVGEzNWMxZlBlM2lDSnZpaFpIeDZxdXVtbnltIiwic3ViIjoiSnd0VGVzdCIsImp0aSI6IjAiLCJhdWQiOiJUZXN0IGNhc2VzIiwiaWF0IjoxNTkwNTY5OTM4LCJleHAiOjE1OTg1MTg3MzgsIm5iZiI6MTU4Nzk3NzkzOCwiZm9vIjoiYmFyIn0.Vx1d2Ua9eivcagpA4TbaB01PTa6S7MgdAZqHj3g2jx-65STR4gwPf5QoMmgRnUY0CWy36nz6tM0VyVO71XJRYA";
    jws = JWTParser_Parse(token);
    CU_ASSERT_PTR_NOT_NULL(jws);

    CU_ASSERT_STRING_EQUAL("1.0", JWS_GetHeader(jws, "version"));
    CU_ASSERT_STRING_EQUAL("Elastos DID", JWS_GetHeader(jws, "library"));

    CU_ASSERT_STRING_EQUAL("JwtTest", JWS_GetSubject(jws));
    CU_ASSERT_STRING_EQUAL("0", JWS_GetId(jws));
    CU_ASSERT_STRING_EQUAL("Test cases", JWS_GetAudience(jws));
    CU_ASSERT_STRING_EQUAL("bar", JWS_GetClaim(jws, "foo"));
}

static int jwt_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    int rc;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(false, storePath);
    if (!store)
        return -1;

    doc = TestData_LoadDoc();
    if (!doc) {
        TestData_Free();
        return -1;
    }

    return 0;
}

static int jwt_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_jwt",                             test_jwt                            },
    { "test_jwt_compatible",                  test_jwt_compatible                 },
    { "test_jws",                             test_jws                            },
    { "test_jws_withdefaultkey",              test_jws_withdefaultkey             },
    { "test_jws_compatible",                  test_jws_compatible                 },
    { "test_jws_compatible_withdefaultkey",   test_jws_compatible_withdefaultkey  },
    { NULL,                                    NULL                               }
};

static CU_SuiteInfo suite[] = {
    { "jwt test",  jwt_test_suite_init, jwt_test_suite_cleanup,  NULL, NULL, cases },
    {  NULL,       NULL,                NULL,                   NULL, NULL, NULL  }
};


CU_SuiteInfo* jwt_test_suite_info(void)
{
    return suite;
}
