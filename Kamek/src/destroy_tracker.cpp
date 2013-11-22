#include <common.h>

// @@ %SET_OFFSET 817F4000

// @@ %IFREGION pal
// @@ %PATCH 801626C0 %b:817F4000
// @@ %ENDIF

int trackDestroy(int objptr) {
	int oldLR;
	__asm__ __volatile__ (
		"mflr %0"
		: "=r" (oldLR)
	);
	OSReport("D%x @%x", objptr, oldLR);
	return objptr;
}


