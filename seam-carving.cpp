#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;
using namespace cv;

Mat create_energy_image(Mat &image, bool show_energy_image) {
    Mat blur, gray, grad_x, grad_y, abs_grad_x, abs_grad_y, grad, energy_image;
    int ddepth = CV_16S;

    GaussianBlur(image, blur, Size(3,3), 0, 0, BORDER_DEFAULT);
    cvtColor(blur, gray, COLOR_BGR2GRAY);
    
    // use Scharr operator to calculate the gradient of the image in the x and y direction
    Scharr(gray, grad_x, ddepth, 1, 0);//, scale, delta, BORDER_DEFAULT);
    Scharr(gray, grad_y, ddepth, 0, 1);//, scale, delta, BORDER_DEFAULT);

    // convert gradients to absolute versions of themselves
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);
    
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
    
    // convert the default values to double precision
    grad.convertTo(energy_image, CV_64F, 1.0/255.0);
    
    // create and show the newly created energy image
    if (show_energy_image) {
        namedWindow("Energy Image", WINDOW_AUTOSIZE); imshow("Energy Image", energy_image);
    }
    
    return energy_image;
}

Mat create_total_energy_map(Mat &energy_image, bool show_colored_map) {
    double a,b,c;
    int rows = energy_image.rows;
    int cols = energy_image.cols;
    
    // initialize the map with zeros
    Mat total_energy_image = Mat(rows, cols, CV_64F, double(0));
    
    // copy the first row
    energy_image.row(0).copyTo(total_energy_image.row(0));
    
    // take the minimum of the three neighbors and add to total, this creates a running sum which is used to determine the lowest energy path
    for (int m = 1; m < rows; m++) {
        for (int n = 0; n < cols; n++) {
            a = total_energy_image.at<double>(m - 1, max(n - 1, 0));
            b = total_energy_image.at<double>(m - 1, n);
            c = total_energy_image.at<double>(m - 1, min(n + 1, cols - 1));

            total_energy_image.at<double>(m, n) = energy_image.at<double>(m, n) + min(a, min(b, c));
        }
    }

    // create and show colored energy map
    if (show_colored_map) {
        Mat color_energy_map;
        double Cmin;
        double Cmax;
        minMaxLoc(total_energy_image, &Cmin, &Cmax);
        float scale = 255.0 / (Cmax - Cmin);
        // scale all values in energy map to 8-bit range and copy to color_energy_map
        total_energy_image.convertTo(color_energy_map, CV_8UC1, scale);
        // apply color map to color_energy_map to visualise total energy for all pixels
        applyColorMap(color_energy_map, color_energy_map, COLORMAP_TURBO);
        namedWindow("Cumulative Energy Map", WINDOW_AUTOSIZE); imshow("Cumulative Energy Map", color_energy_map);
    }
    
    return total_energy_image;
}

vector<int> find_optimal_seam(Mat& energy_map) {
    vector<int> seam;
    double a,b,c;
    double min_val, max_val;
    Point min_pt, max_pt;

    int rows = energy_map.rows;
    int cols = energy_map.cols;

    Mat last_row = energy_map.row(rows - 1);

    // Get optimal column (bottom row pixel with lowest energy) for reconstruction
    minMaxLoc(last_row, &min_val, &max_val, &min_pt, &max_pt);
    int optimal_col = min_pt.x;

    // init seam vector
    seam.resize(rows);
    seam[rows-1] = optimal_col;

    // reconstruct optimal seam based on energy map
    // works from bottom to top, taking the pixel above (either up left, up center, or up right) with minimum energy and adding it to seam
    for (int i = rows - 2; i >= 0; i--) {
        a = energy_map.at<double>(i, max(optimal_col - 1, 0));
        b = energy_map.at<double>(i, optimal_col);
        c = energy_map.at<double>(i, min(optimal_col + 1, cols - 1));
        
        double minimum = min(min(a,b), c);
        int direction = (minimum == a) ? -1 : ((minimum == b) ? 0 : 1);
        
        optimal_col += direction;
        optimal_col = min(max(optimal_col, 0), cols - 1); // Handle edge(literally) cases
        seam[i] = optimal_col;
    }
    return seam;
}

void highlight_seam(Mat &energy, vector<int> seam) {
    for (int i = 0; i < energy.rows; i++) {
        energy.at<double>(i,seam[i]) = 1;
    }
    namedWindow("Seam on Energy Image", WINDOW_AUTOSIZE); imshow("Seam on Energy Image", energy);
}

Mat remove_seam(Mat& image, vector<int> seam) {
    int rows = image.rows;
    int cols = image.cols;

    if (seam.size() != rows) {
        throw runtime_error("Seam vector size does not match the number of rows in the image.");
    }

    // Check if the column indices in the seam vector are valid
    for (int i = 0; i < seam.size(); i++) {
        if (seam[i] < 0 || seam[i] > cols - 1) {
            throw runtime_error("Invalid column index in seam vector.");
        }
    }

    Mat edited_image(rows, cols-1, CV_8UC3);
    for (int i = 0; i < rows; i++) {
        int edited_col_index = 0;
        for (int j = 0; j < cols; j++) {
            if(j != seam[i]) {
                edited_image.at<Vec3b>(i,edited_col_index) = image.at<Vec3b>(i,j);
                edited_col_index++;
            }
        }
    }
    image = edited_image;
    return image;
}

Mat resize_image(Mat image, int shrink) {
    Mat copy = image.clone();
    for (int i = 0; i < shrink; i++) {
        Mat energy = create_energy_image(copy, false);
        Mat map = create_total_energy_map(energy, true);
        vector<int> seam = find_optimal_seam(map);
        remove_seam(copy, seam);
    }
    return copy;
}

int main() {
    string image_path = "images/valve.png";
    Mat img = imread(image_path);
    imshow("original", img);
    img = resize_image(img, 100);

    imshow("resized", img);

    waitKey(0);
}