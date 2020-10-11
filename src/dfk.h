#ifndef DFK_H_
#define DFK_H_

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "config.h"
#include "key-handler.h"

bool dual_function_keys(KBDLLHOOKSTRUCT *input);

#endif /* DFK_H_ */
