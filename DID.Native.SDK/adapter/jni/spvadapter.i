%{
#include "spvadapter.h"
%}

unsigned long create(const char *walletDir, const char *walletId,
        const char *network, const char *resolver);

void destroy(unsigned long adapter);

int createIdTransaction(unsigned long adapter, const char *payload,
        const char *memo, const char *password);

const char *resolve(unsigned long adapter, const char *did);