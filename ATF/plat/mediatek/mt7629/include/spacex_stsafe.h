#ifndef __SPACEX_STSAFE_H_
#define __SPACEX_STSAFE_H_

#include <stdint.h>

extern uint32_t stsafe_bootcount;
extern uint32_t required_rollback_version;

// Update the anti-rollback count of the STSAFE zone corresponding to the active side
// of the board to reflect a desired rollback version. Return -1 on error, 0 on success.
int update_anti_rollback(uint32_t current_stsafe_ver, uint32_t desired_stsafe_ver);

#endif // __SPACEX_STSAFE_H_