// Plotting with MathGL Test

#include "../lib/mathgl-2.4.2-mingw.win64/include/mgl.h"
#include "stdafx.h"


int main() {


	mglGraph gr;
	gr.FPlot("sin(pi*x)");
	gr.WriteFrame("test.png");


}