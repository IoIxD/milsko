#include <Mw/Milsko.h>

int main() {
	MwWidget w = MwVaCreateWidget(MwWindowClass, "main", NULL, MwDEFAULT, MwDEFAULT, 5 + 640 + 5, 5 + 32 + 5,
				      MwNtitle, "progress bar",
				      NULL);
	MwWidget p = MwVaCreateWidget(MwProgressBarClass, "progress", w, 5, 5, 640, 32,
				      MwNvalue, 25,
				      NULL);

	(void)p;

	MwLoop(w);
}
