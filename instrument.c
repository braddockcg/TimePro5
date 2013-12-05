#include <stdio.h>
#include <assert.h>

#include "timepro.h"
#include "timepro_private.h"

void __cyg_profile_func_enter(void *this_fn, void *call_site)
    __attribute__((no_instrument_function));

void __cyg_profile_func_enter(void *this_fn, void *call_site) {
    unsigned long long lfn = (unsigned long long) this_fn;
    unsigned long hash = (lfn & 0xffff) 
        ^ ((lfn & 0xffff0000) >> 16);
    assert(hash <= 65536);
    ++blacklist[hash];
    if (blacklist[hash] > MAX_BLACKLIST_CALLS) {
        if (blacklist[hash] == MAX_BLACKLIST_CALLS + 1)
        {
            fprintf(stderr, "Blacklisting ");
            PrintSymbolName(stderr, this_fn);
            fprintf(stderr, " for %i calls\n", blacklist[hash]);
        }
        return;
    }
#if 0
    {
        char func[100];
        GetSymbolName(this_fn, func, 100);
        fprintf(stderr, "ENTER: %s at %p, from %p\n", func, this_fn, call_site);
    }
#endif
    TPContext *ctx = TPGetContext();
    if (ctx)
        TPStartWithContext(ctx, 0x0, this_fn, -1);
} /* __cyg_profile_func_enter */

void __cyg_profile_func_exit(void *this_fn, void *call_site)
    __attribute__((no_instrument_function));

void __cyg_profile_func_exit(void *this_fn, void *call_site) {
    unsigned long long lfn = (unsigned long long) this_fn;
    unsigned long hash = (lfn & 0xffff) 
        ^ ((lfn & 0xffff0000) >> 16);
    if (blacklist[hash] > MAX_BLACKLIST_CALLS)
        return;
#if 0
    {
        char func[100];
        GetSymbolName(this_fn, func, 100);
        fprintf(stderr, "EXIT: %s at %p, from %p\n", func, this_fn, call_site);
    }
#endif
    TPContext *ctx = TPGetContext();
    if (ctx)
        TPEndWithContext(ctx, 0x0, this_fn, -1);
} /* __cyg_profile_func_enter */
