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
        cout << static_cast<int>(element) << " ";
    }
    cout << "]" << endl;
}

template<typename T>
void print_matrix(const vector<vector<T>>& matrix) {
    for (const auto& row : matrix) {
        cout << "[";
        for (const auto& element : row) {
            cout << static_cast<int>(element) << " ";
        }
        cout << "]";
        cout << "\n" << endl;
    }
}

Mat convert_to_edges(Mat& src) {
    int ddepth = CV_32F;
    Mat output, gray, grad_x, grad_y, energy_image;

    cvtColor(src, gray, COLOR_BGR2GRAY);

    GaussianBlur(gray, output, Size(3, 3), 0);

    Sobel(output, grad_x, ddepth, 1, 0, 3);  // Sobel for X direction
    Sobel(output, grad_y, ddepth, 0, 1, 3);  // Sobel for Y direction

    // Scale
    // convertScaleAbs(grad_x, grad_x);
    // convertScaleAbs(grad_y, grad_y);

    // vector<vector<int>> grad;
    // gray_mat_to_vector(grad_x, grad);
    // print_matrix(grad);

    Mat mag;
    magnitude(grad_x, grad_y, mag);

    normalize(mag, output, 0, 255, cv::NORM_MINMAX, CV_8U);

    // Combine X and Y gradients
    // addWeighted(grad_x, 0.5, grad_y, 0.5, 0, output);
    // cout << "output depth" << output.depth() << endl;
    // output.convertTo(output, CV_16F, 1.0/255.0);
    // GaussianBlur(output, output, Size(5, 5), 0);
    

    return output;
}

vector<int> compute_optimal_seam(Mat energy_image) {
    vector<vector<uint8_t>> energy_matrix;

    int rows = energy_image.rows;
    int cols = energy_image.cols;

    gray_mat_to_vector(energy_image, energy_matrix);

    // print_matrix(energy_matrix);

    vector<vector<int>> optimal_seams(rows, vector<int>(cols, 0));
    vector<vector<signed char>> reconstruct(rows, vector<signed char>(cols, 0));

    // Set energies of side edges to "infinity" so that seams avoid it
    // int last_col_index = energy_matrix[0].size() - 1;
    for(int i = 0; i < rows; i++) {
        energy_matrix[i][0] = numeric_limits<uint8_t>::max();
        energy_matrix[i][cols-1] = numeric_limits<uint8_t>::max();
    }

    // Set top row optimal seams to same values as energy matrix, 
    // Initialize top row of seam reconstruction matrix to 0
    // print_1d_vector(energy_matrix[100]);
    for(int i = 0; i < cols; i++) {
        optimal_seams[0][i] = energy_matrix[0][i];
        reconstruct[0][i] = 0;
    }

    // cout << "energy matrix: " << endl;
    // Compute optimal seams from top to bottom
    for(int i = 1; i < rows; i++) {
        for(int j = 1; j < cols-1; j++) {
            optimal_seams[i][j] = optimal_seams[i-1][j];
            reconstruct[i][j] = 0;
            if (optimal_seams[i-1][j-1] < optimal_seams[i][j]) {
                optimal_seams[i][j] = optimal_seams[i-1][j-1];
                reconstruct[i][j] = -1;
            } else if (optimal_seams[i-1][j+1] < optimal_seams[i][j]) {
                optimal_seams[i][j] = optimal_seams[i-1][j+1];
                reconstruct[i][j] = 1;
            }
            optimal_seams[i][j] += energy_matrix[i][j];
        }
    }

    // Define gradient color mapping
    int minValue = 0;
    int maxValue = 4000;
    cv::Vec3b minColor(255, 0, 0);  // Blue color
    cv::Vec3b maxColor(0, 0, 255); // Red color

    // Create an image with the same size as the vector of vectors
    // int rows = static_cast<int>(optimal_seams.size());
    // int cols = static_cast<int>(optimal_seams[0].size());
    cv::Mat image(rows, cols, CV_8UC3);

    // Iterate through the elements of the vector of vectors
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int value = optimal_seams[i][j];

            // Interpolate color between minColor and maxColor based on value
            double alpha = static_cast<double>(value - minValue) / (maxValue - minValue);
            Vec3b color = minColor * (1 - alpha) + maxColor * alpha;

            image.at<Vec3b>(i, j) = color;
        }
    }

    // Display the resulting image
    imshow("Image", image);

    // print_matrix(optimal_seams);
    // print_1d_vector(optimal_seams[rows-1]);

    // Find "bottom root" of optimal seam
    int optimal_col = 2;
    for(int i = optimal_col; i < cols-2; i++) {
        if (optimal_seams[rows-1][i] < optimal_seams[rows-1][optimal_col]) {
            optimal_col = i;
        }
    }

    cout << optimal_col << endl;
    // print_matrix(reconstruct);

    // vector<int> left_edge;
    // for(int i = 0; i < rows; i ++) {
    //     left_edge.push_back(reconstruct[i][0]);
    // }

    // print_1d_vector(left_edge);

    // Reconstruct optimal seam from bottom to top
    // Store seam as vector containing col index for each row 
    vector<int> seam_indices(rows,0);
    seam_indices[rows-1] = optimal_col;

    for(int i = rows-1; i > 0; i--) {
        optimal_col += reconstruct[i][optimal_col];
        seam_indices[i-1] = optimal_col;
    }

    return seam_indices;
}

