#ifndef MIKR_H
#define MIKR_H

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "pipeline.h"

class mikrosha{

    public:
    mikrosha(void)=default;
    ~mikrosha(void)=default;

    error_code start(void);
};

#endif      