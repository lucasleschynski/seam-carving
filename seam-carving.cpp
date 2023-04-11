#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

Mat convert_to_edges(Mat& src) {
    int ddepth = CV_16S;
    Mat output, gray, grad_x, grad_y, energy_image;

    GaussianBlur(src, gray, Size(3, 3), 0);
    // output = src.clone();
    cvtColor(gray, output, COLOR_BGR2GRAY);

    Sobel(output, grad_x, ddepth, 1, 0, 3);  // Sobel for X direction
    Sobel(output, grad_y, ddepth, 0, 1, 3);  // Sobel for Y direction

    // Scharr(output, grad_x, ddepth, 1, 0);
    // Scharr(output, grad_y, ddepth, 0, 1);

    // Scale
    convertScaleAbs(grad_x, grad_x);
    convertScaleAbs(grad_y, grad_y);

    // Combine X and Y gradients
    addWeighted(grad_x, 0.5, grad_y, 0.5, 0, output);

    output.convertTo(energy_image, CV_64F, 1.0/255.0);
    return output;
}

void mat_to_vector(Mat mat, vector<vector<int>>& dest) {
    vector<vector<int>> vec(mat.rows, vector<int>(mat.cols));
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            vec[i][j] = (int)mat.at<uchar>(i, j);
        }
    }
    dest = vec;
}

void print_2d_vec(const vector<vector<int>>& vec) {
    for (const auto& row : vec) {
        cout << "[";
        for (const auto& element : row) {
            cout << element << ",";
        }
        cout << "]";
        cout << "\n" << endl;
    }
}

int main(int argc, char** argv) {
    string image_path = "images/bikes.jpg";
    Mat img = imread(image_path);
    Mat sobel, edges;

    edges = convert_to_edges(img);

    vector<vector<int>> edge_vec;
    mat_to_vector(edges, edge_vec);

    imshow("original", img);
    imshow("edges", edges);

    waitKey(0);
}

