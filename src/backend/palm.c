/* $Id$ */
#include <Mw/Milsko.h>

#include "../../external/stb_ds.h"

static MwLL MwLLCreateImpl(MwLL parent, int x, int y, int width, int height) {
	(void)(parent);
	(void)(x);
	(void)(y);
	(void)(width);
	(void)(height);

	MwLL r;

	return r;
}

static void MwLLDestroyImpl(MwLL handle) {
	MwLLDestroyCommon(handle);
	free(handle);
}

static void MwLLPolygonImpl(MwLL handle, MwPoint* points, int points_count, MwLLColor color) {
	(void)(handle);
	(void)(points);
	(void)(points_count);
	(void)(color);
}

static void MwLLLineImpl(MwLL handle, MwPoint* points, MwLLColor color) {
	(void)(handle);
	(void)(points);
	(void)(color);
}

// couldn't this be made generic across all implementations?
static MwLLColor MwLLAllocColorImpl(MwLL handle, int r, int g, int b) {
	MwLLColor c = malloc(sizeof(*c));
	MwLLColorUpdate(handle, c, r, g, b);
	return c;
}

static void MwLLColorUpdateImpl(MwLL handle, MwLLColor c, int r, int g, int b) {
	(void)(handle);
	(void)(c);
	(void)(r);
	(void)(g);
	(void)(b);
}

static void MwLLGetXYWHImpl(MwLL handle, int* x, int* y, unsigned int* w, unsigned int* h) {
	*x = 0;
	*y = 0;
	*w = 0;
	*h = 0;
}

static void MwLLSetXYImpl(MwLL handle, int x, int y) {
	(void)(handle);
	(void)(x);
	(void)(y);
}

static void MwLLSetWHImpl(MwLL handle, int w, int h) {
	(void)(handle);
	(void)(w);
	(void)(h);
}

static void MwLLFreeColorImpl(MwLLColor color) {
	free(color);
}

static void MwLLSetBackgroundImpl(MwLL handle, MwLLColor color) {
	(void)(handle);
	(void)(color);
}

static int MwLLPendingImpl(MwLL handle) {
	(void)(handle);
	return 0;
}

static void MwLLNextEventImpl(MwLL handle) {
	(void)(handle);
}

static void MwLLSetTitleImpl(MwLL handle, const char* title) {
	(void)(handle);
	(void)(title);
}

static MwLLPixmap MwLLCreatePixmapImpl(MwLL handle, unsigned char* data, int width, int height) {
	(void)(handle);
	(void)(data);
	(void)(width);
	(void)(height);
	MwLLPixmap r = malloc(sizeof(*r));

	return r;
}

static void MwLLPixmapUpdateImpl(MwLLPixmap r) {
	(void)(r);
}

static void MwLLDestroyPixmapImpl(MwLLPixmap pixmap) {
	(void)(pixmap);
}

static void MwLLDrawPixmapImpl(MwLL handle, MwRect* rect, MwLLPixmap pixmap) {
	(void)(handle);
	(void)(rect);
	(void)(pixmap);
}

static void MwLLSetIconImpl(MwLL handle, MwLLPixmap pixmap) {
	(void)(handle);
	(void)(pixmap);
	// Unimplementable stub
}

static void MwLLForceRenderImpl(MwLL handle) {
	(void)(handle);
}

static void MwLLSetCursorImpl(MwLL handle, MwCursor* image, MwCursor* mask) {
	(void)(handle);
	(void)(image);
	(void)(mask);
	// Unimplementable stub
}

static void MwLLDetachImpl(MwLL handle, MwPoint* point) {
	(void)(handle);
	(void)(point);
}

static void MwLLShowImpl(MwLL handle, int show) {
	(void)(handle);
	(void)(show);
}

static void MwLLMakePopupImpl(MwLL handle, MwLL parent) {
	(void)(handle);
	(void)(parent);
}

static void MwLLSetSizeHintsImpl(MwLL handle, int minx, int miny, int maxx, int maxy) {
	(void)(handle);
	(void)(minx);
	(void)(miny);
	(void)(maxx);
	(void)(maxy);
}

static void MwLLMakeBorderlessImpl(MwLL handle, int toggle) {
	(void)(handle);
	(void)(toggle);
}

static void MwLLFocusImpl(MwLL handle) {
	(void)(handle);
}

static void MwLLGrabPointerImpl(MwLL handle, int toggle) {
	(void)(handle);
	(void)(toggle);
}

static void MwLLSetClipboardImpl(MwLL handle, const char* text) {
	(void)(handle);
	(void)(text);
	// Unimplementable stub
}

static char* MwLLGetClipboardImpl(MwLL handle) {
	(void)(handle);
	// Unimplementable stub
	return NULL;
}

static void MwLLMakeToolWindowImpl(MwLL handle) {
	(void)(handle);
}

static int MwLLPalmCallInitImpl(void) {
	/* TODO: check properly */
	return 0;
}

#include "call.c"
CALL(Palm);
