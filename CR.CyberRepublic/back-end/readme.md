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


## testing

### Currently tests must be `runInBand`

## Migrations
### Suggestions
```
db.getCollection('suggestions').find({ abstract: { $exists: false } }).forEach(
  function(item) {
    db.suggestions.update(
      { "_id": item._id },
      {
        "$set": {
          "abstract": item.shortDesc,
          "goal": item.benefits,
          "motivation": item.desc,
          "budget": "{\"blocks\":[{\"key\":\"1c8g9\",\"text\":\"" + item.funding + "\",\"type\":\"unstyled\",\"depth\":0,\"inlineStyleRanges\":[],\"entityRanges\":[],\"data\":{}}],\"entityMap\":{}}"
        }
      }, false, true
    )
  }
)
```
