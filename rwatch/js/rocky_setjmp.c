#include <rebbleos.h>
#include <setjmp.h>

int setjmp(jmp_buf env)
{
    KERN_LOG("rocky", APP_LOG_LEVEL_ERROR, "Called setjmp. This should not be possible. Expect instability.");
    return 0;
}

void longjmp(jmp_buf env, int val)
{
    KERN_LOG("rocky", APP_LOG_LEVEL_ERROR, "Called longjmp. This should not be possible. Definitely unstable.");
}