Mat remove_seam(Mat image, vector<int> seam) {
    // Check if the size of the seam vector matches the number of rows in the image
    if (seam.size() != image.rows) {
        throw runtime_error("Seam vector size does not match the number of rows in the image.");
    }

    // Check if the column indices in the seam vector are valid
    for (int i = 0; i < seam.size(); i++) {
        if (seam[i] < 0 || seam[i] > image.cols - 1) {
            throw runtime_error("Invalid column index in seam vector.");
        }
    }

    Mat edited_image(image.rows, image.cols-1, CV_8UC3);
    for (int i = 0; i < image.rows; i++) {
        int edited_col_index = 0;
        for (int j = 0; j < image.cols; j++) {
            if(j != seam[i]) {
                edited_image.at<Vec3b>(i,edited_col_index) = image.at<Vec3b>(i,j);
                edited_col_index++;
            }
        }
    }
    return edited_image;
}


Mat highlight_seam(Mat image, vector<int> seam) {
    Mat edited_image(image.rows, image.cols, CV_8UC3);
    Vec3b red_pixel(0, 0, 255);
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            if(j != seam[i]) {
                edited_image.at<Vec3b>(i,j) = image.at<Vec3b>(i,j);
            } else {
                edited_image.at<Vec3b>(i,j) = red_pixel;
            }
        }
    }
    return edited_image;
}

void on_trackbar(int value, int& track_var, void* user_data) {
    track_var = value;
}

Mat resize(Mat image, int shrink) {
    Mat copy = image.clone();
    for(int i = 0; i < shrink; i++) {
        Mat edges = convert_to_edges(copy);
        
        vector<int> seam_vec = compute_optimal_seam(edges);
        // print_1d_vector(seam_vec);
        string e = "energy" + to_string(i);
        if(i % 10 == 0) {
            // Mat highlight = copy.clone();
            // highlight = highlight_seam(highlight, seam_vec);
            // imshow(e, highlight);
            // imshow(e+"edge", edges);
            // print_1d_vector(seam_vec);
        }
        // print_1d_vector(seam_vec);
        copy = remove_seam(copy, seam_vec);
    }
    return copy;
}


int main(int argc, char** argv) {
    string image_path = "images/bikes.jpg";
    Mat img = imread(image_path);
    cout << "depth: " << img.depth() << endl;

    // img.convertTo(img, CV_16U);
    // cout << "depth: " << img.depth() << endl;
    // resize(img, img, Size(), 0.1, 0.1);
    // imshow("resized", img);
    // namedWindow("Seam Carving");
    // int trackbar_value = 0;
    // createTrackbar("Seams removed", "Seam Carving", &trackbar_value, img.cols, on_trackbar);

    Mat edges = convert_to_edges(img);
    // vector<vector<uint8_t>> edge_mat;
    // gray_mat_to_vector(edges, edge_mat);
    // print_matrix(edge_mat);

    imshow("edge", edges);

    vector<int> seam_vec = compute_optimal_seam(edges);

    // print_1d_vector(seam_vec);

    // Mat edit = remove_seam(img, seam_vec);
    // Mat edit = highlight_seam(img, seam_vec);
    Mat shrunk = resize(img, 200);
    // imshow("seam", edit);
    // // imshow("original", img);
    imshow("converted", shrunk);

    waitKey(0);
}

