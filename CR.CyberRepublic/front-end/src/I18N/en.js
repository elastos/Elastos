import { TASK_STATUS, TASK_TYPE, CVOTE_STATUS } from '@/constant'
import council from './en/council'
import home from './en/home'
import release from './en/release'
import proposal from './en/proposal'
import suggestion from './en/suggestion'
import getting_started from './en/getting_started'
import whitepaper from './en/whitepaper'
import elip from './en/elip'
import area from './en/area'
import milestone from './en/milestone'

const en = {
  council,
  home,
  release,
  proposal,
  suggestion,
  getting_started,
  elip,
  whitepaper,
  area,
  milestone,
  // Header
  '0000': 'ALPHA',
  '0001': 'Bounty Programs',
  '0002': 'Community',
  '0003': 'Organizers',
  '0004': 'Account',
  '0005': 'Teams',
  '0006': 'Tasks',
  '0007': 'Help',
  '0008': 'About',
  '0009': 'FAQ',
  '0010': 'Contact',
  '0011': 'Forum',
  '0012': 'Home',

  '0100': 'Developers',
  '0101': 'Events',
  '0102': 'Community',
  '0104': 'My Republic',
  '0105': 'CR100',
  '0106': 'CRcles',
  '0107': 'Ambassadors',
  '0108': 'Council',
  '0109': 'Submissions',
  '0110': 'Blog',
  navigation: {
    council: {
      title: 'Council',
      submenu: {
        incumbent: 'Incumbent',
        candidate: 'Candidate',
      }
    },
    whitepaper: 'Whitepaper',
    suggestion: 'Suggestions',
    elips: 'ELIPs',
    proposal: 'Proposals',
    whatsNew: "What's New",
    resources: {
      title: 'Resources',
      submenu: {
        supernodes: 'Supernodes',
        news: 'News',
        forum: 'Forum',
        blog: 'Blog',
        docs: 'Docs'
      }
    },
    profile: 'My Republic'
  },

  '0200': 'Profile',
  '0201': 'Login/Register',
  '0202': 'Register',
  '0203': 'Admin',
  '0204': 'Logout',
  '0205': 'Login',

  '0300': 'Language',
  '0301': 'English',
  '0302': 'Chinese',

  'link.back': 'Back',

  // Admin breadcrumb
  1100: 'Admin',

  // Admin users
  1200: 'No',
  1201: 'Username',
  1202: 'Email',
  1203: 'Default language',
  1204: 'Role',
  1205: 'Active',
  1206: 'Ela',
  1207: 'Created Date',
  1208: 'Full Name',
  1209: 'Country',

  // Admin navigator
  1300: 'Tasks',
  1301: 'Community',
  1302: 'Users',
  1303: 'Teams',
  1304: 'Issues',
  1305: 'Forms',
  1306: 'Projects',
  'admin.suggestion': 'Suggestions',

  // Profile navigator
  2300: 'My Profile',
  2301: 'My Tasks',
  2302: 'My Teams',
  2303: 'My Issues',
  2304: 'My Communities',
  2305: 'My Projects',
  2306: 'Communities',
  'profile.suggestion': 'My Suggestions',

  'login.success': 'Welcome back',
  'logout.success': 'logout success',

  'mentions.notFound': 'User not found',

  'meta.postedBy': 'Posted By',
  'image.upload.type.error': 'The upload file is not an image.',
  'image.upload.size.error': 'The maximum upload file size is less than 500KB.',

  comments: 'Comments',
  'comments.posts': 'Posts',
  'comments.post': 'Post',
  'comments.noComments': 'No comments yet, ',
  'comments.signIn': 'sign in ',
  'comments.firstToPost': 'to be the first to post!',
  'comments.placeholder': 'Leave a comment',
  'comments.posted.success': 'Your comment has been posted.',
  'comments.updated.success': 'Your comment has been updated.',
  'comments.deleted.success': 'Your comment has been deleted.',
  'comments.delete.confirm': 'Delete this comment?',
  // Admin tasks
  'admin.tasks.status': 'Status',

  // Pop-up Announcement
  'popup.changes.title': 'Important CR Announcement',
  'popup.changes.2018-12-26.1': 'After in-depth discussions by the Council and Elastos members, we will be revising the website in the near future to better embody the concept of openness, transparency and community autonomy in Cyber Republic.',
  'popup.changes.2018-12-26.2': 'CR100/CRcles/Ambassadors on the CR website will be temporarily suspended (stop accepting new submissions and applications). For the ongoing community activities and CR100 projects, specific plans will be given in the next few weeks.',
  'popup.changes.2018-12-26.3': 'A new Cyber Republic forum will be launched Dec 31, 2018, we encourage everyone to use that to voice their questions and concerns. We will also invite everyone from Discord to use the forum, this is mainly due to Discord being inaccessible by the Chinese community.',
  'popup.changes.2018-12-26.4': 'You can also automatically login to the forum through the link in the top right:',
  'popup.changes.2018-12-26.5': 'The updated CR website will be relaunched on January 15, 2019, more information can be found at',
  'popup.changes.2018-12-26.blog_link': 'https://blog.cyberrepublic.org/2018/12/26/important-cyber-republic-announcement',
  
  'popup.suggestion.title': 'Important notice',
  'popup.suggestion.member': 'Dear CR members:',
  'popup.suggestion.content': 'The Cyber Republic Consensus will be launched at 10:00 am, June 10th, 2020, Beijing time. Accordingly the CR website will be upgraded recent days. The Suggestion function will be suspended and restarted when the new council is elected. Newly submitted suggestions will be handled by the new council.',
  'popup.suggestion.team': 'CR website developer team',

  // LoginForm
  'login.label_username': 'Please input your email address or username',
  'login.username': 'Email',
  'login.label_password': 'Please input your password',
  'login.password': 'Password',
  'login.logged': 'Remember me',
  'login.forget': 'Forgot password',
  'login.submit': 'Go',
  'login.reset': 'Reset password',
  'login.title': 'Login to Cyber Republic',
  'login.description_1': 'Input your credentials below.',
  'login.withDid': 'Log in with Elastos DID',
  'login.qrcodeTip': 'Open ELA wallet app and scan to log in.',

  // Logout
  'logout.title': 'Are you sure you want to logout?',

  // ApplyForm
  'apply.form.attachment': 'Supporting Attachment',
  'apply.form.suited': 'What Makes You Most Suited?',
  'apply.form.prompt': 'What Makes You Like To Apply?',

  // RegisterFrom
  'register.title': 'Become a Contributor',
  'register.description_1': 'This will only take a few seconds.',
  'register.description_2': 'As a member you can sign up for bounties on Cyber Republic.',
  'register.required': 'Required Fields',
  'register.error.code': 'The code you entered does not match',
  'register.error.passwords': 'Two passwords you entered do not match',
  'register.error.password_length_1': 'The password must be at least',
  'register.error.password_length_2': 'characters.',
  'register.form.input_code': 'Please input your code',
  'register.form.confirmation_code': 'Confirmation code',
  'register.form.label_first_name': 'Please input your first name',
  'register.form.first_name': 'First name',
  'register.form.label_last_name': 'Please input your last name',
  'register.form.last_name': 'Last name',
  'register.form.label_username': 'Please input your username',
  'register.error.username': 'Username must be more than 6 characters',
  'register.form.username': 'Username',
  'register.form.label_email': 'Please input your email',
  'register.error.email': 'Invalid email',
  'register.error.duplicate_email': 'This email is already taken',
  'register.form.email': 'Email',
  'register.form.label_password': 'Please input a Password',
  'register.form.password': 'Password',
  'register.form.label_password_confirm': 'Please input your password again',
  'register.form.password_confirm': 'Password confirm',
  'register.form.label_country': 'Please select your country',
  'register.form.option': 'Where are you from?',
  'register.form.about_section': 'Tell us a bit about yourself.',
  'register.form.organizer': 'Do you want to be an organizer?',
  'register.form.yes': 'Yes',
  'register.form.no': 'No',
  'register.form.developer': 'Are you a software developer or engineer?',
  'register.form.hear': 'Where did you hear about us?',
  'register.code': 'We have sent a confirmation code to ',
  'register.submit': 'Register',
  'register.welcome': 'Welcome to the Cyber Republic!',
  'register.join_circle': 'Join a CRcle and earn ELA',
  'register.join': 'Join',
  3533: 'More about you',
  3534: 'Country',
  'register.code.title': 'Become a citizen',

  // Forgot / Reset Password Form
  'forgot.title':
    'Forgot your password? Enter your email to be sent a reset password link.',
  'forgot.form.label_email': 'Please input a email',
  'forgot.form.email': 'Email',
  'forgot.form.submit': 'Reset password',
  'forgot.sent_email':
    'Success, if the email matches your user you should receive an email',
  'forgot.new_password': 'Please enter a new password',
  'forgot.success': 'Password changed successfully',

  // UserEditForm
  'user.edit.form.label_email': 'Email is required',
  'user.edit.form.label_role': 'Please select a role',
  'user.edit.form.role': 'Role',
  'user.edit.form.section.general': 'General',
  'user.edit.form.section.social': 'Social',
  'user.follow': 'Follow',
  'user.unfollow': 'Unfollow',

  // suggestion
  'suggestion.nodata': 'No Data',
  'suggestion.befirst': 'Be the first one to create a suggestion',
  'suggestion.subject': 'Subject',
  'suggestion.suggestion': 'Suggestion',
  'suggestion.referred': 'Referred in',
  'suggestion.title': 'Suggestions',
  'suggestion.add': 'Add a Suggestion',
  'suggestion.sort': 'Sort by',
  'suggestion.likes': 'Most Popular',
  'suggestion.views': 'Views',
  'suggestion.mostViews': 'Most Views',
  'suggestion.activeness': 'Trending',
  'suggestion.owner': 'Owner',
  'suggestion.new': 'New',
  'suggestion.postedBy': 'Posted By',
  'suggestion.viaCouncilMember': 'via Council member',
  'suggestion.follow': 'Follow',
  'suggestion.reportAbuse': 'Report Abuse',
  'suggestion.translate': 'Translate in: ',
  'suggestion.translate.en': 'English',
  'suggestion.translate.zh': '简体中文',
  'suggestion.translatedByGoogle': 'Translated by Google',
  'suggestion.councilMember': 'Council Member',
  'suggestion.copy': 'COPY',
  'suggestion.copied': 'Copied',
  'suggestion.back': 'Back',
  'suggestion.viewAll': 'View All',
  'suggestion.viewArchived': 'View Archived',
  'suggestion.mySuggestions': 'My Suggestions',
  'suggestion.all': 'All',
  'suggestion.addedByMe': 'Added by me',
  'suggestion.commentedByMe': 'Commented by me',
  'suggestion.subscribed': 'Followed',
  'suggestion.followed': 'Followed',
  'suggestion.summary': 'Summary',
  'suggestion.dislikes': 'Dislikes',
  'suggestion.comments': 'Comments',
  'suggestion.created': 'Created',
  'suggestion.archived': 'Archived',
  'suggestion.archive': 'Archive',
  'suggestion.unarchive': 'Unarchive',
  'suggestion.search': 'Search',
  'suggestion.cancel': 'Cancel',
  'suggestion.submit': 'Submit',
  'suggestion.header.edit': 'Edit Suggestion',
  'suggestion.editHistory': 'Version History',
  'suggestion.showEditHistory': 'Show Edit History',
  'suggestion.edited': 'Edited',
  'suggestion.form.mention.allCouncil': 'All Council Members',
  'suggestion.form.fields.suggestion': 'Suggestion',
  'suggestion.form.fields.subject': 'Subject',
  'suggestion.form.fields.desc': 'Description',
  'suggestion.form.fields.coverImg': 'Cover Image',
  'suggestion.form.fields.shortDesc': 'Short Description',
  'suggestion.form.fields.fullDesc': 'Full Description',
  'suggestion.form.fields.benefits': 'Benefits to Elastos Community/Ecosystem ',
  'suggestion.form.fields.funding': 'Funding Estimated (USD)',
  'suggestion.form.fields.timeline': 'Expected Completion',
  'suggestion.form.fields.links': 'Links',
  'suggestion.form.fields.linksSplit': 'Links (Split By Comma)',

  // social share
  'socialShare.wechat.desc': 'Scan QR code below using WeChat, then share this web page with your WeChat friends.',

  // Suggestion Button Text
  'suggestion.btnText.edit': 'Edit Suggestion',
  'suggestion.btnText.editDraft': 'Edit Draft',
  'suggestion.btnText.consider': 'Under Consideration',
  'suggestion.btnText.markConsider': 'Mark as Under Consideration',
  'suggestion.btnText.needMoreInfo': 'Needs More Info',
  'suggestion.btnText.makeIntoProposal': 'Make into Proposal',

  // suggestion error
  'suggestion.create.error.required': 'This field is required',
  'suggestion.create.error.tooShort': 'This field is too short',

  // suggestion rules
  'suggestion.rules': 'Rules',
  'suggestion.intro.1': 'Suggestions are the prelude to ',
  'suggestion.intro.1.proposals': 'Proposals',
  'suggestion.intro.1.1':
    ', the best or most popular ideas can be picked up by council members and turned into Proposals for voting consideration.',
  'suggestion.intro.2':
    'This process is known collectively as the Cyber Republic Consensus, for preliminary info see our',
  'suggestion.intro.2.blog': ' blog.',
  'suggestion.intro.3': 'For more information, please refer to: ',
  'suggestion.rules.guidelines': 'Guidelines',
  'suggestion.rules.guidelines.1':
    'You may suggest your company, team or self as the candidate for the proposal, this is at the discretion of the council member who picks up the suggestion',
  'suggestion.rules.guidelines.2':
    'Describe why you think this suggestion would benefit the Elastos ecosystem',
  'suggestion.rules.guidelines.3':
    'Describe a reasonable timeline for the proper execution of the suggestion',
  'suggestion.rules.rulesAndGuidelines': 'Rules and Guidelines',
  'suggestion.rules.guarantee':
    'There is no guarantee a suggestion will be selected by the council',
  'suggestion.rules.response':
    'Suggestions that are popular are more likely to receive a response by a council member.',
  'suggestion.rules.1':
    'Must be related to Cyber Republic as either a policy change, project or specific deliverable',
  'suggestion.rules.2':
    'Spamming suggestions will not be tolerated, please wait a reasonable time before modifying and resubmitting',
  'suggestion.rules.3':
    'If there is a deliverable, it must have an educated estimate of cost and expected return on investment',
  'suggestion.rules.infoRequest':
    'Requests for information from Elastos Foundation or specific CR members are not permitted and outside of the scope of the Cyber Republic Consensus program, for these matters please contact the Community Manager.',

  // Roles & Permissions
  'permission.title': 'Roles & Permissions',

  // Developer
  'developer.breadcrumb.developers': 'Community',
  'developer.breadcrumb.search': 'Search',
  'developer.learn': 'Learn',
  'developer.learn.basics': 'Elastos Basics',
  'developer.learn.concepts': 'Key Concepts',
  'developer.learn.start': 'Getting Started',
  'developer.learn.tutorials': 'Tutorials',
  'developer.project.title': 'Projects',
  'developer.tasks.title': 'Tasks',
  'developer.learn.description': 'Tutorials, Resources and more',
  'developer.teams.title': 'Teams',
  'developer.teams.description': 'Connect, Form Teams and work on projects',
  'developer.projects.description': 'Overview of Cyber Republic Projects',
  'developer.tasks.description': 'Overview of Cyber Republic Tasks',
  'developer.learn.resources': 'Training, webinars, bootcamps',
  'developer.team': 'Team Search',
  'developer.team.join': 'Join a team looking for your skills',
  'developer.team.create_profile': 'Create a profile',
  'developer.team.create_project': 'Create a project',
  'developer.project': 'Project Search',
  'developer.project.top': 'Top 100 projects',
  'developer.project.join': 'Join a project that is in active development',
  'developer.project.issue': 'Submit an issue',
  'developer.task': 'Task Search',
  'developer.task.top': 'Top tasks',
  'developer.task.join': 'Join a task that is in active development',
  'developer.task.issue': 'Submit an issue',
  'developer.action': 'Is Elastos suited to my project?',
  'developer.components': 'Elastos Core Components',
  'developer.components.core.title': 'Elastos RunTime (RT)',
  'developer.components.core.languages': 'C++',
  'developer.components.core.description':
    'lots of information about Elastos Run Time',
  'developer.components.spv': 'Elastos SPV (SPV)',
  'developer.components.spv.languages': 'C++, Javascript, Go',
  'developer.components.spv.description':
    'Payment module with full feature SDK',
  'developer.components.issues': 'Issues',
  'developer.components.docs': 'Docs',
  'developer.components.info': 'More Info',
  'developer.cr100.application.success': 'Application Successful',
  'developer.cr100.application.view': 'View',
  'developer.cr100.projects': 'Projects',
  'developer.cr100.submit_whitepaper': 'Submit Whitepaper',
  'developer.cr100.welcome.1':
    'This program is temporarily suspended and will soon become a group of special proposals. When the new Cyber Republic BETA launches each team will have priority access to the new council members to pitch their proposals.',
  'developer.cr100.welcome.2':
    'We still value every person and team that has already submitted a proposal during the ALPHA, and deeply regret the growing pains that everyone has endured. Every effort will be made by the new CR secretariat team to contact and ensure they are responded to individually.\n',
  'developer.cr100.welcome.3':
    'Feel free to join us on the new forums launching Dec 31, 2018 to bring up your proposals and discuss the future of CR. ',
  'developer.cr100.welcome.title': 'Cyber Republic 100',
  'developer.cr100.disclaimer':
    'Projects described above are subject to change and are only used as an example.',
  'developer.cr100.disclaimer.title': 'Disclaimer',
  'developer.cr100.dontseeProject': 'Submit Your Own Idea/Proposal',
  'developer.cr100.dontseeProject.title': "Don't see a project you like?",
  'developer.cr100.pitch.problem': 'Problem',
  'developer.cr100.pitch.valueProposition': 'Value Proposition',
  'developer.cr100.pitch.useCase': 'Use Case',
  'developer.cr100.pitch.beneficiaries': 'Beneficiaries',
  'developer.cr100.pitch.elaInfrastructure': 'Elastos Infrastructure',

  'developer.search.assignment': 'Assignment',
  'developer.search.assignment.all': 'All',
  'developer.search.assignment.unassigned': 'Unassigned',
  'developer.search.event': 'Event',
  'developer.search.sort': 'Sort',
  'developer.search.sort.createdAt': 'Created',
  'developer.search.sort.updatedAt': 'Updated',
  'developer.search.sort.asc': 'Ascending',
  'developer.search.sort.desc': 'Descending',
  'developer.search.team': 'Team',
  'developer.search.project': 'Project',
  'developer.search.task': 'Task',
  'developer.search.taskCategory': 'Task category',
  'developer.search.category': 'Category',
  'developer.search.category.social': 'Social',
  'developer.search.category.iot': 'IoT',
  'developer.search.category.media': 'Media',
  'developer.search.category.finance': 'Finance',
  'developer.search.circle': 'Circle',
  'developer.search.lookingFor': 'Looking for',
  'developer.search.search.placeholder': 'Search',
  'developer.search.skillset': 'Skillset',
  'developer.search.showMore': 'Show More…',
  'developer.search.hide': 'Hide',
  'developer.search.apply': 'Apply',
  'developer.search.view': 'View',
  'developer.search.subtitle_prefix': 'No longer accepting',
  'developer.search.subtitle_bids': 'bids',
  'developer.search.subtitle_applications': 'applications',
  'developer.search.submit_bid': 'Submit Bid',
  'developer.form.submission.type.required': 'Please select a type',
  'developer.form.submission.type.option.bug': 'Bug',
  'developer.form.submission.type.option.security': 'Security Issue',
  'developer.form.submission.type.option.suggestion': 'Suggestion',
  'developer.form.submission.type.option.other': 'Other',
  'developer.form.submission.title.required': 'Please put in some title',
  'developer.form.submission.description.required': 'Please put in some title',
  'developer.form.submission.placeholder.subject': 'Subject',
  'developer.form.submission.placeholder.description': 'Describe your issue',
  'developer.form.submission.message.success':
    'Your issue has been submitted. Thanks!',
  'developer.member.search.title': 'Member Search',
  'developer.member.table.column.member': 'Member',
  'developer.member.table.column.username': 'Username',
  'developer.member.table.column.circles': 'Circles',

  'myrepublic.teams': 'Teams',
  'myrepublic.teams.all': 'All',
  'myrepublic.teams.owned': 'Owned',
  'myrepublic.teams.active': 'Active',
  'myrepublic.teams.applied': 'Applied',
  'myrepublic.teams.rejected': 'Rejected',
  'myrepublic.teams.create': 'Create Team',

  'myrepublic.projects': 'Projects',
  'myrepublic.projects.all': 'All',
  'myrepublic.projects.owned': 'Owned',
  'myrepublic.projects.active': 'Active',
  'myrepublic.projects.applied': 'Applied',
  'myrepublic.projects.subscribed': 'Subscribed',
  'myrepublic.projects.liked': 'Liked',
  'myrepublic.projects.cr100': 'CR100',
  'myrepublic.projects.create': 'Create Project',
  'myrepublic.projects.create.cr100': 'Create CR100',

  // Empower 35
  'emp35.header.title.part1': 'CR',
  'emp35.header.title.part2': 'cles',
  'emp35.header.title.part3': ' & Empower35',
  'emp35.header.content.1':
    'Empower 35 which has recently been expanded to the CRcle program will be temporarily suspended. The current vision is that skilled members of our community will be encouraged to form and lead teams. For certain roles that the Secretariat team requires, qualified candidates will be promoted to join as an adviser. More details will follow on how community members can advance to more permanent positions in Cyber Republic.',
  'emp35.header.content.2': '',
  'emp35.header.content.3': '',

  'emp35.empower.title': 'Empower 35',
  'emp35.empower.content':
    'You can join up to two CRcles. Once inside, form even smaller teams, create tasks for approval and begin showcasing your talents. CRcles are smart networks within the larger smart network of Cyber Republic. As each CRcle expands, the entire network expands with it.',
  'emp35.teamHeader.title': 'CRcles',

  'emp35.mycircles.title': 'My CRcles',
  'emp35.circles.statement': '',
  'emp35.form.reason.max': 'Reason too long',
  'emp35.form.field.required': 'This must be filled out',
  'emp35.form.suitedreason.max': 'Suited reason too long',
  'emp35.form.field.upload': 'Upload',
  'emp35.form.funding': 'Funding: US $100K Annual in ELA',
  'emp35.form.apply.text': 'Why Would You Like to Apply?',
  'emp35.form.most.suited': 'What Makes You Most Suited?',
  'emp35.form.support': 'Supporting Attachment',

  // Not found
  'error.notfound': '404, not found',

  // Form ext
  'formext.anni2018.app.title': 'Elastos 2018 Anniversary Event Registration',
  'formext.anni2018.video.title':
    'Elastos 2018 Anniversary Event - Community Video',
  'formext.anni2018.organizer.title': 'Organizer Application',
  'formext.anni2018.training.title': 'Evangelist Training Application',

  // Circle Detail
  'circle.title': 'Crcle Detail',
  'circle.header.join': 'Join',
  'circle.header.leave': 'Leave',
  'circle.header.maxReached': 'You can only join 2 CRcles',
  'circle.createPost': 'Create Post',
  'circle.joinToPost': 'Join the CRcle to post',
  'circle.registerToPost': 'Register to join the CRcle and post',
  'circle.members': 'Members',
  'circle.tasks': 'Tasks',
  'circle.posts': 'Posts',
  'circle.uploadtext': 'If you have a whitepaper ready, drag it or click here.',
  'circle.uploadhint':
    'Use the Comments below to speak to the Project Owner and find out more.',

  // Training
  'training.header.title': 'Ambassadors Training',
  'training.subscribeemail.text':
    'Stay updated by subscribing below with your E-Mail',
  'training.header.content.1':
    'This program will be postponed until after a developer hackathon that is',
  'training.header.content.2':
    ' in the early planning phases. We value everyone’s hard work hosting and',
  'training.header.content.3':
    'organizing events around the globe and will still engage existing',
  'training.header.content.4':
    'ambassadors individually. Stay tuned for more news soon, especially as',
  'training.header.content.5': 'we near the 2nd Elastos Anniversary event.',
  'training.evangelist.title': 'Ambassadors Training',
  'training.evangelist.content':
    'The goal of the program is for participants to not only learn the technology from the Elastos Team first hand, but to develop a presentation crafted with marketing and PR professionals that will allow ambassadors to be professionally prepared to represent the project to the public. Cyber Republic plans to run this program several times per year. ',
  'training.thanksForApplying':
    'Thank you for applying, we will be in touch shortly!',
  'training.applyError': 'An error has occured. Please contact us directly.',
  'training.apply.title': 'Apply to be an ambassador?',

  // Itinerary
  'training.itinerary.title': 'Itinerary Example',
  'training.itinerary.content.venueLabel': 'Venue',
  'training.itinerary.content.venue': 'Houses in Silicon Valley',
  'training.itinerary.content.day13Label': 'Day 1-4',
  'training.itinerary.content.day13': 'Learning Elastos Vision and Tech',
  'training.itinerary.content.day4Label': 'Day 5',
  'training.itinerary.content.day4': 'Public Speaking Skills, Q&A training',
  'training.itinerary.content.day5Label': 'Day 6',
  'training.itinerary.content.day5': 'TBD',
  'training.itinerary.content.day6Label': 'Day 7',
  'training.itinerary.content.day6': 'Pitch presentation practice',
  'training.disclaimer': 'Training content and itinerary subject to change',

  // Project/Team detail
  'project.detail.bid_selected': 'Winning bid has been selected',
  'project.detail.app_selected': 'Winning application already selected',
  'project.detail.deadline': 'Application Deadline',
  'project.detail.completion_deadline': 'Completion Deadline',
  'project.detail.progress': 'Progress',
  'project.detail.team_size': 'Team Size',
  'project.detail.recruiting': 'Desired Skills',
  'project.detail.not_recruiting': 'Recruitment Closed',
  'project.detail.recruiting_skills_unknown': 'Not Specified',
  'project.detail.current_contributors': 'Current Assignee',
  'project.detail.owner': 'Project Owner',
  'project.detail.pending_applications': 'Pending Applications',
  'project.detail.pending_bids': 'Pending Bids',
  'project.detail.your_bids': 'Your Bids',
  'project.detail.your_bid': 'Your Bid',
  'project.detail.total_bids': 'Total Bids',
  'project.detail.subscribers': 'Liked By',
  'project.detail.current_members': 'Current Members',
  'project.detail.leave': 'Withdraw Application',
  'project.detail.you_bid': 'You bid',
  'project.detail.leave_bid': 'Withdraw Bid',
  'project.detail.view': 'View',
  'project.detail.approve': 'Approve',
  'project.detail.remove': 'Remove',
  'project.detail.disapprove': 'Disapprove',
  'project.detail.withdraw_application': 'Withdraw',
  'project.detail.popup.leave_question': 'Are you sure you want to leave?',
  'project.detail.popup.leave_team': 'Leave Team',
  'project.detail.popup.applied': 'Applied',
  'project.detail.popup.join_team': 'Join Team',
  'project.detail.popup.join_project': 'Apply for Project',
  'project.detail.popup.join_task': 'Apply for Task',
  'project.detail.popup.bid_project': 'Bid on Project',
  'project.detail.popup.bid_task': 'Bid on Task',
  'project.detail.bidding': 'Bidding Open',
  'project.detail.bidding_closed': 'Bidding Closed',
  'project.detail.tell_us_why_join': 'Tell us why you want to join.',
  'project.detail.tell_us_why_bid': 'Tell us why you want to bid.',
  'project.detail.no_attachments': 'No attachments',
  'project.detail.bid_updated': 'Bid updated',
  'project.detail.bidding_winner': 'Bidding Winner',
  'project.detail.bidding_cur_1': 'There are currently',
  'project.detail.bidding_cur_2': 'other bids',
  'project.detail.budget': 'Budget',
  'project.detail.reward': 'Reward',
  'project.detail.reference_bid': 'Reference Bid',
  'project.detail.noapplications': 'There are no applications yet',

  'project.detail.columns.name': 'Name',
  'project.detail.columns.action': 'Action',

  'project.detail.comments_disabled':
    'Comments are disabled for closed bidding projects/tasks',
  'project.detail.statusHelp.approvedBy': 'Approved by',

  'project.admin.statusHelp.created': 'this task does not require approval',
  'project.admin.statusHelp.pending': 'this task is awaiting approval',
  'project.admin.statusHelp.successReward':
    'this task is awaiting ELA disbursement',
  'project.admin.statusHelp.successNoReward':
    'this task does not require ELA, no further action is needed',
  'project.admin.statusHelp.approvedBy': 'this task is approved by',
  'project.admin.statusHelp.approvedOn': 'on',

  'project.public.statusHelp.pending':
    'this task is awaiting approval by an admin',
  'project.public.statusHelp.submitted':
    'this task is awaiting council sign off',
  'project.public.statusHelp.success':
    'an admin will review and disburse the ELA reward if any',

  'project.public.statusHelp.markAsComplete': 'Mark as Complete',
  'project.public.statusHelp.markAsCompleteConfirm':
    'Are you sure you want to mark this task as complete?',

  'team.owner': 'Leader',
  'team.description': 'Description',
  'team.applyMessage': 'Apply',
  'team.detail.view': 'View',

  'team.detail.team_active': 'The team is currently active',

  'team.create.error.nameRequired': 'Team name is required',
  'team.create.error.descriptionRequired': 'Team description is required',
  'team.create.error.nameTooShort': 'Team name is too short',
  'team.create.error.descriptionTooShort': 'Team description is too short',

  // Team specializations
  'team.spec.media': 'Media',
  'team.spec.iot': 'IoT',
  'team.spec.authenticity': 'Authenticity',
  'team.spec.currency': 'Currency',
  'team.spec.gaming': 'Gaming',
  'team.spec.finance': 'Finance',
  'team.spec.sovereignty': 'Sovereignty',
  'team.spec.social': 'Social',
  'team.spec.exchange': 'Exchange',

  // Skillsets
  'team.skillset.cpp': 'C++',
  'team.skillset.javascript': 'JavaScript',
  'team.skillset.go': 'Go',
  'team.skillset.python': 'Python',
  'team.skillset.java': 'Java',
  'team.skillset.swift': 'Swift',

  'task.owner': 'Owner',
  'task.circle': 'Circle',
  'task.type': 'Type',
  'task.description': 'Description',
  'task.category': 'Category',
  'task.applyDeadline': 'Apply deadline',
  'task.completionDeadline': 'Deadline',
  'task.applyMessage': 'See full task and apply',
  'task.referenceBid': 'Reference bid',
  'task.referenceBid.none': 'Bidding Open',
  'task.budget': 'Budget',
  'task.reward': 'Reward',
  'task.infoLink': 'Further info',
  'task.goals': 'Goals',
  'task.eventStart': 'Event start',
  'task.eventEnd': 'Event end',
  'task.descBreakdown': 'Budget breakdown',
  'task.location': 'Event location',
  'task.community': 'Community',
  'task.attachment': 'Attachment',
  'task.budget.explain': 'Budget is for expenses/costs',
  'task.reward.explain': 'Reward is for labor and time',
  'task.bid': 'Bid',
  'task.approvedBy': 'Approved By',
  'task.application': 'Application',
  'task.appliedOn': 'Applied on',
  'task.soloApply': 'Apply as an Individual',
  'task.teamApply': 'Apply as a Team',
  'task.applyReason': 'Why you wanted to join this task',
  'task.createNew': 'Propose New Task',

  // General
  'select.placeholder': 'Please select',
  '.ok': 'Ok',
  '.apply': 'Apply',
  '.cancel': 'Cancel',
  '.delete': 'Delete',
  '.status': 'Status',
  '.edit': 'Edit',
  '.upload': 'Click to Upload',
  '.yes': 'Yes',
  '.no': 'No',
  '.loading': 'Loading...',
  ela: 'ELA',
  '.areYouSure': 'Are you sure?',
  '.suspended': 'Suspended',

  // Temp Notices
  'notice.suspended': 'Further plans will be given in the next few weeks',

  // Community
  'community.nomember': 'no members',
  'community.buton.join': 'Join',
  'community.buton.leave': 'Leave',
  'community.noorganizers': 'No organizers yet',
  'community.applytobeorganizer': 'Apply to be an Organizer',
  'community.message.success.joincommunity': 'You were added to community',
  'community.message.success.leavecommunity':
    'You left this community successfully',
  'community.message.error.joincommunity':
    'Error while adding you to community',
  'community.message.error.apply': 'You must be logged in to apply',
  'community.selectcountry': 'Select a country',
  'community.guidecontainer.part1':
    'Hello there! Looks like your we do not have an organizer for',
  'community.guidecontainer.part2':
    'We are always looking for new organizers especially in new communities.',
  'community.guidecontainer.part3':
    "if you'd like to be an organizer for your region please register, we'll add your country and you can then apply to be an organizer on this page.",
  'community.button.register': 'Click to Register',
  'community.button.selectcountry': 'or you can select a country from above',
  'community.link.toevent': 'See Event',

  // Council
  'council.list': 'List',
  'council.voting': 'Voting',
  'council.list.proposals': 'Any suggestions, proposals can be sent to',
  'council.voting.proposal': 'Proposal',
  'council.voting.referred': 'Referred to',
  'council.voting.proposalList': 'Proposals',
  'council.voting.number': 'No.',
  'council.voting.published': 'Published',
  'council.voting.title': 'Title',
  'council.voting.type': 'Type',
  'council.voting.author': 'Author',
  'council.voting.voteBy': 'Vote By',
  'council.voting.voteByCouncil': 'Votes by Council Members',
  'council.voting.councilMembersVotes': 'Council Members Votes',
  'council.voting.votingEndsIn': 'Voting Ends In',
  'council.voting.votingEndsIn.ended': 'Ended',
  'council.voting.votingEndsIn.day': 'Day',
  'council.voting.votingEndsIn.days': 'Days',
  'council.voting.status': 'Status',
  'council.voting.createdAt': 'Created',
  'council.voting.proposedAt': 'Proposed',
  'council.voting.chainStatus.chained':'Chained',
  'council.voting.chainStatus.chaining':'Chaining',
  'council.voting.chainStatus.unchain':'UnChain',
  'council.voting.chainStatus.failed':'Failed',


  'council.voting.type.newMotion': 'New Motion',
  'council.voting.type.motionAgainst': 'Motion Against',
  'council.voting.type.anythingElse': 'Anything Else',
  'council.voting.type.standardTrack': 'Standards Tracking ELIP',
  'council.voting.type.information': 'Information ELIP',
  'council.voting.type.process': 'Process ELIP',
  'council.voting.type.support': 'Yes',
  'council.voting.type.reject': 'No',
  'council.voting.type.abstention': 'Abstained',
  'council.voting.type.undecided': 'Undecided',
  'council.voting.ifConflicted':
    'Potential Conflict with Existing Constitution',

  'council.voting.btnText.yes': 'Vote Yes',
  'council.voting.btnText.no': 'Oppose with a Reason',
  'council.voting.btnText.abstention': 'Abstain',
  'council.voting.btnText.notesSecretary': 'Notes from Secretary',
  'council.voting.btnText.editNotes': 'Edit Notes',
  'council.voting.btnText.editProposal': 'Edit Proposal',
  'council.voting.btnText.completeProposal': 'Complete Proposal',
  'council.voting.btnText.closeIncomplete': 'Close with Incompletion',
  'council.voting.btnText.publish': 'Publish',
  'council.voting.btnText.delete': 'Delete',

  'council.voting.modal.deleteDraft':
    'Are you sure to delete this draft proposal?',
  'council.voting.modal.complete': 'Are you sure to complete this proposal?',
  'council.voting.modal.incomplete': 'Are you sure to close with incompletion?',
  'council.voting.modal.updateNotes': 'Notes from Secretary',
  'council.voting.modal.voteYes': 'Are you sure to Vote Yes?',
  'council.voting.modal.voteAbstain': 'Are you sure to Abstain?',
  'council.voting.modal.voteReason': 'Reason',
  'council.voting.modal.confirm': 'Submit',
  'council.voting.modal.cancel': 'Cancel',

  'council.voting.voteResult.yes': 'Vote Yes',
  'council.voting.voteResult.opposed': 'Opposed',
  'council.voting.voteResult.abstention': 'Abstained',

  'council.voting.voteResult.show': 'Show',
  'council.voting.voteResult.all': 'All',
  'council.voting.voteResult.unvoted': 'Unvoted by me',

  'council.voting.voteResult.onchain':'Vote onchain',
  
  // Landing
  'landing.cr': 'Cyber Republic',
  'landing.footer.note': 'Stay up to date with Cyber Republic',
  'landing.footer.email': 'Enter Email',
  'landing.footer.resources': 'Resources',
  'landing.footer.wallet': 'Wallet',
  'landing.footer.explorer': 'Block Explorer',
  'landing.footer.github': 'Github',
  'landing.footer.assets': 'Logo Assets',
  'landing.footer.elaNews': 'ELA News',
  'landing.footer.contact': 'Contact',
  'landing.footer.community': 'Global Community',
  'landing.footer.support': 'Support',
  'landing.footer.contacts': 'Other',
  'landing.footer.join': 'Join Us On',
  'landing.footer.privacyPolicy': 'Privacy Policy',
  'landing.footer.termsAndConditions': 'Terms & Conditions',

  // Project Detail
  'pdetail.like': 'Like',
  'pdetail.unlike': 'Unlike',
  'pdetail.involve': 'Apply',
  'pdetail.funding': 'Funding: 100k for 5% of the equity or coins/tokens',
  'pdetail.title': 'Project Detail',

  // Our Vision
  'vision.00': 'Our Vision',
  'vision.01':
    'The vision for Cyber Republic starts with an ambitious idea: create a self-running and self-governed community of entrepreneurs and developers who can function independently of Elastos but with the unified goal to grow it into a global success.',
  'vision.02':
    'This new website is being launched in its Alpha version. The basic functions of finding tasks and earning ELA remain, but the addition of our CR100 and Empower35 projects are still in their early stages and are meant to not only represent our plan for long term success but are presented as programs that need the community and future leaders of Cyber Republic to help craft and shape the mechanics, workflow, and economics of these projects.',
  'vision.03':
    'The truth is, Rong Chen and the Elastos Foundation have unselfishly given a large responsibility and power to the international community to determine the direction of the project. While Cyber Republic is certainly part of Elastos, the future governance and economic model of this project will be self-organized by the members of Cyber Republic, not The Elastos Foundation. Cyber Republic will in many ways become completely independent of the Foundation and control its own destiny. This vision, however, will take time to achieve.',
  'vision.04':
    'In the meantime, we have assembled a Cyber Republic Council to make the important decisions as this project moves in the direction of full democratic decentralization. This Council, made up of members Feng Zhang, Kevin Zhang, and Yipeng Su, will utilize community input and make strategic decisions to grow the project. In time, the members of the Empower35 project along with other leaders in Cyber Republic will form a government and become self-running. Our Constitution is currently in progress.',
  'vision.05':
    'The Elastos vision to change the internet runs parallel to Cyber Republic’s vision to change the way people organize and build a global project. This Alpha version is the template and the framework for our goal to move towards a completely modern and unique online republic. Your input and active participation are necessary for this vision to continue to take shape.',
  'vision.06':
    'More information and more specifics on how to apply for individual projects are to come.',
  'vision.07':
    'We look forward to building an international haven for entrepreneurship  and innovation for the new internet.',

  // Role
  'role.member': 'User',
  'role.organizer': 'Organizer',
  'role.admin': 'Admin',
  'role.council': 'Council',
  'role.secretary': 'Secretary',
  'role.custom': 'Custom',
  'role.admin.mode': 'Admin Mode',

  // Profile
  'profile.associateDid': 'Associate DID',
  'profile.qrcodeTip': 'Open ELA wallet and scan',
  'profile.reassociateDid': 'Re-associate DID',
  'profile.thanksForCompleting': 'Thanks for updating your profile!',
  'profile.skillsets': 'My Skillsets',
  'profile.completeProfile': 'Complete your Profile',
  'profile.completeProfile.explanation':
    'Get more tasks and connect with talent all over the world',
  'profile.editProfile': 'Edit Profile',
  'profile.editBasicProfile': 'Edit Basic Profile',
  'profile.editFullProfile': 'Edit Full Profile',
  'profile.editProfile.section.1': 'Basic Information',
  'profile.editProfile.section.2': 'Skill Set',
  'profile.editProfile.section.3': 'Social Profile',
  'profile.portfolio.placeholder': 'Relevant for Designers',
  'profile.portfolio.github': 'Relevant for Developers',
  'profile.profession.ENGINEERING': 'Engineering',
  'profile.profession.COMPUTER_SCIENCE': 'Computer Science',
  'profile.profession.PRODUCT_MANAGEMENT': 'Product Management',
  'profile.profession.ART_DESIGN': 'Art / Design',
  'profile.profession.SALES': 'Sales',
  'profile.profession.MARKETING': 'Marketing',
  'profile.profession.BUSINESS_FINANCE': 'Business / Finance',
  'profile.profession.ENTREPRENEUR': 'Entrepreneur',
  'profile.profession.STUDENT': 'Student',
  'profile.profession.HEALTH_MEDICINE': 'Health/Medicine',
  'profile.profession.LITERATURE_WRITING': 'Literature/Writing',
  'profile.profession.TRANSLATION': 'Translation/Interpretation',
  'profile.profession.LAW': 'Law',
  'profile.profession.ECONOMICS': 'Economics',
  'profile.profession.MANAGEMENT': 'Management',
  'profile.profession.OTHER': 'Other',
  'profile.complete': 'Complete Your Profile',
  'profile.complete.dismiss': 'Dismiss',
  'profile.skillsDetails.placeholder':
    'Explain more about your skills, work experience, etc.',
  'profile.motto.placeholder': 'Do you have a life motto?',
  'profile.previous': 'Previous',
  'profile.next': 'Next',
  'profile.localTime': 'Local Time',
  'profile.sendMessage': 'Send Direct Message',
  'profile.viewProfile': 'View Profile',
  'profile.showPublicProfile': 'Public Profile',
  'profile.crContributors': 'CR Contributors',
  'profile.followers': 'Followers',
  'profile.edit': 'Edit',
  'profile.publicProfile': 'Public Profile',
  'profile.save': 'Save',
  'profile.projectsTasks': 'Projects/Tasks',
  'profile.view': 'View',
  'profile.community.leave.success': 'You left community successfully',
  'profile.community.table.name': 'Name',
  'profile.community.table.geolocation': 'Geolocation',
  'profile.community.table.type': 'Type',
  'profile.community.table.actions': 'Actions',
  'profile.community.title': 'Communities',
  'profile.community.joincommunity': 'Joined Communities',
  'profile.info.title': 'Info',
  'profile.submission.create': 'Create Issue',
  'profile.tasks.table.name': 'Name',
  'profile.tasks.table.owner': 'Owner',
  'profile.tasks.table.category': 'Category',
  'profile.tasks.table.type': 'Type',
  'profile.tasks.table.date': 'Date',
  'profile.tasks.table.created': 'Created',
  'profile.tasks.table.community': 'Community',
  'profile.tasks.table.status': 'Status',
  'profile.tasks.create.task': 'Create Task',
  'profile.detail.thankforinterest': 'Thanks for your interest.',
  'profile.detail.selectoption':
    'Please select below the option which describes you best.',
  'profile.detail.complookup': 'How much ELA do you want?',
  'profile.detail.solo': 'Tell us why do you want to join',
  'profile.detail.form.bid.required': 'Bid is required',
  'profile.detail.upload.whitepaper':
    'If you have a whitepaper ready, drag it or click here.',
  'profile.detail.upload.comment':
    'Use the Comments below to speak to the Project Owner and find out more.',
  'profile.detail.table.name': 'Name',
  'profile.detail.table.action': 'Action',
  'profile.detail.noapplications': 'No applications yet',
  'profile.detail.finding':
    'Funding: 100k for 5% of the equity or coins/tokens',
  'profile.detail.sendmessage': 'Send Message',
  'profile.detail.sendmessage.disabled':
    'You cannot send a message to yourself',
  'profile.detail.comingsoon': 'Coming soon...',
  'profile.detail.follow.disabled': 'You cannot follow yourself',
  'profile.skillset.header': 'Skill Set',
  'profile.social.header': 'Social Profile',
  'profile.portfolio': 'Portfolio',

  'profile.tasks.filter.all': 'All',
  'profile.tasks.filter.need_approval': 'Need Approval',
  'profile.tasks.filter.owned': 'Owned',
  'profile.tasks.filter.active': 'Active',
  'profile.tasks.filter.applied': 'Applied',
  'profile.tasks.filter.subscribed': 'Subscribed',

  'profile.submission.filter.all': 'All',
  'profile.submission.filter.created': 'Created',
  'profile.submission.filter.subscribed': 'Subscribed',

  'profile.submission.table.title': 'Title',
  'profile.submission.table.type': 'Type',
  'profile.submission.table.created': 'Created',

  'profile.member.vote.qrcodeTip':'Open ELA wallet and scan vote',

  // Validate Form
  'ambassadors.form.required': 'This must be filled out',
  'ambassadors.form.reason.max': 'Reason too long',
  'ambassadors.form.suitedreason.max': 'Suited reason too long',

  // Social
  'social.formcontribution.required': 'Please select a category',
  'social.formcontribution.option.blog': 'Blog',
  'social.formcontribution.option.video': 'Video',
  'social.formcontribution.option.podcast': 'Podcast',
  'social.formcontribution.option.other': 'Other',
  'social.formcontribution.button.submit': 'Submit',
  'social.joincommunity.community.required': 'This field is required',
  'social.joincommunity.button.join': 'Join community',
  'social.joincommunity.button.cancel': 'Cancel',
  'social.columns.name': 'Name',
  'social.columns.community': 'Community',
  'social.columns.reward': 'Reward',
  'social.columns.deadline': 'Deadline',
  'social.generalevent.header': 'General Events and Community Tasks',
  'social.generalevent.description':
    'This program is for members interested in helping organizers plan events or take on small tasks created by organizers to help promote Elastos to the community',
  'social.addmember.success': 'You was added to the community. Thanks!',
  'social.addmember.error': 'Error while joining the community',

  // Task Application
  'taks.application.social': 'Social',
  'taks.application.developer': 'Developer',
  'taks.application.general': 'General',
  'taks.create.project': 'Create Project',
  'taks.create.task': 'Create Task / Event',

  // Team Detail
  'team.detail.title': 'Team Detail',

  // Module Profile
  'profile.detail.requiredlogin':
    'You must login/register first to send a message',
  'profile.detail.columns.type': 'Type',
  'profile.detail.columns.name': 'Name',
  'profile.detail.columns.date': 'Date',
  'profile.detail.profile.title': 'Your Profile',
  'profile.detail.public': 'Public Profile',
  'profile.detail.button.edit': 'Edit',
  'profile.detail.username': 'Username',
  'profile.detail.role': 'Role',
  'profile.detail.email': 'Email',
  'profile.detail.firstname': 'First Name',
  'profile.detail.lastname': 'Last Name',
  'profile.detail.bio': 'Bio',
  'profile.detail.gender': 'Gender',
  'profile.detail.avatar': 'Avatar',
  'profile.detail.country': 'Country',
  'profile.detail.timezone': 'Timezone',
  'profile.detail.walletaddress': 'Wallet Address',
  'profile.detail.tobeorganizer': 'Do you want to be an organizer?',
  'profile.detail.tobeengineer': 'Are you a software developer or engineer?',
  'profile.detail.yes': 'Yes',
  'profile.detail.no': 'No',

  'profile.popover.email': 'Email',
  'profile.popover.did': 'DID',
  'profile.popover.name': 'Name',
  'profile.popover.copy': 'Copy',
  'profile.popover.viewProfile': 'View Profile',

  // Module form
  'from.CVoteForm.reason.yes.required': 'Please input the approval reason',
  'from.CVoteForm.reason.oppose.required': 'Please input the opposed reason',
  'from.CVoteForm.reason.abstain.required': 'Please input the abstained reason',
  'from.CVoteForm.message.delete.success': 'Delete success',
  'from.CVoteForm.message.updated.success': 'Update success',
  'from.CVoteForm.message.create.success': 'Create success',
  'from.CVoteForm.yes': 'YES',
  'from.CVoteForm.no': 'NO',
  'from.CVoteForm.proposal.title':
    'Cyber Republic Council Members Proposal Form',
  'from.CVoteForm.proposal.content':
    'Cyber Republic Council members can use this form to propose motion. All Cyber Republic citizen can view and share their own idea (offline). All proposals will be discussed in regular council meetings. All results will be disclosed to the public.',
  'from.CVoteForm.label.voteStatus': 'Status',
  'from.CVoteForm.label.publish': 'Publish',
  'from.CVoteForm.label.title': 'Title',
  'from.CVoteForm.label.type': 'Type',
  'from.CVoteForm.label.content': 'Content',
  'from.CVoteForm.label.proposedby': 'Proposed By',
  'from.CVoteForm.label.motion': 'Motion',
  'from.CVoteForm.label.motion.help':
    'If this is a motion against existing motion, refer to existing motion #',
  'from.CVoteForm.label.conflict': 'Conflict?',
  'from.CVoteForm.label.conflict.help':
    'Is this proposal potentially conflict with existing constitution?',
  'from.CVoteForm.label.note': 'Notes from Secretary',
  'from.CVoteForm.message.note.update.success': 'Update notes success!',
  'from.CVoteForm.text.onlycouncil':
    'Only Council Member could create or edit proposal.',

  'from.CVoteForm.button.add': 'Add a Proposal',
  'from.CVoteForm.button.cancel': 'Cancel',
  'from.CVoteForm.button.continue': 'Continue',
  'from.CVoteForm.button.preview': 'Preivew',
  'from.CVoteForm.button.saveDraft': 'Save as Draft',
  'from.CVoteForm.button.saveAndPublish': 'Save & Publish',
  'from.CVoteForm.button.saveChanges': 'Save changes',

  'from.CVoteForm.modal.publish': 'Are you sure to publish this proposal?',
  'from.CVoteForm.modal.title': 'Are you sure to complete this proposal?',
  'from.CVoteForm.modal.confirm': 'Confirm',
  'from.CVoteForm.modal.cancel': 'Cancel',
  'from.CVoteForm.message.proposal.update.success':
    'Complete proposal success!',

  'from.OrganizerAppForm.fullLegalName.required': 'Please input an name',
  'from.OrganizerAppForm.occupation.required': 'Please input an occupation',
  'from.OrganizerAppForm.education.required': 'Please input an education',
  'from.OrganizerAppForm.field.required': 'This is a required field',
  'from.OrganizerAppForm.fullLegalName.min': 'Name too short',
  'from.OrganizerAppForm.occupation.min': 'Occupation too short',
  'from.OrganizerAppForm.education.min': 'Education too short',
  'from.OrganizerAppForm.field.max': 'Text too long',
  'from.OrganizerAppForm.click.upload': 'Click to upload',
  'from.OrganizerAppForm.mustlogged':
    'You must be logged in to apply for organizer',
  'from.OrganizerAppForm.community.apply': 'Community Applying For:',
  'from.OrganizerAppForm.fullLegalName': 'Full Legal Name',
  'from.OrganizerAppForm.Occupation': 'Occupation',
  'from.OrganizerAppForm.Education': 'Education',
  'from.OrganizerAppForm.language':
    'What is your native language, who is your audience and where are they located? What are the language(s) you plan to use to present Elastos.',
  'from.OrganizerAppForm.speaking':
    'Please describe your public speaking experience and provide any examples.',
  'from.OrganizerAppForm.organizer':
    'Do you have any experience organizing events and provide any examples.',
  'from.OrganizerAppForm.contributions':
    'Please list any current or past contributions promoting Elastos.',
  'from.OrganizerAppForm.areyoudeveloper': 'Are you a developer?',
  'from.OrganizerAppForm.notdeveloper':
    'If you are not a developer, please explain how you are familiar with Elastos technology and what problems we solve.',
  'from.OrganizerAppForm.describeElastos':
    'Describe Elastos in your own words.',
  'from.OrganizerAppForm.inspired':
    'Tell us in a few words what inspired you to join Cyber Republic.',
  'from.OrganizerAppForm.divider.submitvideo':
    'Please submit a video explaining<br/>what Elastos means to you.',
  'from.OrganizerAppForm.submit': 'Submit',
  'from.OrganizerAppForm.attachment': 'Attachment',
  'from.OrganizerAppForm.message.success':
    'Success - one of the admins will be in touch shortly',

  'from.SubmissionCreateForm.title.required': 'Title is required',
  'from.SubmissionCreateForm.type.required': 'Please select an issue type',
  'from.SubmissionCreateForm.description.required':
    'You must have a description',
  'from.SubmissionCreateForm.description.max': 'Task description too long',
  'from.SubmissionCreateForm.type': 'Type',
  'from.SubmissionCreateForm.title': 'Title',
  'from.SubmissionCreateForm.description': 'Description',
  'from.SubmissionCreateForm.createissue': 'Create Issue',

  'from.TaskCreateForm.message.error':
    'You must confirm you have read the payment rules and disclaimer',
  'from.TaskCreateForm.taskName.required': 'Please input a task name',
  'from.TaskCreateForm.taskName.min': 'Task Name too short',
  'from.TaskCreateForm.taskCategory.required': 'Please select a category',
  'from.TaskCreateForm.taskType.required': 'Please select a task type',
  'from.TaskCreateForm.taskType.option.event': 'Event',
  'from.TaskCreateForm.taskType.option.task': 'Task',
  'from.TaskCreateForm.taskType.option.project': 'Project',
  'from.TaskCreateForm.taskDesc.required': 'You must have a description',
  'from.TaskCreateForm.taskDesc.max': 'Task description too long',
  'from.TaskCreateForm.taskDescBreakdown.max': 'Task breakdown too long',
  'from.TaskCreateForm.taskGoals.max': 'Task goals too long',
  'from.TaskCreateForm.taskLink.required': 'Please input an info link',
  'from.TaskCreateForm.taskLocation.required': 'Please input a location',
  'from.TaskCreateForm.taskCandLimit.required': 'You must set a limit',
  'from.TaskCreateForm.uploadtext': 'Click to upload',
  'from.TaskCreateForm.problem.max': 'Too long',
  'from.TaskCreateForm.label.name': 'Name',
  'from.TaskCreateForm.label.assigntocircle': 'Assign to Circle',
  'from.TaskCreateForm.label.community': 'Community',
  'from.TaskCreateForm.label.thumbnail': 'Thumbnail',
  'from.TaskCreateForm.attachment.confirm.remove':
    'Are you sure you want to remove this thumbnail?',
  'from.TaskCreateForm.text.ok': 'OK',
  'from.TaskCreateForm.label.category': 'Category',
  'from.TaskCreateForm.label.type': 'Type',
  'from.TaskCreateForm.label.application': 'Application Deadline',
  'from.TaskCreateForm.label.completeBy': 'Complete By',
  'from.TaskCreateForm.label.description': 'Description',
  'from.TaskCreateForm.label.goals': 'Goals',
  'from.TaskCreateForm.label.info': 'Info Link',
  'from.TaskCreateForm.label.problems': 'Problem you want to solve',
  'from.TaskCreateForm.label.valueProposition': 'Value proposition',
  'from.TaskCreateForm.label.usecase': 'Use Case',
  'from.TaskCreateForm.label.beneficiaries': 'Beneficiaries',
  'from.TaskCreateForm.label.elaInfrastructure': 'ELA Infrastructure',
  'from.TaskCreateForm.label.domain': 'Domain',
  'from.TaskCreateForm.label.recruiting': 'Recruiting Skillsets',
  'from.TaskCreateForm.label.prictures': 'Pictures',
  'from.TaskCreateForm.label.daterange': 'Date Range',
  'from.TaskCreateForm.label.eventdate': 'Event Date',
  'from.TaskCreateForm.label.start': ' Start',
  'from.TaskCreateForm.label.end': 'End',
  'from.TaskCreateForm.label.dateconfirm': 'Date Confirmation',
  'from.TaskCreateForm.label.location': 'Location',
  'from.TaskCreateForm.label.paymentassigment': 'Payment & Assignment',
  'from.TaskCreateForm.label.budgetlabor':
    'Budget is for expenses/costs, reward is for labor and time',
  'from.TaskCreateForm.label.private': 'Private',
  'from.TaskCreateForm.label.taskyourself':
    '- You wish to do this task yourself',
  'from.TaskCreateForm.label.proposing.approval':
    '- You are proposing a budget/reward for approval',
  'from.TaskCreateForm.label.notvisible': '- This is not visible to others',
  'from.TaskCreateForm.label.public': 'Public',
  'from.TaskCreateForm.label.taskforother': '- This is a task for others to do',
  'from.TaskCreateForm.label.publicly': '- This is listed publicly on the site',
  'from.TaskCreateForm.label.rewardbidding': '- Set a reward or allow bidding',
  'from.TaskCreateForm.label.rewardtype': 'Reward Type',
  'from.TaskCreateForm.label.fiat': 'Fiat ($USD)',
  'from.TaskCreateForm.label.usdbudget': 'USD Budget',
  'from.TaskCreateForm.label.usdreward': 'USD Reward',
  'from.TaskCreateForm.label.elabudget': 'ELA Budget',
  'from.TaskCreateForm.label.elareward': 'ELA Reward',
  'from.TaskCreateForm.label.disclaimerrule':
    'I have read the payment rules and disclaimer',
  'from.TaskCreateForm.label.referbid': 'Reference Bid',
  'from.TaskCreateForm.label.attachment': 'Attachment',
  'from.TaskCreateForm.label.remove.attachment':
    'Are you sure you want to remove this attachment?',
  'from.TaskCreateForm.button.savechange': 'Save Changes',
  'from.TaskCreateForm.button.createtask': 'Create Task',
  'from.TaskCreateForm.button.submitpropsal': 'Submit Proposal',
  'from.TaskCreateForm.label.paymentrules': 'Payment Rules and Disclaimer',
  'from.TaskCreateForm.button.close': 'Close',
  'from.TaskCreateForm.text.payment.billable':
    'Any billable work that goes beyond the original task description must be approved first',
  'from.TaskCreateForm.text.payment.upon':
    'Upon completion of the task/event, a full report is required to receive the reward payment',
  'from.TaskCreateForm.text.payment.exchange':
    'If payment figures are in USD, the exchange rate at the time of disbursement from',
  'from.TaskCreateForm.text.payment.used': 'will be used',
  'from.TaskCreateForm.text.payment.expenses':
    'All expenses require invoices or receipts',
  'from.TaskCreateForm.text.payment.agreement':
    'This agreement is only required if the the task is billable',

  'from.TeamCreateForm.text.upload': 'Upload',
  'from.TeamCreateForm.label.teamname': 'Team Name',
  'from.TeamCreateForm.label.type': 'Type',
  'from.TeamCreateForm.label.recrui': 'Recruiting Skillsets',
  'from.TeamCreateForm.label.description': 'Description',
  'from.TeamCreateForm.label.pictures': 'Pictures',
  'from.TeamCreateForm.button.save': 'Save',
  'from.TeamCreateForm.button.create': 'Create',

  'from.TeamEditForm.field.required': 'This field is required',
  'from.TeamEditForm.radio.yes': 'Yes',
  'from.TeamEditForm.radio.no': 'No',
  'from.TeamEditForm.label.name': 'Name',
  'from.TeamEditForm.label.type': 'Type',
  'from.TeamEditForm.label.recrui': 'Recruiting',
  'from.TeamEditForm.label.des': 'Description',
  'from.TeamEditForm.label.tags': 'Tags',
  'from.TeamEditForm.label.save': 'Save Changes',

  'from.Training1Form.field.required': 'This field is required',
  'from.Training1Form.field.email.invalid': 'Invalid email',
  'from.Training1Form.field.min': 'This field too short',
  'from.Training1Form.field.max': 'This field too long',
  'from.Training1Form.button.upload': 'Click to upload',
  'from.Training1Form.label.email': 'Email',
  'from.Training1Form.label.fullllegal': 'Full Legal Name',
  'from.Training1Form.label.occupation': 'Occupation',
  'from.Training1Form.label.education': 'Education',
  'from.Training1Form.text.nativelanguage':
    'What is your native language, who is your audience and where are they located? What are the language(s) you plan to use to present Elastos.',
  'from.Training1Form.text.describeyour':
    'Please describe your public speaking experience and provide any examples.',
  'from.Training1Form.text.listanycurrent':
    'Please list any current or past contributions promoting Elastos.',
  'from.Training1Form.text.adeveloper': 'Are you a developer?',
  'from.Training1Form.text.explain':
    'If you are not a developer, please explain how you are familiar with Elastos technology and what problems we solve.',
  'from.Training1Form.text.describe': 'Describe Elastos in your own words.',
  'from.Training1Form.text.tellfeword':
    'Tell us in a few words what inspired you to join Cyber Republic.',
  'from.Training1Form.text.submitvideo':
    'Please submit a video of your introduction to Cyber Republic.',
  'from.Training1Form.label.attachment': 'Attachment',
  'from.Training1Form.button.submit': 'Submit',

  'from.UserContactForm.message.success': 'Email sent successfully',
  'from.UserContactForm.field.required': 'This field is required',
  'from.UserContactForm.placeholder.message': 'Message',
  'from.UserContactForm.text.emailreply':
    "The email reply-to address will be set to your account's email, responses will go directly to your email",
  'from.UserContactForm.button.send': 'Send Message',

  'from.UserEditForm.label.bio': 'Biography',
  'from.UserEditForm.label.motto': 'Motto',
  'from.UserEditForm.label.profession': 'Profession',
  'from.UserEditForm.label.portfolio': 'Portfolio',
  'from.UserEditForm.label.github': 'GitHub',
  'from.UserEditForm.username.required': 'Username is required',
  'from.UserEditForm.firstName.required': 'First name is required',
  'from.UserEditForm.lastName.required': 'Last name is required',
  'from.UserEditForm.country.required': 'Please select your country',
  'from.UserEditForm.walletAddress.len': 'Address length error',
  'from.UserEditForm.timezone.placeholder': 'Select Timezone...',
  'from.UserEditForm.telegram.min': 'please enter at least 4 characters',
  'from.UserEditForm.label.firstName': 'First Name',
  'from.UserEditForm.label.lastName': 'Last Name',
  'from.UserEditForm.label.password': 'Password',
  'from.UserEditForm.label.confirm': 'Confirm Password',
  'from.UserEditForm.label.gender': 'Gender',
  'from.UserEditForm.label.wallet': 'Wallet',
  'from.UserEditForm.label.country': 'Country',
  'from.UserEditForm.label.timezone': 'Timezone',
  'from.UserEditForm.label.skillset': 'Skillset',

  'from.UserProfileForm.firstName.required': 'First name is required',
  'from.UserProfileForm.lastName.required': 'Last name is required',
  'from.UserProfileForm.bio.required': 'Biography is required',
  'from.UserProfileForm.upload.avatar': 'Upload Avatar',
  'from.UserProfileForm.upload.banner': 'Upload Banner',
  'from.UserProfileForm.text.firstName': 'First Name',
  'from.UserProfileForm.text.lastName': 'Last Name',
  'from.UserProfileForm.text.slogan': 'Bio',
  'from.UserProfileForm.text.motto': 'Profile Motto',

  // CR Video / Earn ELA page
  'cr-video.here': 'here',
  'cr-video.header.1': 'Reclaim Your Internet',
  'cr-video.header.2':
    'The Cyber Republic is a global community of collaborators who are forging the path to a secure and equitable smartweb, powered by blockchain technology.',
  'cr-video.join': 'Join us now',
  'cr-video.q1': 'What’s Wrong With the Internet Today?',
  'cr-video.q1.title.1': 'Disproportional Wealth Distribution',
  'cr-video.q1.paragraph.1':
    'Due to the structure of the internet, there is increased concentration of power and knowledge in a select few multi-national companies that are hard to regulate, prone to corruption, censorship and  abuse their power',
  'cr-video.q1.title.2': 'Lack of Security',
  'cr-video.q1.paragraph.2':
    'Devices, Operating systems and the internet are increasingly insecure by design leaving data and identity vulnerable to hackers, malware, spyware and bugs. Data and storage is centralised making it economical and easy for hackers and corruption. This makes a future with Internet of Things (loT) devices impossible.',
  'cr-video.q1.title.3': 'No Protection of Digital Assets',
  'cr-video.q1.paragraph.3':
    'Content creators (musicians, artists, writers etc) and innovators do not have the capacity to own, protect or sell their work like they can in the real world. This forces advertising to be the dominant monetisation strategy so intermediaries own the value chain leaving those in creative industries chronically underpaid.',
  'cr-video.q2': 'So the Internet is Broken. How do we Fix It?',
  'cr-video.q2.paragraph.1':
    'This is exactly the question that prompted Rong Chen to leave his role as a senior software developer at Microsoft in the year 2000 and start working on a solution with the power to circumvent the barriers to security, data ownership, and allocation of wealth that continue to be either ignored, or actively exploited, by the powerful companies who monopolise today’s internet services. Today, that solution exists. It’s called Elastos, and it’s the the world’s first internet operating system.',
  'cr-video.q3': 'Elastos: Smartweb Powered by Blockchain Technology',
  'cr-video.q3.paragraph.1':
    'Elastos is the safe and reliable internet of the future. Built utilizing the blockchain, this technological breakthrough provides the first completely safe environment on the web where decentralized applications are detached from the internet while also permitting full scalability to billions of users. Elastos enables the generation of wealth through ownership and exchange of your data and digital assets.',
  'cr-video.q3.paragraph.2':
    'Elastos is not a blockchain project but rather a network operating system project powered by blockchain technology so in that sense, Elastos is not directly competing with any other blockchain projects. It can work together with them to form this new ecosystem where the decentralized applications run directly on the device instead of running on the blockchain along with decentralized peer to peer network to transfer assets in a completely closed sandboxed environment, thereby solving the three pillar issues that are prevalent in the internet of today - security, scalability and decentralization.',
  'cr-video.q3.subtitle.1': 'Learn More About Elastos with these Resources:',
  'cr-video.q3.link.1': 'Elastos Whitepaper',
  'cr-video.q3.link.2': 'Elastos Developer Guide',
  'cr-video.q3.link.3': 'Elastos Non-Developer Guide',
  'cr-video.q3.link.4': 'Elastos.org',
  'cr-video.q4':
    'Where does the Cyber Republic fit in the Elastos Smartweb Framework?',
  'cr-video.q4.paragraph.1':
    'While the Elastos network operating system enables the secure environment necessary to house a new and improved internet can grow, it is an infrastructure only- unable to govern or foster the ecosystem without human instruction, participation, and collaboration.',
  'cr-video.q4.paragraph.2':
    'With this in mind, the Elastos foundation have created a collaborative virtual environment in which innovators from all locations, educational & professional backgrounds, skill sets, and schools-of-thought can collectively build on top of the Elastos infrastructure, write the new rules of internet governance (no more monopolies!), and ultimately shape the future of a virtual economy that serves everybody, not just a privileged few. In short, the Cyber Republic is a “Virtual Nation” that transcends borders and welcomes every member of the global community as a citizen, a contributor, and a governing entity.',
  'cr-video.q4.paragraph.3':
    'Practically speaking, the Cyber Republic (CR) is the virtual membership platform for developers, designers, entrepreneurs, organisers, and others to work together on projects that will facilitate a successful global shift onto the secure, economically viable, equal opportunity internet that the Elastos operating system makes possible.',
  'cr-video.q4.paragraph.4':
    'At this time, the majority of CR projects revolve either around building, enhancing, and testing the decentralised applications (dApps) that will run on Elastos, or around increasing the visibility of Elastos through marketing, writing, referrals, media, and a number of other promotion channels.',
  'cr-video.q4.paragraph.5':
    'Contributors can do anything from bug checking to enterprise dApp development, from writing a blog to organising an international conference. There are no parameters around your level of involvement in the Cyber Republic, there is freedom to define and execute your good ideas, and your perspective and expertise, whatever they may encompass, will be welcome in the community.',
  'cr-video.q5': 'Why Should I Join the Cyber Republic?',
  'cr-video.q5.paragraph.1':
    'Why not? If you’re reading this text right now, perhaps you’ve already decided that you need something different from the internet you’re currently being offered. Who is in the best position to shape the way you want the new internet to look? You know the answer to that. So go ahead; Sign up. Connect. Change the World. You’re a smartweb cofounder now.',
  'cr-video.q6': 'How to Join the Cyber Republic',
  'cr-video.q6.title.1': 'Step 1: Register on the Cyber Republic Platform',
  'cr-video.q6.paragraph.1': 'You can do that ',
  'cr-video.q6.title.1_2':
    'Step 1: Register, looks like you already did, but have you done the rest?',
  'cr-video.q6.paragraph.1_2':
    'Join CRcles, join a team, look for tasks/projects or even be an entrepreneur and form a team for a CR100 project',
  'cr-video.q6.title.2': 'Step 2: Join up to 2 CRcles',
  'cr-video.q6.paragraph.2':
    'CRcles represent different subsets of the CR community with shared skills and willingness to contribute to a specific kind of task or project (e.g. writing, marketing, development, etc.)',
  'cr-video.q6.title.3': 'Step 3: Engage with your fellow CRcle members',
  'cr-video.q6.paragraph.3_2': 'Say hi on the discussion board, ',
  'cr-video.q6.paragraph.3.link': 'join the discord chat, ',
  'cr-video.q6.paragraph.3_3':
    'or register for the next virtual meetup with your new collaborators.',
  'cr-video.q6.title.4':
    'Step 4: Browse active projects and tasks in the CR community',
  'cr-video.q6.paragraph.4':
    'Apply for the ones you find most interesting, and ',
  'cr-video.q6.paragraph.4.link': 'earn ELA for your contribution',
  'cr-video.q6.title.5':
    'Step 5: As a cofounder of the new internet, the rest is up to you.',
  'cr-video.q6.paragraph.5': 'Get out there and bring your ideas to life!',

  'cr-video.q6.subtitle.1':
    'Learn More About the Cyber Republic with these Resources:',
  'cr-video.q6.link.1': 'Cyber Republic Tutorial - Become an Organizer',
  'cr-video.q6.link.2': 'Cyber Republic Tutorial - Public Tasks',

  'cr-video.q7.title': 'Have Questions? Just Want to Get In Touch?',
  'cr-video.q7.subtitle': 'Enter your email and we will contact you personally',
  'cr-video.q7.button_text': 'Submit',

  'crcle.category.essential': 'Essential',
  'crcle.category.advanced': 'Advanced',
  'crcle.category.services': 'Services',
  'crcle.category.developer': 'Developer',
  'crcle.product': 'Product',
  'crcle.support': 'Support',
  'crcle.media': 'Media',
  'crcle.dAppAnalyst': 'dApp Analyst',
  'crcle.administration': 'Administration',
  'crcle.projectManager': 'Project Manager',
  'crcle.design': 'Design',
  'crcle.dAppConsultant': 'dApp Consultant',
  'crcle.operations': 'Operations',
  'crcle.security': 'Security',
  'crcle.translation': 'Translation',
  'crcle.finance': 'Finance',
  'crcle.businessDevelopment': 'Business Development',
  'crcle.partnership': 'Partnership',
  'crcle.investment': 'Investment',
  'crcle.marketing': 'Marketing',
  'crcle.qa': 'QA',
  'crcle.writing': 'Writing',
  'crcle.hr': 'HR',
  'crcle.legal': 'Legal',

  'user.skillset.select': 'Select skillsets',
  'user.profession.select': 'Select profession',

  'user.skillset.group.DESIGN': 'Design',
  'user.skillset.group.MARKETING': 'Marketing',
  'user.skillset.group.WRITING': 'Writing',
  'user.skillset.group.VIDEO': 'Video',
  'user.skillset.group.MUSIC': 'Music',
  'user.skillset.group.DEVELOPER': 'Developer',
  'user.skillset.group.BUSINESS': 'Business',

  'user.skillset.LOGO_DESIGN': 'Logo Design',
  'user.skillset.FLYERS': 'Flyers Design',
  'user.skillset.PACKAGING': 'Packaging Design',
  'user.skillset.ILLUSTRATION': 'Illustrations',
  'user.skillset.INFOGRAPHIC': 'Infographics',
  'user.skillset.PRODUCT_DESIGN': 'Product Design',
  'user.skillset.MERCHANDISE': 'Merchandise Design',
  'user.skillset.PHOTOSHOP': 'Photoshop',
  'user.skillset.SOCIAL_MEDIA_MARKETING': 'Social Media Marketing',
  'user.skillset.SEO': 'SEO',
  'user.skillset.CONTENT_MARKETING': 'Content Marketing',
  'user.skillset.VIDEO_MARKETING': 'Video Marketing',
  'user.skillset.EMAIL_MARKETING': 'Email Marketing',
  'user.skillset.MARKETING_STRATEGY': 'Marketing Strategy',
  'user.skillset.WEB_ANALYTICS': 'Web Analytics',
  'user.skillset.ECOMMERCE': 'E-Commerce',
  'user.skillset.MOBILE_ADVERTISING': 'Mobile Advertising',
  'user.skillset.TRANSLATION': 'Translation',
  'user.skillset.PRODUCT_DESCRIPTIONS': 'Product Desc. Writing',
  'user.skillset.WEBSITE_CONTENT': 'Website Content Writing',
  'user.skillset.TECHNICAL_WRITING': 'Technical Writing',
  'user.skillset.PROOFREADING': 'Proofreading',
  'user.skillset.CREATIVE_WRITING': 'Creative Writing',
  'user.skillset.ARTICLES_WRITING': 'Articles Writing',
  'user.skillset.SALES_COPY': 'Sales Copy Writing',
  'user.skillset.PRESS_RELEASES': 'Press Release Writing',
  'user.skillset.LEGAL_WRITING': 'Legal Writing',
  'user.skillset.INTROS': 'Intro Videos',
  'user.skillset.LOGO_ANIMATION': 'Logo Animation',
  'user.skillset.PROMO_VIDEOS': 'Promo Videos',
  'user.skillset.VIDEO_ADS': 'Video Ads',
  'user.skillset.VIDEO_EDITING': 'Video Editing',
  'user.skillset.VIDEO_MODELING': 'Video Modeling',
  'user.skillset.PRODUCT_PHOTO': 'Product Photography',
  'user.skillset.VOICE_OVER': 'Voice Overs',
  'user.skillset.MIXING': 'Music Mixing',
  'user.skillset.MUSIC_PRODUCTION': 'Music Production',
  'user.skillset.CPP': 'C++',
  'user.skillset.JAVASCRIPT': 'JavaScript',
  'user.skillset.GO': 'GO',
  'user.skillset.PYTHON': 'Python',
  'user.skillset.JAVA': 'Java',
  'user.skillset.SWIFT': 'Swift',
  'user.skillset.SOFTWARE_TESTING': 'Software Testing',
  'user.skillset.VIRTUAL_ASSISTANT': 'Virtual Assistant',
  'user.skillset.DATA_ENTRY': 'Data Entry',
  'user.skillset.MARKET_RESEARCH': 'Market Research',
  'user.skillset.BUSINESS_PLANS': 'Business Plans',
  'user.skillset.LEGAL_CONSULTING': 'Legal Consulting',
  'user.skillset.FINANCIAL_CONSULTING': 'Financial Consulting',
  'user.skillset.PRESENTATION': 'Business Presentations',

  // Council & Secretariat
  cs: {
    candidates: 'CANDIDATES',
    incumbent: 'INCUMBENT',
    council: 'CYBER REPUBLIC COUNCIL',
    intro: 'INTRODUCTION',
    voting: 'VOTING',
    secretariat: {
      title: 'SECRETARIAT',
      general: 'SECRETARIAT GENERAL',
      staff: 'SECRETARIAT STAFF',
      positions: {
        open: 'View Open Positions',
        title: 'SECRETARIAT - OPEN POSITIONS',
        jobDesc: 'JOB DESCRIPTION',
        toastMsg: 'We are now hiring for Council and Secretariat positions.',
        howtoApply: {
          title: 'HOW TO APPLY',
          desc: 'Apply for it via email: secretariat@cyberrepublic.org'
        },
        viewMore: 'View More',
        position_1: {
          title: 'Technology and Development Advisor',
          desc:
            'Provide professional advice to the council members regarding the proposals.'
        },
        position_2: {
          title: 'Investment Advisor',
          desc:
            'Provide professional advice to the council members regarding the proposals.'
        },
        position_3: {
          title: 'BD/PR Advisor',
          desc:
            'Provide professional advice to the council members regarding the proposals.'
        },
        position_4: {
          title: 'Project Manager',
          desc:
            'Proposal execution, manage the project to ensure the proposal is executed successfully.'
        }
      }
    },
    rule: {
      tile: 'Rules For Council',
      show: {
        click: 'Click',
        here: 'here',
        view: 'to view the rules for council'
      }
    },
    contact: 'Contact',
    no1: {
      name: 'Alex Shipp',
      intro: 'Elastos Foundation Writer, Era of Quantum Wealth co-author, Quantum Wealth Supernode Owner, without a home base.',
      email: 'ashipp37@gmail.com'
    },
    no2: {
      name: 'Yipeng Su',
      intro: 'Chief Architect of Elastos Foundation, based in Beijing',
      email: 'suyipeng@elastos.org'
    },
    no3: {
      name: 'Feng Zhang',
      intro: 'Lawyer, Founder of Digital-Era Blockchain Service Alliance, Co-Founder of Bit University, based in Shanghai',
      email: '41059324@qq.com'
    },
    rebecca: {
      name: 'Rebecca Zhu',
      intro: 'Project Director of Elastos Foundation, based in Beijing'
    }
  },

  // ConstitutionNavigator
  'counstitution.menu1': '01 Elastos Cyber Republic Constitution',
  'counstitution.menu2': '02 Elastos Cyber Republic Council Founding Document',
  'counstitution.menu3': '03 Elastos Cyber Republic Voting Rules',
  'counstitution.menu4':
    '04 Elastos Cyber Republic Council Conflict of Interest Policy',
  'counstitution.title1': 'Elastos Cyber Republic Constitution',
  'counstitution.title2': 'Elastos Cyber Republic Council Founding Document',
  'counstitution.title3': 'Elastos Cyber Republic Voting Rules',
  'counstitution.title4':
    'Elastos Cyber Republic Council Conflict of Interest Policy'
}

