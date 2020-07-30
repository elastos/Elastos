from cffi import FFI

ffibuilder = FFI()

with open("ela_did.ffi.h") as f:
    cdef = f.read()

with open("ela_jwt.ffi.h") as f:
    cdef += f.read()

ffibuilder.cdef(cdef)

ffibuilder.set_source("eladid",
"""
    #define DID_DYNAMIC
    #define DID_BUILD
    #include "ela_did.h"
    #include "ela_jwt.h"
""",
    libraries=['eladid', 'hdkey', 'cjson', 'curl', 'ssl', 'crypto', 'jansson', 'cjose', 'zip', 'z'],
    include_dirs=['include'],
    library_dirs=['lib'])

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
