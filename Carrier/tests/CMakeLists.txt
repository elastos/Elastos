include(AddSubModule)

add_submodule(elatests
    DIRECTORY api
    DEPENDS ela-carrier ela-session CUnit libconfig)