// lang mappings

// TASK_STATUS
en[`taskStatus.${TASK_STATUS.CREATED}`] = 'Created'
en[`taskStatus.${TASK_STATUS.PENDING}`] = 'Pending'
en[`taskStatus.${TASK_STATUS.APPROVED}`] = 'Approved'
en[`taskStatus.${TASK_STATUS.ASSIGNED}`] = 'Assigned'
en[`taskStatus.${TASK_STATUS.SUBMITTED}`] = 'Submitted'
en[`taskStatus.${TASK_STATUS.SUCCESS}`] = 'Success'
en[`taskStatus.${TASK_STATUS.DISTRIBUTED}`] = 'Distributed'
en[`taskStatus.${TASK_STATUS.CANCELED}`] = 'Canceled'
en[`taskStatus.${TASK_STATUS.EXPIRED}`] = 'Expired'

en[`taskType.${TASK_TYPE.TASK}`] = 'Task'
en[`taskType.${TASK_TYPE.SUB_TASK}`] = 'Sub Task'
en[`taskType.${TASK_TYPE.PROJECT}`] = 'Project'
en[`taskType.${TASK_TYPE.EVENT}`] = 'Event'

en[`cvoteStatus.${CVOTE_STATUS.DRAFT}`] = 'DRAFT'
en[`cvoteStatus.${CVOTE_STATUS.PROPOSED}`] = 'PROPOSED'
en[`cvoteStatus.${CVOTE_STATUS.ACTIVE}`] = 'PASSED'
en[`cvoteStatus.${CVOTE_STATUS.REJECT}`] = 'REJECTED'
en[`cvoteStatus.${CVOTE_STATUS.FINAL}`] = 'FINAL'
en[`cvoteStatus.${CVOTE_STATUS.INCOMPLETED}`] = 'INCOMPLETED'
en[`cvoteStatus.${CVOTE_STATUS.DEFERRED}`] = 'DEFERRED'
en[`cvoteStatus.${CVOTE_STATUS.NOTIFICATION}`] = 'NOTIFICATION'
en[`cvoteStatus.${CVOTE_STATUS.VETOED}`] = 'VETOED'

export default en
