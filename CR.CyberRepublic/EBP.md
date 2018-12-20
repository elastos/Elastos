# Gist
Elastos is an open source project made for the community. We have reserved 16.5M ELA in order to reward the Elastos community members who contribute to the commmunity. EBP - Elastos Bounty Program is a web app platform for community members to take tasks, make contributions and get ELA as reward.

The whole EBP contains several subsidiary programs
- Elastos Developer Bounty Program - EDBP. Developers contribute code to Elastos
- Elastos Bug Bounty Program - EBBP. Users or testers find bugs and report to Elastos developers
- Elastos Social Bounty Program - ESBP. Social media users and influencers introduce Elastos to social media.
- Elastos Leader Bounty Program - ELBP. Reward to those leaders in different subsidiary communities (Community leaders in Universities, Citites and Countries)
- Elastos Referral Bounty Program - ERBP. Reward to members who refer other users, leaders, members to join Elastos community that made significant contribution.

## EDBP: Elastos Developer Bounty Program
EDBP will start around June 2018.
Developers all over the world can join this program no matter geolocation and working hours.
The tasks will include
- Core components
- DApp samples
- Tutorial and training material
- Utilities, tools


## EBBP: Elastos Bug Bounty Program
EBBP will start Q4 2018.
All users can test our system, find bug and report to developers to get reward

The tasks will include
- Find bugs and report
- Join testing program, running real human test(especially on UI / UX)
- Report user experiences (automatic user behavior report)

## ESBP: Elastos Social Bounty Program
ESBP will start Q2 2018
All users can join ESBP and the tasks include
- Record YouTube videos, podcasts etc explaining Elastos (social or technical)
- Write blogs, tweets, facebook posts etc explaining Elastos
- Ask or answer questions on social media, especially on Stackoverflow
- Repost, forward and rebroadcast any information related to Elastos for maximum exposure to the wider community

## ELBP: Elastos Leader Bounty Program
ELBP will start Q2 2018
Candidates need to lead a local Elastos community group, either at a University or in a country/region/city. They need to
- Organize local meetups, Hackathons, Seminars, Webinar events
- Introduce Elastos to the wider audience
- Lead DApp projects

[Leader Proposal Guidelines](./ELBP_Guidelines.md)

## ERBP: Elastos Referral Bounty Program
ERBP will start Q2, 2018
- Members will get referral reward for referring other members who then went on to make significant contribution to the Elastos community.

## Other special bounty program
//TODO

# Terminalogies
## Campaign
A task is a campaign.
A campaign meets the following criteria
- Campaign manager: Someone who is in charge for a particular campaign. This person designs the campaign and explains it to other people. They also issue ELA as reward to the winners.
- Start and Expire time: Even circulating campaign needs an expiration and auto renew policy
- Reward: How the reward is calculated
- Detail all the rules in plain English so anyone can understand and participate

Campaign has several types
### Development task
Example: I want someone to build some sample code for Elastos carrier. I post the requirements and offer 100 ELA + 100 Voting power as reward. I require candidates to send proposal to me in 3 days. 3 candidates send me the proposal and their schedule. I choose A to be the winner. If A complete the task on time and with quality, I will send ELA and issue the voting power to A. If A did not make it, or did not complete the task on time (who may not even have met any middle milestone), the campaign would be stopped, and I would restart the campaign and may issue the reward to B. And so on.

### Competition task
Example: I want to have a Elastos logo competition. I offer 100 ELA + 100 Voting power for the winner. Candidates need to submit their design logo in one month. After one month, 10 candidates logos are received. I will post all design logos to the EBP site. Everyone in the community can vote which one is the best out of all the possible candidates. The winner will then receive ELA and Voting power.
Sometime we also offer the 2nd or 3rd some kind of reward, like uncle reward etc.

### Proof of Work task
Example: I post a long term campaign. I offer 1 ELA and 1 voting power to 100 members every month who answer technical questions that are valuable on Stackoverflow. All candidates need to submit some proof of work to me. If I get 200 candidates submits, I can then choose the top 100 and issue them ELA and voting power.

