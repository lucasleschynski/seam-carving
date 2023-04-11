#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat convert_to_edges(Mat src) {
    int ddepth = CV_16S;
    Mat output, grad_x, grad_y;

    // GaussianBlur(src, output, Size(3, 3), 0);
    output = src.clone();
    cvtColor(output, output, COLOR_BGR2GRAY);

    Sobel(output, grad_x, ddepth, 1, 0, 3);//, 0.01);  // Sobel for X direction
    Sobel(output, grad_y, ddepth, 0, 1, 3);//, 0.01);  // Sobel for Y direction

    // Scale
    convertScaleAbs(grad_x, grad_x);
    convertScaleAbs(grad_y, grad_y);

    // Combine X and Y gradients
    addWeighted(grad_x, 0.5, grad_y, 0.5, 0, output);
    return output;
}

int main(int argc, char** argv) {
    string image_path = "images/shapes.jpg";
    Mat img = imread(image_path);
    Mat sobel, edges;

    resize(img, img, Size(), 0.5, 0.5);

    edges = convert_to_edges(img);


    Point2f P(5, 1);
    Scalar pixel = img.at<uchar>(P);
    cout << "Point (2D) = " << pixel << endl << endl;

    imshow("sobel", edges);

    // imshow("Test1", imgBlur1);
    // imshow("Test2", imgBlur2);

    waitKey(0);
}

