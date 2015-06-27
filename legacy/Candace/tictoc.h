#ifndef tictoc_h
#define tictoc_h

#include "circular.h"

extern unsigned int last_tic;
extern unsigned int last_toc;

#define TIC last_tic=T0TC
#define TOC last_toc=T0TC

void writeTicToc(circular& buf);

#endif
