# Code Structure

## folder description
* api_doc : swaagger api doc
* env : environment files
* src/db : mongodb schema and model (mongoose)
* src/router : rest api definition
* src/service : service layer
* src/utility : helper methods
* test : unit test

## how to add rest api
    1. design, definite the input and output with swagger.
    2. add a new router class under src/router, src/router/test is a sample to reference.
    3. router class could not include any logic code, just call one or more service to return the result.
    4. src/router/Base.ts is super class. every router class need to inherit.

## how to add service
    1. service layer is under src/service, there is a super class at src/service/Base.ts.
    2. src/service/TestService.ts is reference sample.
    3. design the DB schema and model under moogose at src/db, and service could get the db object to deal with any logic inside.

## to be more ...