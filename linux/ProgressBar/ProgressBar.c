
// MT, 2016mar30

#include <stdio.h>
#include <assert.h>
#include "ProgressBar.h"

void ProgressBar_print(int const inFirst, int const inCur, int const inLast, int const inWidth, bool const inShowAbs)
{
    double const lastD = (double)(inLast-inFirst),
        curD = (double)(inCur-inFirst),
        percentD = (100.0*curD)/lastD;
    int const percent = (int)(percentD+0.5); // Rounds

    int const maxLen = inWidth-2,
        barLen = (int)((((double)maxLen)*percentD)/100.0+0.5), // Rounds
        percentPos = (int)(((double)maxLen)/2.0-2.0/*+0.5*/); // Truncates

    int i = 0;

    if(inCur>inFirst)
    {
        printf("\r");
    }
    printf("[");
    for(;i<barLen;++i)
    {
        if(i==percentPos)
        {
            printf("%3d%%", percent);
            i += 3;
        }
        else
        {
            if(i<(barLen-1))
            {
                printf("=");
            }
            else
            {
                printf(">");
            }
        }
    }

    for(;i<maxLen;++i)
    {
        if(i==percentPos)
        {
            printf("%3d%%", percent);
            i += 3;
        }
        else
        {
            printf(" ");
        }
    }
    printf("]");
    if(inShowAbs)
    {
        printf(" %d/%d", (int)curD, (int)lastD);
    }
    fflush(stdout);
}