### Voting task
Example: Every 3 months, we vote for the next country leader for the Indian community. We will require the entire Indian community number to vote and the winner will be selected as the next community leader in the Indian community. The leader themselves would get a fixed amount of bonus due to their work and contribution during these 3 months.

### Peer Bonus task
Example: Every month, we will ask members to transfer their voting power to anyone who has provided them with tremendous big help during the current month. It could be a team leader, or some youtuber, or someone who answered tough technical questions. These voting power will be converted to 1:1 ELA as bonus to the helper. So if you want to get more bonus, the best way is by helping more people!

## Reward
We have two kind of reward in the EBP system.
- ELA reward and
- Voting Power reward

### ELA reward.
It is just ELA. Winner will get ELA from the campaign manager.
### Voting Power reward
Usually a winner will get another kind of reward besides ELA. It is called voting power.
Voting power is a point reward system.

Voting power:
- Can be used to vote in the EBP system. For example, it can be used to determine the best DApp in a competition or possibly the next country leader
- Will be destroyed after voting. It is a limited resource
- Cannot be used to vote for themselves
- Cannot be converted to ELA or any kind of currency
- Can only be obtained as a reward from the EBP, eg, doing a task
- Will expire if not use within a certain timeframe

Voting power is inspired by Steem's design. It encourage members to earn more voting power so that they can contribute to the Elastos community more meaningfully. The more voting power you own, the more voting weight you have.

The differences between ELA and Voting power
- ELA won't expire but Voting power will. User needs to use them on time of they will expire
- ELA can be exchanged to or from other currency/coin or fiat but Voting power can't. It has no real cash value
- ELA can be used anonymously but Voting power needs real user ID.
- ELA can be purchased but Voting power is only issued to people who contribute to EBP. If you do not participate in the EBP, you won't have voting power

# EBP platform webapp brief introduction
Before the Elastos platform is complete and live, we can only use traditional centralized platform to build the EBP webapp. It will be a cloud based webapp. It will have a mobile version as well.

## Accounts
User needs to login to participate in EBP. Login will be associates with
- github account for all developers and testers.
- google account for all youtubers
- other account system for proof of ownership
- ELA public address for receiving ELA

A user can have multiple accounts associate with them

User can login, logout, edit and save profile. They can also check account balance and send voting power etc.


## Roles
- System admin
- Campaign manager
- Users

## ELA
ELA will NOT be saved to this platform. We only record the transaction history. Campaign manager needs to use ELA web wallet or any other ELA compatible wallet to transfer ELA funds

## Voting power
Voting power will be saved in the system. Every time a user receives voting power, it will be recorded with the amount they have accumulated along with the expiration date.
For example:

| Amount | Issue date | Expire date | Status | Memo |
|-----|------|------|-----|------|
| 100 | 01-01-2018 | 04-01-2018 | expired | from campaign ABC |
| 50 | 04-01-2018 | 07-01-2018 | active | from gift bonus program BCE |


When sending or consuming voting power, the oldest will be used first.

Unused voting power will expire and will no longer be valid.
After consuming, the voting power will be burned.

## Cascade organization
In the beginning, Elastos foundation will be the only campaign manager. We shall issue ELA to each local community leader and each DAPP project leader. They could be the next campaign manager who could then break down the tasks and post to the EBP platform. The community member then takes the task and gets rewarded after completing their tasks.
As an example, this is how the system will work. ELA funds flow from the Elastos foundation as a source then gets transferred to each smaller level project leaders and local team leaders. They get these ELA funds and break down the tasks. ELA will then flow to the local community members. And so on.

### EBP Templates

These are for reference only:

Task Proposal Template: https://docs.google.com/spreadsheets/d/1gu5V20B4qBoB38IviPE5lh0aVfQJuUa4CHvObFO33NE/edit#gid=0

# [EBP platform technical design](./EBP_Tech_Design.md)

# [EBP platform UX design](./EBP_UX_Design.md)

# Future of EBP
When Elastos is ready, we will move the EBP to Elastos platform, as one of the DApps on Elastos.

