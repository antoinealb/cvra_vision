#include "vision.hpp"
#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

int main(int argc, char** argv)
{

#ifndef COMPILE_ON_ROBOT
    namedWindow("Display window", WINDOW_AUTOSIZE);
#endif

    vector<Triangle> triangle = vision_triangle_detect();

#ifndef COMPILE_ON_ROBOT
    waitKey(0);
#endif

	return 0;
}
