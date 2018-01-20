#ifndef __ELA_TURNSERVER_H__
#define __ELA_TURNSERVER_H__

#define ELA_MAX_TURN_SERVER_LEN         63

#define ELA_MAX_TURN_USERNAME_LEN       127

#define ELA_MAX_TURN_PASSWORD_LEN       63

#define ELA_MAX_TURN_REALM_LEN          127

typedef struct ElaTurnServer {
    char server[ELA_MAX_TURN_SERVER_LEN + 1];
    uint16_t port;
    char username[ELA_MAX_TURN_USERNAME_LEN + 1];
    char password[ELA_MAX_TURN_PASSWORD_LEN + 1];
    char realm[ELA_MAX_TURN_REALM_LEN + 1];
} ElaTurnServer;

CARRIER_API
int ela_get_turn_server(ElaCarrier *carrier, ElaTurnServer *turn_server);

#endif // __ELA_TURNSERVER_H__
