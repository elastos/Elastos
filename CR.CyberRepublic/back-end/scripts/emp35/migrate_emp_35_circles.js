var campaignToName = {
 MARKETING: 'Marketing',
 PRODUCT_MANAGER: 'Product',
 LEGAL: 'Legal',
 DESIGNER: 'Design',
 MEDIA_PRODUCER: 'Media',
 WRITER: 'Writing',
 PARTNERSHIP: 'Partnership',
 INVESTMENTS: 'Investment',
 BUSINESS_DEVELOPMENT: 'Business Development',
 MEDIA: 'Media',
 WRITER_CONTENT: 'Writing',
 WRITER_TECHNICAL: 'Writing',
 LEAD_DEVELOPER_SUPPORT: 'Support',
 DAPP_ANALYST: 'dApp Analyst',
 ADMINISTRATOR: 'Administration',
 HR_DIRECTOR: 'HR',
 SECURITY: 'Security',
 OPEN_TITLE: null,
 LEAD_TRANSLATOR: 'Translation',
 DAPP_CONSULTANT: 'dApp Consultant',
 REGIONAL_EVANGELIST: null
}

db.submissions.find({type: 'EMPOWER_35'}).forEach(function (submission) {
 db.users.find({_id: submission.createdBy}).forEach(function (user) {
   if ((user.circles || []).length < 2 && campaignToName[submission.campaign]) {
      db.teams.find({type: 'CRCLE', name: campaignToName[submission.campaign]}).forEach(function (circle) {
         db.users.update({ _id: user._id }, { $push: { circles: circle._id } })
         
         db.user_teams.insert({
           status: 'NORMAL',
           role: 'MEMBER',
		   team: circle._id,
           user: user._id,
		   comments: []
         })
         
         var userTeam = db.user_teams.findOne({
           status: 'NORMAL',
           role: 'MEMBER',
		   team: circle._id,
           user: user._id,
         })
         
        db.teams.update({ _id: circle._id }, { $push: { members: userTeam._id } })
        print('[' + user.username + '] ' + submission.campaign + '-->' + circle.name + '-->' + userTeam._id)
	print('----------------------------------------')		
      })
   }
 })
})
