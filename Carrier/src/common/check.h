#include <assert.h>

#ifdef NDEBUG
#define CHECK(expr)                 do { if (!(expr)) return; } while(0)

#define CHECK_RETURN(expr, value)   do { if (!(expr)) return value; } while(0)

#define CHECK_GOTO(expr, label)     do { if (!(expr)) goto label; } while(0)
#else
#define CHECK(expr)                 do { \
                                        assert(expr); \
                                        if (!(expr)) \
                                            return; \
                                    } while(0)

#define CHECK_RETURN(expr, value)   do { \
                                        assert(expr); \
                                        if (!(expr)) \
                                            return value; \
                                    } while(0)

#define CHECK_GOTO(expr, label)     do { \
                                        assert(expr); \
                                        if (!(expr)) \
                                            goto label; \
                                    } while(0)
#endif
