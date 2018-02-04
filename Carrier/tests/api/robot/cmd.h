#ifndef __ROBOT_CMD_H__
#define __ROBOT_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ela_carrier.h"
#include "ela_session.h"
#include "../cond.h"
#include "test_helper.h"

#define MAX_CHANNEL_COUNT   256

char* read_cmd(void);
void  do_cmd(TestContext*, char*);

extern CarrierContext carrier_context;
extern TestContext test_context;

#ifdef __cplusplus
}
#endif

#endif
