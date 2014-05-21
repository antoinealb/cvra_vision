#include "vision.hpp"
#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

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

    /*if( argc != 2) {
        printf( " No image data \n " );
        return -1;
    }
    Mat img = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // image as argument
    if (!img.data ) {
        cout <<  "Could not open or find the image." << endl ;
        return -1;
    }*/

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

    vector<Triangle> triangle = vision_triangle_detect();

    /*unsigned char color = vision_check_color();
    cout << "\nColor: " << (int)color << endl;*/

#ifndef COMPILE_ON_ROBOT
    waitKey(0);
#endif

	return 0;
}
