#ifndef _ASSERT_H
#define _ASSERT_H

#include "dbglog.h"

#define assert(condition){                       \
  if (!(condition))                              \
  {                                              \
        LOG_ERROR("Assert failed");              \
  }                                              \
 }                                               \

#endif //_ASSERT_H
