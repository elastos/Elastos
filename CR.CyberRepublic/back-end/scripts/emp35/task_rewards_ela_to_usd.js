var conversionRate = 8;

db.tasks.find({ "reward.isUsd": false }).forEach(function (task) {
  		var ela = task.reward.ela / 1000
  		var usd = (ela * conversionRate) * 100

		db.tasks.update(
			{ "_id": task._id }, { $set:
		  	{
	  			"reward.usd": usd,
	  			"reward.isUsd": true,
		  	}
		})
		print('[' + task._id + '] converting ELA reward [' + ela + '] to USD value [' + usd + ']')
})

db.tasks.find({ "rewardUpfront.isUsd": false }).forEach(function (task) {
  		var ela = task.rewardUpfront.ela / 1000
  		var usd = (ela * conversionRate) * 100

		db.tasks.update(
			{ "_id": task._id }, { $set:
		  	{
	  			"rewardUpfront.usd": usd,
	  			"rewardUpfront.isUsd": true,
		  	}
		})
	    print('[' + task._id + '] converting ELA upfront reward [' + ela + '] to USD value [' + usd + ']')
})
