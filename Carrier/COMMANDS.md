## List of commands for the Elastos Carrier shell application.

**help** - Display available command list.<br>
**help [Command]** - Display usage documentation for command.<br>
**clear** - Clear log and output view in shell.<br>
**clear [ log | out ]** - Clear log or output view in shell.<br>

**address** - Display own address.<br>
**nodeid** - Display own node ID.<br>
**userid** - Display own user ID.<br>
**me** - Display own user ID, name, description, gender, phone, email and region.<br>
**me set [name | description | gender | phone | email | region] [Value]** - Set own user details individually.<br>
**nospam** - Display current nospam value.<br>
**nospam [Value]** - Change nospam value to enforce address change.<br>
**presence** - Display self presence.<br>
**presence [none | away | busy]** - Change own presence.

**fadd [Address] [Message]** - Add new friend.<br>
**faccept [User ID]** - Accept friend request.<br>
**fremove [User ID]** - Remove friend.<br>
**friends** - List all friends.<br>
**friend [User ID]** - Display friend details.<br>
**label [User ID] [Name]** - Add label to friend.<br>
**msg  [User ID] [Message]** -  Send message to a friend.<br>
**invite [User ID] [Message]** - Invite friend.<br>
**ireply [User ID] confirm [Message] *OR* ireply [User ID] refuse [Message]** - Confirm or refuse invitation with a message.

**gnew** - Create new group.<br>
**gleave [Group ID]** - Leave group.<br>
**ginvite [Group ID] [User ID]** - Invite user to group.<br>
**gjoin [User ID] cookie** - Group invitation from user with cookies.<br>
**gmsg [Group ID] [Message]** - Send message to group.<br>
**gtitle [Group ID]** - Display title of group.<br>
**gtitle [Group ID] [Title]** - Set title of group.<br>
**gpeers [Group ID]** - Display list of participants in group.<br>
**glist** - Display list of joined group.

**sinit** - Initialize session.<br>
**snew  [User ID]** - Start new session with user.<br>
**sadd [plain | reliable | multiplexing | portforwarding]** - Add session properties.<br>
**sremove [Session ID]** - Leave session.<br>
**srequest bundle** - Bundle and start session.<br>
**sreply ok** - Accept session request.<br>
**sreply refuse [Message]** - Refuse session request with reason as a message.<br>
**swrite [Stream ID] [String]** - Send data to stream.<br>
**sbulkwrite [Stream ID] [Packet size] [Packet count]** -  Send bulk data to stream.<br>
**sbulkrecv [ start | end ]** - Start or end receiving in bulk.<br>
**scadd [Stream]** - Add stream channel.<br>
**sinfo [ID]** - Display stream information.<br>
**scclose [Stream] channel** - Close stream channel.<br>
**scwrite [Stream] channel [String]** - Write to stream channel.<br>
**scpend [Stream] channel** - Display pending stream channels.<br>
**scresume [Stream] channel** - Resume stream.<br>
**sclose** - Close session.<br>
**spfsvcadd [Name] [tcp|udp] [Host] [Port]** - Add service to session.<br>
**spfsvcremove [Name]** - Remove service from session.<br>
**spfopen [Stream] [Service] [tcp|udp] [Host] [Port]** - Open portforwarding.<br>
**spfclose [Stream] [PF ID]** - Close portforwarding.<br>
**scleanup** - Cleanup session.<br>
**kill** - Stop carrier.<br>
