# EBP Backend

## how to run
```
cd back-end
cp ./env/dev.sample ./env/dev.env
// change env file to fix your environment
npm i
npm start
```

## run with docker
```
# start
docker-compose up -d

# show log
docker logs -f ebp-backend

# stop
docker-compose down
```

## api doc via swagger
```
# api doc folder : /api_doc

# run swagger
docker-compose up -d
```
swagger doc is running on **http://localhost:9001**


## command
```
# unit test
npm test

# tslint validate
npm run tslint
```