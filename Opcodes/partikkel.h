/*
Partikkel - a granular synthesis module for Csound 5
Copyright (C) 2006-2007 �yvind Brandtsegg, Torgeir Strand Henriksen,
Thom Johansen

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "csdl.h"

typedef struct {
    FUNC *table;
    double phase, delta;
    double sweepoffset, sweepdecay;
    MYFLT gain;
} WAVEDATA;

typedef struct {
    unsigned start, stop;
    double envphase, envinc;
    double envattacklen, envdecaystart;
    double env2amount;
    MYFLT fmamp;
    FUNC *fmenvtab;
    unsigned harmonics;
    MYFLT falloff, falloff_pow_N;
    MYFLT gain1, gain2;
    unsigned chan1, chan2;
    WAVEDATA wav[5];
} GRAIN;

/* which of the wav[] entries above correspond to the trainlet generator */
#define WAV_TRAINLET 4

/* support structs for the grain pool routines */
typedef struct NODE_ {
    GRAIN grain;
    struct NODE_ *next;
} NODE;

typedef struct {
    NODE *grainlist;
    char *mempool;
    unsigned free_nodes;
} GRAINPOOL;

typedef struct PARTIKKEL_GLOBALS_ENTRY_ {
    MYFLT id;
    MYFLT *synctab;
    struct PARTIKKEL_GLOBALS_ENTRY_ *next;
} PARTIKKEL_GLOBALS_ENTRY;

typedef struct {
    /* default tables. name describes table contents, 'z' is 0.0, 'o' is 1.0 */
    FUNC *ooo_tab;
    FUNC *zzz_tab;
    FUNC *zzooooo_tab;
    char *tablestorage;
    PARTIKKEL_GLOBALS_ENTRY *rootentry;
} PARTIKKEL_GLOBALS;

typedef struct {
    OPDS h;
    /* output arrays */
    MYFLT *output1, *output2, *output3, *output4;
    MYFLT *output5, *output6, *output7, *output8;

    /* opcode parameters */
    MYFLT *grainfreq;
    MYFLT *distribution;
    MYFLT *dist;
    MYFLT *sync;
    MYFLT *env2_amount;
    MYFLT *env2;
    MYFLT *env_attack;
    MYFLT *env_decay;
    MYFLT *sustain_amount;
    MYFLT *a_d_ratio;
    MYFLT *duration;
    MYFLT *amplitude;
    MYFLT *gainmasks;
    MYFLT *wavfreq;
    MYFLT *freqsweepshape;
    MYFLT *wavfreq_startmuls;
    MYFLT *wavfreq_endmuls;
    MYFLT *fm;
    MYFLT *fm_indices;
    MYFLT *fm_env;
    MYFLT *cosine;
    MYFLT *trainletfreq;
    MYFLT *harmonics;
    MYFLT *falloff;
    MYFLT *channelmasks;
    MYFLT *randommask;
    MYFLT *waveform1, *waveform2, *waveform3, *waveform4;
    MYFLT *waveamps;
    MYFLT *samplepos1, *samplepos2, *samplepos3, *samplepos4;
    MYFLT *wavekey1, *wavekey2, *wavekey3, *wavekey4;
    MYFLT *max_grains;
    MYFLT *opcodeid;

    /* internal variables */
    PARTIKKEL_GLOBALS *globals;
    PARTIKKEL_GLOBALS_ENTRY *globals_entry;
    GRAINPOOL gpool;
    NODE *grainroot;
    int out_of_voices_warning;
    unsigned num_outputs;
    int grainfreq_arate;
    int synced;
    AUXCH aux, aux2;
    CsoundRandMTState randstate;
    FUNC *wavetabs[4];
    FUNC *costab;
    unsigned cosineshift;
    MYFLT zscale;
    FUNC *disttab;
    unsigned distindex;
    unsigned disttabshift;
    FUNC *env2_tab, *env_attack_tab, *env_decay_tab;
    FUNC *fmenvtab;
    FUNC *gainmasktab;
    unsigned gainmaskindex;
    FUNC *wavfreqstarttab, *wavfreqendtab;
    unsigned wavfreqstartindex, wavfreqendindex;
    FUNC *fmamptab;
    unsigned fmampindex;
    FUNC *channelmasktab;
    unsigned channelmaskindex;
    FUNC *wavgaintab;
    unsigned wavgainindex;
    double grainphase, prevphase, graininc;
    double nextgrainphase;
} PARTIKKEL;

typedef struct {
    OPDS h;
    /* output arrays */
    MYFLT *output;

    /* opcode parameters */
    MYFLT *opcodeid;

    /* internal variables */
    PARTIKKEL_GLOBALS_ENTRY *ge;
} PARTIKKELSYNC;
