//
// Created by kunshanyu on 11/04/2018.
//

#ifndef SPEEDTEST_SPEEDTEST_H
#define SPEEDTEST_SPEEDTEST_H

typedef struct ElaFriendInfoNode {
    ElaFriendInfo info;
    struct ElaFriendInfoNode *next;
} ElaFriendInfoNode;

#ifdef __cplusplus
extern "C" {
#endif

int start_carrier(const char *address, const char *transferred_file);
void stop_carrier();
void add_friend(const char *address);
int accept_friend(const char *userid);
int remove_friend(const char *userid);
void request_test_speed(const char *userid);
void accept_test_speed(const char *userid);
void refuse_test_speed(const char *userid);
//ElaCarrier *get_carrier();
char *get_userid(char *userid, size_t len);
char *get_address(char *address, size_t len);
ElaFriendInfoNode *get_friends(void **context);
int free_friends(void *context);

#ifdef __cplusplus
}
#endif

#endif //SPEEDTEST_SPEEDTEST_H
