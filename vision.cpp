#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

extern "C" {
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
}

using namespace std;
using namespace cv;

#define DIFFERENCE(n1, n2) (abs(n1 - n2))
#define THETA_THRESH 0.2
#define RHO_THRESH 20

#define HSV_SATURATION_MIN 127
#define HSV_SATURATION_MAX 240
#define HSV_VALUE_MIN 127
#define HSV_VALUE_MAX 240
#define HSV_RED_MIN 175
#define HSV_RED_MAX 5
#define HSV_YELLOW_MIN 15
#define HSV_YELLOW_MAX 30
#define FRACTION_IMG_THRESH 0.75

Mat vision_triangle_filter_img(Mat img)
{
    Mat img_hsv;
    cvtColor(img, img_hsv, CV_BGR2HSV);

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

vector<Vec2f> vision_lines_intersect(vector<Vec2f> lines_cart)
{
    vector<Vec2f> points;
    Vec2f p;

    for (int i = 0; i < lines_cart.size(); i++) {
        for (int j = i + 1; j < lines_cart.size(); j++) {
            p[0] = (lines_cart[j][1] - lines_cart[i][1]) / (lines_cart[i][0] - lines_cart[j][0]);
            p[1] = lines_cart[i][0] * p[0] + lines_cart[i][1];

            points.push_back(p);
            cout << "points: " << p << endl;
        }
    }

    return points;
}

vector<Vec2f> vision_lines_polar2cart(vector<Vec2f> lines)
{
    vector<Vec2f> lines_cart;
    Vec2f y;        // cartesian form of line equation y = m*x + b, y=[m,b]
    int rho = 0, theta = 0;

    for (int i = 0; i < lines.size(); i++) {
        
        if (lines[i][0] >= 0) {
            y[0] = -(cos(lines[i][1]) / sin(lines[i][1]));
            y[1] = lines[i][0] / sin(lines[i][1]);
        } else {
            y[0] = -(cos(lines[i][1]) / sin(lines[i][1]));
            y[1] = -(abs(lines[i][0]) / sin(lines[i][1]));
        }

        lines_cart.push_back(y);
    }

    return lines_cart;
}

vector<Vec2f> vision_lines_group(vector<Vec2f> lines)
{
    vector<Vec2f> lines_grouped;
    lines_grouped.push_back(lines[0]);
    lines.erase(lines.begin());

    while (!lines.empty()) {
        cout << "- [" << lines[0][0] << "," << lines[0][1] << "]" << endl;
        cout << lines_grouped.size() << endl;
        for (int j = 0; j < lines_grouped.size(); j++) {
            cout << "-- [" << lines_grouped[j][0] << "," << lines_grouped[j][1] << "]" << endl;
            if ((DIFFERENCE(lines[0][1], lines_grouped[j][1])) < THETA_THRESH && 
                DIFFERENCE(lines[0][0], lines_grouped[j][0]) < RHO_THRESH) {
                lines.erase(lines.begin());
                break;
            } else {
                if (j == (lines_grouped.size() - 1)) {
                    lines_grouped.push_back(lines[0]);
                    lines.erase(lines.begin());
                    break;
                }
            }
        }
    }

    return lines_grouped;
}

void vision_draw_line(Mat img, vector<Vec2f> lines, vector<Vec2f> lines_cart, vector<Vec2f> points){
    cout << "\nlines: \n";
    for (int i = 0; i < lines.size(); i++ ) {
        cout << "  [" << lines[i][0] << "," << lines[i][1] << "]" << endl;
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line(img, pt1, pt2, Scalar(255, 0, i*90), 1, CV_AA);
    }

    cout << "\nlines_cart: \n";
    for (int i = 0; i < lines_cart.size(); i++) {
        cout << "  [" << lines_cart[i][0] << "," << lines_cart[i][1] << "]" << endl;
    }

    cout << "\npoints: \n";
    Point point;
    for (int i = 0; i < points.size(); i++) {
        point.x = (int)points[i][0];
        point.y = (int)points[i][1];
        circle(img, point, 10, Scalar(255, 200, 0));
        cout << "  " << point << endl;
    }
}

int vision_triangle_detect(Mat img)
{
    /* smooth color image */
    GaussianBlur(img, img, Size(11, 11), 0, 0);

    Mat img_filt = vision_triangle_filter_img(img);

    Mat img_edges;
    Canny(img_filt, img_edges, 100, 1500);

    vector<Vec2f> lines;
    HoughLines(img_edges, lines, 1, CV_PI/180, 50, 0, 0);
    //vision_draw_line(img, lines);
        /*float rho = -25, theta = 1.7;
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line(img, pt1, pt2, Scalar(0, 180, 255), 1, CV_AA);*/
    cout << "\ninitial lines.size(): " << lines.size() << endl << endl;

    vector<Vec2f> lines_cart, points;
    if (lines.size() != 0) {
        lines = vision_lines_group(lines);
        lines_cart = vision_lines_polar2cart(lines);

        points = vision_lines_intersect(lines_cart);
    }


#ifndef COMPILE_ON_ROBOT
    vision_draw_line(img, lines, lines_cart, points);

    cout << "img.size: " << img.size() << endl;
    imshow("img", img);
    moveWindow("img", 0, 0);

    cout << "img_filt.size: " << img_filt.size() << endl;
    imshow("img_filt", img_filt);
    moveWindow("img_filt", 640, 0);
#endif

    return 0;
}

/* returns the dominant color that is above a certain threshhold
   r: red, y: yellow, e: else
 */
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
/*#ifdef COMPILE_ON_ROBOT
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(4242);

#endif*/

#ifndef COMPILE_ON_ROBOT
    namedWindow("Display window", WINDOW_AUTOSIZE);
#endif

/*    VideoCapture camera(0);     // open default camera
    if(!camera.isOpened())
        return -1;*/

    if( argc != 2) {
        printf( " No image data \n " );
        return -1;
    }
    Mat img = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // image as argument
    if (!img.data ) {
        cout <<  "Could not open or find the image." << endl ;
        return -1;
    }

//    Mat img, img_orig;
//    int i = 0;

    /* vision main loop */
/*    for (;;) {
        camera >> img >> img_orig;      // get new frame from camera

#ifndef COMPILE_ON_ROBOT
        cout << "color: " << vision_check_color(img) << endl;

        imshow("img", img_orig);
        if(waitKey(10) >= 0) break;
#endif

#ifdef COMPILE_ON_ROBOT
        char buf[2];
        sprintf(buf, "%c", vision_check_color(img));

        sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        i++;
        imwrite(("./images/img" + to_string(i) + ".jpg"), img_orig);     // record series of images
        waitKey(200);
#endif
    }*/

    vision_triangle_detect(img);

#ifndef COMPILE_ON_ROBOT
    waitKey(0);
#endif

	return 0;
}
