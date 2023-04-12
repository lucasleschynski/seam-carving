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

void gray_mat_to_vector(Mat mat, vector<vector<int>> &dest) {
    if(mat.channels() != 1) {
        throw runtime_error("Error: image provided is not grayscale");
    }

    for (int i = 0; i < mat.rows; ++i) {
        vector<int> row;
        for (int j = 0; j < mat.cols; ++j) {
            row.push_back(mat.at<int>(i, j));
        }
        dest.push_back(row);
    }
}

Mat gray_vector_to_mat(vector<vector<int>> vec) {
    Mat newmat(vec.size(), vec[0].size(), CV_8U);
    for (int i = 0; i < newmat.rows; i++) {
        for (int j = 0; j < newmat.cols; j++) {
            newmat.at<int>(i, j) = vec[i][j];
        }
    }
    return newmat;
}

template<typename T>
void print_1d_vector(const vector<T>& vec) {
    cout << "[";
    for (const auto& element : vec) {
        cout << element << " ";
    }
    cout << "]" << endl;
}

template<typename T>
void print_matrix(const vector<vector<T>>& matrix) {
    for (const auto& row : matrix) {
        cout << "[";
        for (const auto& element : row) {
            cout << element << " ";
        }
        cout << "]";
        cout << "\n" << endl;
    }
}

vector<int> compute_optimal_seam(Mat energy_image) {
    vector<vector<int>> energy_matrix;

    int rows = energy_image.rows;
    int cols = energy_image.cols;

    // cout << "rows:" << rows << "cols:" << cols << endl;

    gray_mat_to_vector(energy_image, energy_matrix);

    vector<vector<int>> optimal_seams(rows, vector<int>(cols, 0));
    vector<vector<int>> reconstruct(rows, vector<int>(cols, 0));

    // cout << "Int Max:" << numeric_limits<int>::max() <<  "Int Max:" << static_cast<unsigned int>(numeric_limits<uint8_t>::max()) << endl;

    // Set energies of side edges to "infinity" so that seams avoid it
    // int last_col_index = energy_matrix[0].size() - 1;
    for(int i = 0; i < rows; i++) {
        energy_matrix[i][0] = numeric_limits<int>::max();
        energy_matrix[i][cols-1] = numeric_limits<int>::max();
    }
    
    // cout << "recrow:" << reconstruct.size() << "reccols:" << reconstruct[0].size() << endl;
    // cout << "erows:" << energy_matrix.size() << "ecols:" << energy_matrix[0].size() << endl;

    // Set top row optimal seams to same values as energy matrix, 
    // Initialize top row of seam reconstruction matrix to 0
    for(int i = 0; i < cols; i++) {
        optimal_seams[0][i] = energy_matrix[0][i];
        reconstruct[0][i] = 0;
    }

    // Compute optimal seams from top to bottom
    for(int i = 1; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            optimal_seams[i][j] = optimal_seams[i-1][j];
            reconstruct[i][j] = 0;
            if (optimal_seams[i-1][j-1] < optimal_seams[i-1][j]) {
                optimal_seams[i][j] = optimal_seams[i-1][j-1];
                reconstruct[i][j] = -1;
            } else if (optimal_seams[i-1][j+1] < optimal_seams[i-1][j]) {
                optimal_seams[i][j] = optimal_seams[i-1][j+1];
                reconstruct[i][j] = 1;
            }
            optimal_seams[i][j] += energy_matrix[i][j];
        }
    }

    // Find "bottom root" of optimal seam
    int optimal_col;
    for(int i = 1; i < cols; i++) {
        if (optimal_seams[rows-1][i] < optimal_col) {
            optimal_col = i;
        }
    }

    // cout << "optcol:" << optimal_col << endl;
    // print_matrix(reconstruct);

    // Reconstruct optimal seam from bottom to top
    // Store seam as vector containing col index for each row 
    vector<int> seam_indices;//(rows,0);
    seam_indices.push_back(optimal_col);

    for(int i = rows-1; i > 0; i--) {
        optimal_col += reconstruct[i][optimal_col];
        seam_indices.push_back(optimal_col); 
    }

    return seam_indices;
}



int main(int argc, char** argv) {
    string image_path = "images/bikes.jpg";
    Mat img = imread(image_path);//, IMREAD_GRAYSCALE);
    cout << img.rows << img.cols << endl;
    // resize(img, img, Size(), 0.05, 0.05);
    Mat edges;

    cout << "3D" << img.at<Vec3b>(100, 100) << endl;

    edges = convert_to_edges(img);

    imshow("converted", edges);


    // vector<vector<uint8_t>> edge_vec;
    // gray_mat_to_vector(edges, edge_vec);

    vector<int> seam_vec;
    seam_vec = compute_optimal_seam(edges);

    print_1d_vector(seam_vec);

    cout << seam_vec.size() << endl;
    
    // Mat sobel = gray_vector_to_mat(edge_vec);

    // Mat image = imread("images/bikes.jpg", IMREAD_GRAYSCALE);

    // imshow("original", img);
    // imshow("converted", edges);

    waitKey(0);
}

