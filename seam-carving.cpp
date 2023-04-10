#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    string image_path = "images/golf.jpg";
    Mat img = imread(image_path);
    Mat imgGray, imgBlur1, imgBlur2;

    // cvtColor(img,imgGray,COLOR_BGR2RGB);

    GaussianBlur(img,imgBlur1,Size(25,25),5,5);
    GaussianBlur(img,imgBlur2,Size(5,5),45,45);

    Point2f P(5, 1);

    Scalar pixel = img.at<uchar>(P);
    cout << "Point (2D) = " << pixel << endl << endl;
    //imshow("Test1", imgGray);

    // imshow("Test1", imgBlur1);
    // imshow("Test2", imgBlur2);

    waitKey(0);
}

// int main(int argc, char** argv) {
//     string vid_path = "images/earth.mp4";
//     VideoCapture cap(1);
//     Mat img;

//     while(true) {
//         cap.read(img);
//         imshow("Image", img);
//         waitKey(1);
//     }

//     // imshow("Test", img);
//     // waitKey(0);
// }

