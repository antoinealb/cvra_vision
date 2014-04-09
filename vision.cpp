#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <sstream>

using namespace std;
using namespace cv;

#define HSV_SATURATION_MIN 127
#define HSV_SATURATION_MAX 240
#define HSV_VALUE_MIN 127
#define HSV_VALUE_MAX 240
#define HSV_RED_MIN 175
#define HSV_RED_MAX 5
#define HSV_YELLOW_MIN 15
#define HSV_YELLOW_MAX 30
#define FRACTION_IMG_THRESH 0.75

Mat vision_triangle_filter_img(Mat img_blur)
{
	Mat img_hsv;
    cvtColor(img_blur, img_hsv, CV_BGR2HSV);

    /* green filter */
    Mat img_filt_green;
    inRange(img_hsv, Scalar(60, 0, 0), Scalar(100, 255, 255), img_filt_green);
    /* black filter */
    Mat img_filt_black;
    inRange(img_hsv, Scalar(0, 0, 0), Scalar(255, 255, 65), img_filt_black);
    /* saturation filter */
    Mat img_filt_sat;
    inRange(img_hsv, Scalar(0, 0, 0), Scalar(255, 125, 255), img_filt_sat);

    /* total filtered image */
    Mat img_filt = img_filt_green + img_filt_black + img_filt_sat;

    /* smoothing the filtered image */
    Mat struct_elem = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    morphologyEx(img_filt, img_filt, MORPH_OPEN, struct_elem);

    return img_filt;
}

int vision_triangle_detect(Mat img)
{
    /* smooth color image */
    Mat img_blur;
    GaussianBlur(img, img_blur, Size(11, 11), 0, 0);

    Mat img_filt = vision_triangle_filter_img(img_blur);

#ifndef COMPILE_ON_ROBOT
    /* display the different results */
    namedWindow("Display window", WINDOW_AUTOSIZE);

    cout << "img.size: " << img.size() << endl;
    imshow("img", img);
    moveWindow("img", 0, 0);

    cout << "img_filt.size: " << img_filt.size() << endl;
    imshow("img_filt", img_filt);


    waitKey(0);
#endif

    return 0;
}

char vision_check_color(Mat img)
{
    char color = '0';

    medianBlur(img, img, 21);       // smooth color image 

    cvtColor(img, img, CV_BGR2HSV);

    unsigned int cnt_red = 0, cnt_yellow = 0;
    Vec3b hsv_pixel;
    for (int x = 0; x <= img.cols; x++) {
        for (int y = 0; y <= img.rows; y++) {
            hsv_pixel = img.at<Vec3b>(y, x);   
            /* check if saturation and value are in a good range */
            //if (hsv_pixel[1] >= HSV_SATURATION_MIN && hsv_pixel[1] <= HSV_SATURATION_MAX && 
            //    hsv_pixel[2] >= HSV_VALUE_MIN && hsv_pixel[2] <= HSV_VALUE_MAX) {
                /* check for colour */
                if (hsv_pixel[0] >= HSV_RED_MIN || hsv_pixel[0] <= HSV_RED_MAX)
                    cnt_red++;
                else if (hsv_pixel[0] >= HSV_YELLOW_MIN && hsv_pixel[0] <= HSV_YELLOW_MAX)
                    cnt_yellow++;
            //}
        }
    }

    /* testing if count in threshhold */
    unsigned int cnt_thresh = img.cols * img.rows * FRACTION_IMG_THRESH;
    if (cnt_red >= cnt_thresh)
        return 'r';
    else if (cnt_yellow >= cnt_thresh)
        return 'y';

    /* no colour found */
    return 'e';
}

int main(int argc, char** argv)
{
    namedWindow("Display window", WINDOW_AUTOSIZE);

    VideoCapture camera(0);     // open default camera
    if(!camera.isOpened())
        return -1;

    //Mat img = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // image as argument
    //if (!img.data ) {
    //    cout <<  "Could not open or find the image." << endl ;
    //    return -1;
    //}

    //int i = 0;
    //ostringstream s;

    Mat img;

    /* vision main loop */
    for (;;) {
        camera >> img;      // get new frame from camera

    //    i++;
    //    imwrite(("./images/img" + to_string(i) + ".jpg"), img);

        imshow("img", img);
        if(waitKey(100) >= 0) break;

        cout << "color: " << vision_check_color(img) << endl;
    }

    //vision_triangle_detect(img);

    //waitKey(0);

	return 0;
}
