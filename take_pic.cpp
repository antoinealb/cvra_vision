#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

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

    VideoCapture camera(0);     // open default camera
    if(!camera.isOpened())
        return -1;

    if( argc != 2) {
        printf( "\nNo path indicated!\n" );
        return -1;
    }

    Mat img;

    camera >> img;      // get new frame from camera

    imwrite(argv[1], img);		// record series of images

/*#ifdef COMPILE_ON_ROBOT
    char buf[2];
    sprintf(buf, "%c", vision_check_color(img));

    sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    i++;
    imwrite(("./images/img" + to_string(i) + ".jpg"), img_orig);     // record series of images
    waitKey(200);
#endif*/

	return 0;
}
