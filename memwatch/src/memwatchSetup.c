/*
 * memwatchSetup.c
 *
 *  Created on: May 27, 2018
 *      Author: xiaofenh
 */

#include "memwatch.h"

void memwatch_setup(void (*myTraceOutput)(int))
{
    mwSetOutFunc(myTraceOutput);
    mwDoFlush(1);
    mwStatistics(MW_STAT_LINE);
    mwSetAriFunc(NULL);
}
