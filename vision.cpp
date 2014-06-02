#include "vision.hpp"

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

unsigned char vision_pixel_color(Vec3b hsv_pixel)
{
    /* check if saturation and value are in a good range */
    /* check for colour */
    if (hsv_pixel[0] >= HSV_RED_MIN || hsv_pixel[0] <= HSV_RED_MAX)
        return RED;
    else if (hsv_pixel[0] >= HSV_YELLOW_MIN && hsv_pixel[0] <= HSV_YELLOW_MAX)
        return YELLOW;

    return ERROR;
}

vector<Triangle> vision_triangle_centroids(vector<Point2f> edges, Mat img)
{
    vector<Triangle> triangles;
    struct Triangle tmp_triangle;

    cvtColor(img, img, CV_BGR2HSV);

    for (int i = 0; i < edges.size(); i++) {
        for (int j = i + 1; j < edges.size(); j++) {
            for (int k = j + 1; k < edges.size(); k++) {
                tmp_triangle.x = (edges[i].x + edges[j].x + edges[k].x) / 3;
                tmp_triangle.y = (edges[i].y + edges[j].y + edges[k].y) / 3;
                tmp_triangle.z = 0;

                tmp_triangle.color = vision_pixel_color(img.at<Vec3b>((int)tmp_triangle.y,
                    (int)tmp_triangle.x));

                tmp_triangle.horizontal = true;

                triangles.push_back(tmp_triangle);
            }
        }
    }

    return triangles;
}

vector<Point2f> vision_lines_intersect(vector<Vec2f> lines_cart)
{
    vector<Point2f> edges;
    Point2f p;

    for (int i = 0; i < lines_cart.size(); i++) {
        for (int j = i + 1; j < lines_cart.size(); j++) {
            p.x = (lines_cart[j][1] - lines_cart[i][1]) / (lines_cart[i][0] - lines_cart[j][0]);
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
    vector<Point2f> edges, vector<Triangle> triangles){
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

    cout << "\nedges: " << edges.size() << endl;
    for (int i = 0; i < edges.size(); i++) {
        circle(img, edges[i], 10, Scalar(255, 200, 0));
    }

    cout << "\ntriangles: " << triangles.size() << endl;
    Point2f pnt_tmp;
    for (int i = 0; i < triangles.size(); i++) {
        pnt_tmp.x = triangles[i].x;
        pnt_tmp.y = triangles[i].y;
        circle(img, pnt_tmp, 5, Scalar(255, 0, 0));
        cout << "\ntriangles_centeroid: " << triangles[i].x << ", " << triangles[i].y << ", " <<
            triangles[i].z << endl;
    }
}

/* 10 mm: 48 pxl -> 24 pxl */
Triangle vision_pxl2mm(Triangle triangle)
{
    Point2f pnt_mm;

    triangle.x = triangle.x;
    /* see img_to_3d_interpol.m to find fomula */
    triangle.y = 0.639124976364529 + 0.420199226762668 * triangle.y +
        -0.000123372385815 * pow(triangle.y,2);

    cout << "\npxl_mm: " << triangle.x << ", " << triangle.y << endl;

    return triangle;
}

Triangle vision_img_coord_to_3d(Triangle triangle)
{
    float height_triangle;  /* height over the table */

    triangle = vision_pxl2mm(triangle);
    triangle.y = 355.5 - triangle.y + 88.85;

    if (triangle.horizontal)
        triangle.z = 36;        /* centroid if triangle horizontal */
    else
        triangle.z = 40.4;      /* centroid if triangle vertical */

    return triangle;
}

Mat vision_take_picture()
{
    Mat img;

    VideoCapture camera(0);     // open default camera
    if(!camera.isOpened()) {
        cout << "Cam not opened.";
    }

    camera >> img;

    return img;
}

Mat vision_open_picture()
{
    Mat img = imread("./img_part1.jpg", CV_LOAD_IMAGE_COLOR);   // image as argument
    if (!img.data ) {
        cout <<  "Could not open or find the image." << endl ;
    }

    return img;
}

/* returns the dominant color that is above a certain threshhold
   r: red, y: yellow, e: else
 */
unsigned char vision_check_color()
{
    unsigned char color = ERROR;

    Mat img = vision_open_picture();

    medianBlur(img, img, 21);       // smooth color image

    cvtColor(img, img, CV_BGR2HSV);

    unsigned int cnt_red = 0, cnt_yellow = 0;
    Vec3b hsv_pixel;
    for (int x = 0; x <= img.cols; x++) {
        for (int y = 0; y <= img.rows; y++) {
            hsv_pixel = img.at<Vec3b>(y, x);
            if (vision_pixel_color(hsv_pixel) == RED)
                cnt_red++;
            else if (vision_pixel_color(hsv_pixel) == YELLOW)
                cnt_yellow++;
        }
    }

    /* testing if count in threshhold */
    unsigned int cnt_thresh = img.cols * img.rows * FRACTION_IMG_THRESH;
    if (cnt_red >= cnt_thresh)
        color = RED;
    else if (cnt_yellow >= cnt_thresh)
        color = YELLOW;

    return color;
}

vector<Triangle> vision_triangle_detect()
{
    vector<Triangle> triangles;

#ifdef COMPILE_ON_ROBOT
    Mat img = vision_take_picture();
#else
    Mat img = vision_open_picture();
#endif

    /* smooth color image */
    GaussianBlur(img, img, Size(11, 11), 0, 0);

    Mat img_filt = vision_triangle_filter_img(img);

    Mat img_edges;
    Canny(img_filt, img_edges, 100, 1500);

    vector<Vec2f> lines;
    HoughLines(img_edges, lines, 1, CV_PI/180, 50, 0, 0);
    cout << "\ninitial lines.size(): " << lines.size() << endl << endl;

    vector<Vec2f> lines_cart;
    vector<Point2f> edges;
    if (lines.size() != 0) {
        lines = vision_lines_group(lines);
        lines_cart = vision_lines_polar2cart(lines);

        if(lines.size() == 3) {
            edges = vision_lines_intersect(lines_cart);

            triangles = vision_triangle_centroids(edges, img);
            triangles[0] = vision_img_coord_to_3d(triangles[0]);

        }
    }

#ifndef COMPILE_ON_ROBOT
    vision_draw_line(img, lines, lines_cart, edges, triangles);

    imshow("img", img);
    moveWindow("img", 0, 0);

    imshow("img_filt", img_filt);
    moveWindow("img_filt", 640, 0);
#endif

    return triangles;
}
