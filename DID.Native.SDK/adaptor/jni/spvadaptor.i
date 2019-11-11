%{
#include "spvadaptor.h"
%}

unsigned long create(const char *walletDir, const char *walletId,
        const char *network, const char *resolver);

void destroy(unsigned long adaptor);

int createIdTransaction(unsigned long adaptor, const char *payload,
        const char *memo, const char *password);

const char *resolve(unsigned long adaptor, const char *did);