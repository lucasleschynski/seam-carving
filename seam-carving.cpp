#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;
using namespace cv;

Mat convert_to_edges(Mat& src) {
    int ddepth = CV_16S;
    Mat output, gray, grad_x, grad_y, energy_image;

    GaussianBlur(src, gray, Size(3, 3), 0);
    cvtColor(gray, output, COLOR_BGR2GRAY);

    Sobel(output, grad_x, ddepth, 1, 0, 3);  // Sobel for X direction
    Sobel(output, grad_y, ddepth, 0, 1, 3);  // Sobel for Y direction

    // Scale
    convertScaleAbs(grad_x, grad_x);
    convertScaleAbs(grad_y, grad_y);

    // Combine X and Y gradients
    addWeighted(grad_x, 0.5, grad_y, 0.5, 0, output);

    output.convertTo(energy_image, CV_64F, 1.0/255.0);
    return output;
}

void gray_mat_to_vector(Mat mat, vector<vector<uint8_t>> &dest) {
    if(mat.channels() != 1) {
        throw runtime_error("Error: image provided is not grayscale");
    }

    for (int i = 0; i < mat.rows; ++i) {
        vector<uint8_t> row;
        for (int j = 0; j < mat.cols; ++j) {
            row.push_back(mat.at<uint8_t>(i, j));
        }
        dest.push_back(row);
    }
}

Mat gray_vector_to_mat(vector<vector<uint8_t>> vec) {
    Mat newmat(vec.size(), vec[0].size(), CV_8U);
    for (int i = 0; i < newmat.rows; i++) {
        for (int j = 0; j < newmat.cols; j++) {
            newmat.at<uint8_t>(i, j) = vec[i][j];
        }
    }
    return newmat;
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

vector<vector<uint8_t>> compute_seams(Mat energy_image) {
    vector<vector<uint8_t>> energy_matrix;
    vector<vector<int>> optimal_seams;

    int rows = energy_image.rows;
    int cols = energy_image.cols;

    cout << "rows:" << rows << "cols:" << cols << endl;

    gray_mat_to_vector(energy_image, energy_matrix);

    vector<vector<uint8_t>> reconstruct(rows, vector<uint8_t>(cols, 0));

    // Set energies of side edges to "infinity" so that seams avoid it
    // int last_col_index = energy_matrix[0].size() - 1;
    for(int i = 0; i < rows; i++) {
        energy_matrix[i][0] = numeric_limits<int>::max();
        energy_matrix[i][cols-1] = numeric_limits<int>::max();
    }
    
    cout << "recrow:" << reconstruct.size() << "reccols:" << reconstruct[0].size() << endl;
    cout << "erows:" << energy_matrix.size() << "ecols:" << energy_matrix[0].size() << endl;

    // Set top row optimal seams to same values as energy matrix, 
    // Initialize top row of seam reconstruction matrix to 0
    for(int i = 0; i < cols; i++) {
        optimal_seams[0][i] = energy_matrix[0][i];
        reconstruct[0][i] = 0;
    }

    // for(int i = 1; i < rows; i++) {
    //     for(int j = 0; j < cols; j++) {
    //         optimal_seams[i][j] = optimal_seams[i-1][j];
    //         reconstruct[i][j] = 0;
    //         if (optimal_seams[i-1][j-1] < optimal_seams[i-1][j]) {
    //             optimal_seams[i][j] = optimal_seams[i-1][j-1];
    //             reconstruct[i][j] = -1;
    //         } else if (optimal_seams[i-1][j+1] < optimal_seams[i-1][j]) {
    //             optimal_seams[i][j] = optimal_seams[i-1][j+1];
    //             reconstruct[i][j] = 1;
    //         }
    //         optimal_seams[i][j] += energy_matrix[i][j];
    //     }
    // }

    // int optimal_col;
    // for(int i = 1; i < cols; i++) {
    //     if (optimal_seams[rows-1][i] < optimal_col) {
    //         optimal_col = i;
    //     }
    // }

    return reconstruct;
}



int main(int argc, char** argv) {
    string image_path = "images/golf.jpg";
    Mat img = imread(image_path);//, IMREAD_GRAYSCALE);
    Mat edges;

    cout << "3D" << img.at<Vec3b>(100, 100) << endl;

    edges = convert_to_edges(img);

    // vector<vector<uint8_t>> edge_vec;
    // gray_mat_to_vector(edges, edge_vec);

    vector<vector<uint8_t>> seam_vec;
    seam_vec = compute_seams(edges);
    
    // Mat sobel = gray_vector_to_mat(edge_vec);

    // Mat image = imread("images/bikes.jpg", IMREAD_GRAYSCALE);

    // imshow("original", img);
    // imshow("converted", edges);

    waitKey(0);
}

