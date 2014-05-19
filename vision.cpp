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
#define THETA_THRESH 0.4
#define RHO_THRESH 40

#define HSV_SATURATION_MIN 127
#define HSV_SATURATION_MAX 245
#define HSV_VALUE_MIN 127
#define HSV_VALUE_MAX 255
#define HSV_RED_MIN 175
#define HSV_RED_MAX 10
#define HSV_YELLOW_MIN 15
#define HSV_YELLOW_MAX 35
#define FRACTION_IMG_THRESH 0.75

Mat vision_triangle_filter_img(Mat img)
{
    Mat img_hsv;
    cvtColor(img, img_hsv, CV_BGR2HSV);

    /* green filter */
    Mat img_filt_green;
    inRange(img_hsv, Scalar(50, 0, 0), Scalar(100, 255, 255), img_filt_green);
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

char vision_pixel_color(Vec3b hsv_pixel)
{
    /* check if saturation and value are in a good range */
    //if (hsv_pixel[1] >= HSV_SATURATION_MIN && hsv_pixel[1] <= HSV_SATURATION_MAX && 
    //    hsv_pixel[2] >= HSV_VALUE_MIN && hsv_pixel[2] <= HSV_VALUE_MAX) {
        /* check for colour */
        if (hsv_pixel[0] >= HSV_RED_MIN || hsv_pixel[0] <= HSV_RED_MAX)
            return 'r';
        else if (hsv_pixel[0] >= HSV_YELLOW_MIN && hsv_pixel[0] <= HSV_YELLOW_MAX)
            return 'y';
    //}

    return -1;
}

vector<Point2f> vision_triangle_centroids(vector<Point2f> edges)
{
    vector<Point2f> centroids;
    Point2f point;

    for (int i = 0; i < edges.size(); i++) {
        for (int j = i + 1; j < edges.size(); j++) {
            for (int k = j + 1; k < edges.size(); k++) {
                point.x = (edges[i].x + edges[j].x + edges[k].x) / 3;
                point.y = (edges[i].y + edges[j].y + edges[k].y) / 3;

                //if (vision_pixel_color(hsv_pixel))
                    centroids.push_back(point);
            }
        }
    }

    return centroids;
}

vector<Point2f> vision_lines_intersect(vector<Vec2f> lines_cart)
{
    vector<Point2f> edges;
    Point2f p;

    for (int i = 0; i < lines_cart.size(); i++) {
        for (int j = i + 1; j < lines_cart.size(); j++) {
            p.x = (lines_cart[j][1] - lines_cart[i][1]) / (lines_cart[i][0] - 
                lines_cart[j][0]);
            p.y = lines_cart[i][0] * p.x + lines_cart[i][1];

            edges.push_back(p);
        }
    }

    return edges;
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
            cout << "-- [" << lines_grouped[j][0] << "," << lines_grouped[j][1] 
                << "]" << endl;
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

void vision_draw_line(Mat img, vector<Vec2f> lines, vector<Vec2f> lines_cart, 
    vector<Point2f> edges, vector<Point2f> centroids){
    cout << "\nlines: " << lines.size() << endl;
    for (int i = 0; i < lines.size(); i++ ) {
        cout << "  [" << lines[i][0] << "," << lines[i][1] << "]" << endl;
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 2000*(-b));
        pt1.y = cvRound(y0 + 2000*(a));
        pt2.x = cvRound(x0 - 2000*(-b));
        pt2.y = cvRound(y0 - 2000*(a));
        line(img, pt1, pt2, Scalar(255, 0, i*90), 1, CV_AA);
    }

    cout << "\nlines_cart: " << lines_cart.size() << endl;
    for (int i = 0; i < lines_cart.size(); i++) {
        //cout << "  [" << lines_cart[i][0] << "," << lines_cart[i][1] << "]" 
            << endl;
    }

    cout << "\nedges: " << edges.size() << endl;
    for (int i = 0; i < edges.size(); i++) {
        circle(img, edges[i], 10, Scalar(255, 200, 0));
        //cout << "  " << edges[i] << endl;
    }

    cout << "\ncentroids: " << centroids.size() << endl;
    for (int i = 0; i < centroids.size(); i++) {
        circle(img, centroids[i], 5, Scalar(0, 150, 255));
        //cout << "  " << centroids[i] << endl;
    }
}

/* returns the dominant color that is above a certain threshhold
   r: red, y: yellow, e: else
 */
char vision_check_color(Mat img)
{
    char color = '0';

    medianBlur(img, img, 21);       // smooth color image 

    imwrite("./img_part.jpg", img);

    cvtColor(img, img, CV_BGR2HSV);

    unsigned int cnt_red = 0, cnt_yellow = 0;
    Vec3b hsv_pixel;
    for (int x = 0; x <= img.cols; x++) {
        for (int y = 0; y <= img.rows; y++) {
            hsv_pixel = img.at<Vec3b>(y, x);
            if (vision_pixel_color(hsv_pixel) == 'r')
                cnt_red++;
            else if (vision_pixel_color(hsv_pixel) == 'y')
                cnt_yellow++;
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

Point3f vision_img_coord_to_3d(Point2f pnt_img, bool horizontal)
{
    float height_triangle;  /* height over the table */
    /* initialize camera matrix (see camera_data_matrix.xml) */
    float data_mat[9] = {0.000693407496133f, 0.0f, 
        -0.443434093777100f, 0.0f, 0.000693407496133f, -0.332488894395808f, 
        0.0f, 0.0f, 1.000000000000000f};
    Mat cam_mat_inv(3, 3, CV_32FC1, data_mat);

    cout << "\npoint_image: " << pnt_img << endl;

    float data_pnt[3] = {pnt_img.x, pnt_img.y, 1};
    Mat pnt_img_homogene(3, 1, CV_32FC1, data_pnt);

    pnt_img_homogene = pnt_img_homogene * 140/220;  /* conversion to mm */
    pnt_img_homogene = cam_mat_inv * pnt_img_homogene;

    cout << "\nres: " << pnt_img_homogene << endl;

    if (horizontal)
        height_triangle = 36;       /* centroid if triangle horizontal */
    else
        height_triangle = 40.4;     /* centroid if triangle vertical */

    Point3f pnt_img_homo(pnt_img_homogene.at<float>(0,0) * height_triangle, 
        pnt_img_homogene.at<float>(0,1) * height_triangle, height_triangle);

    return pnt_img_homo;
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
        pt1.x = cvRound(x0 + 2000*(-b));
        pt1.y = cvRound(y0 + 2000*(a));
        pt2.x = cvRound(x0 - 2000*(-b));
        pt2.y = cvRound(y0 - 2000*(a));
        line(img, pt1, pt2, Scalar(0, 180, 255), 1, CV_AA);*/
    cout << "\ninitial lines.size(): " << lines.size() << endl << endl;

    vector<Vec2f> lines_cart;
    vector<Point2f> edges, centroids;
    vector<Point3f> centroids_robot;
    if (lines.size() != 0) {
        lines = vision_lines_group(lines);
        lines_cart = vision_lines_polar2cart(lines);


        if(lines.size() == 3) {
            edges = vision_lines_intersect(lines_cart);

            centroids = vision_triangle_centroids(edges);

            Mat img_part;
            img(Rect(centroids[0].x - 10, centroids[0].y - 10, 20, 20)).copyTo(
                img_part);
            cout << "\nColor of triangle: " << vision_check_color(img_part) << 
                endl;

            bool horizontal = true;

            centroids_robot.push_back(vision_img_coord_to_3d(centroids[0], 
                horizontal));
            cout << "\ncentroids_robot: " << centroids_robot[0] << endl;
        } /*else {
            return -1;
        }*/
    }


#ifndef COMPILE_ON_ROBOT
    vision_draw_line(img, lines, lines_cart, edges, centroids);

    imshow("img", img);
    moveWindow("img", 0, 0);

    imshow("img_filt", img_filt);
    moveWindow("img_filt", 640, 0);
#endif

    return 0;
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

        imshow("img", img);
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
