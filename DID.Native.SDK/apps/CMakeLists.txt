include(AddSubModule)

add_submodule(agent
    DIRECTORY agent
    DEPENDS ela-did)
add_submodule(resolver
    DIRECTORY resolver
    DEPENDS ela-did)
