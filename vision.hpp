#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

using namespace std;
using namespace cv;

extern "C" {
    const unsigned char YELLOW = 0;
    const unsigned char RED = 1;
    const unsigned char ERROR = 2;

    typedef struct triangle_iterator_t TriangleIterator;
    typedef struct triangle_t Triangle;

    bool hasNext(TriangleIterator * iter);
    Triangle * next(TriangleIterator * iter);

    float getX(Triangle * t);
    float getY(Triangle * t);
    float getZ(Triangle * t);
    unsigned char getColor(Triangle * t);
    bool getHorizontal(Triangle * t);

    TriangleIterator * getTriangles(void);
    void deleteTriangleIterator(TriangleIterator * iter);

    unsigned char vision_check_color(void);
}

struct triangle_t {
    float x;
    float y;
    float z;
    unsigned char color;
    bool horizontal;
};

struct triangle_iterator_t {
    vector<Triangle> triangles;
    unsigned int current_index;
};

vector<Triangle> vision_triangle_detect();

