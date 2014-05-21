#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

using namespace std;
using namespace cv;

const unsigned char YELLOW = 0;
const unsigned char RED = 1;
const unsigned char ERROR = 2;

struct Triangle {
   	float x;
   	float y;
    unsigned char color;
};

vector<Triangle> vision_triangle_detect();

/* returns the dominant color that is above a certain threshhold
   r: red, y: yellow, e: else
 */
unsigned char vision_check_color();
