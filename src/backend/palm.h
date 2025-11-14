/* $Id$ */
#ifndef __PALM_H__
#define __PALM_H__

#include <Mw/MachDep.h>
#include <Mw/TypeDefs.h>
#include <Mw/LowLevel.h>

struct _MwLLPalm {
	struct _MwLLCommon common;

	int a;
};

struct _MwLLPalmColor {
	struct _MwLLCommonColor common;

	int b;
};

struct _MwLLPalmPixmap {
	struct _MwLLCommonPixmap common;

	int c;
};

#endif
