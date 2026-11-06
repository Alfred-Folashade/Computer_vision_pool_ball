#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace cv;
using namespace std;


// Read ground truth table from  csv 
unordered_map<string, unordered_map<string, Point>>
readGroundTruth(const string& filename) {

    unordered_map<string, unordered_map<string, Point>> locations;
    ifstream file(filename);
    string line;
    getline(file, line);

    while (getline(file, line)) {

        stringstream ss(line);
        string imageName, ballName, rowText, columnText;
        getline(ss, imageName, ',');
        getline(ss, ballName, ',');
        getline(ss, rowText, ',');
        getline(ss, columnText, ',');

        if (imageName.empty() || ballName.empty() ||
            rowText.empty() || columnText.empty()) {
            continue;
        }

        int row = stoi(rowText);
        int column = stoi(columnText);
        locations[imageName][ballName] = Point(column, row);
    }

    file.close();

    return locations;
}


void MyApplication() {
    auto groundTruth = readGroundTruth("BallLocations.csv");

    
    if (groundTruth.empty()) {
        cout << "No ground-truth coordinates were loaded!" << endl;
        return;
    }

    // Retrieve ground truth for Table6
    auto tableData = groundTruth.find("AllBalls.jpeg");

    if (tableData == groundTruth.end()) {
        cout << "No ground truth found for Table6!" << endl;
        return;
    }

    cout << "Ground truth for Table6:" << endl;

    for (const auto& ball : tableData->second) {
        cout << "Ball " << ball.first
             << ": (" << ball.second.x
             << ", " << ball.second.y << ")" << endl;
    }


    // Load image
    Mat image = imread("Table6.jpeg", IMREAD_COLOR);

    if (image.empty()) {
        cout << "Could not load image!" << endl;
        return;
    }

    // Original table corner coordinates
    Point2f source[4];

    source[0] = Point2f(337, 35);    // Top left
    source[1] = Point2f(1570, 21);   // Top right
    source[2] = Point2f(1888, 451);  // Bottom right
    source[3] = Point2f(38, 472);   // Bottom left

    // Coordinates in the transformed image
    Point2f destination[4];

    destination[0] = Point2f(0, 0);
    destination[1] = Point2f(1433, 0);
    destination[2] = Point2f(1433, 682);
    destination[3] = Point2f(0, 682);

    // Calculate perspective transformation
    Mat perspective_matrix =
        getPerspectiveTransform(source, destination);

    // Transform image
    Mat transformed;

    warpPerspective(
        image,
        transformed,
        perspective_matrix,
        Size(1434, 683)
    );

    // Convert to grayscale
    Mat gray_image;

    cvtColor(transformed, gray_image, COLOR_BGR2GRAY);

    // Detect circles
    vector<Vec3f> circles;

    HoughCircles(
        gray_image,
        circles,
        HOUGH_GRADIENT,
        2,
        43,
        300,
        20,
        20,
        23
    );

    // Draw detected circles
    Mat output = transformed.clone();

    for (const Vec3f& c : circles) {

        Point centre(cvRound(c[0]), cvRound(c[1]));

        int radius = cvRound(c[2]);

        circle(output, centre, radius, Scalar(0, 255, 0), 2);

        circle(output, centre, 2, Scalar(0, 0, 255), -1);
    }

    
    imwrite("transformed.jpg", transformed);
    imwrite("detected_circles.jpg", output);

    cout << "Images saved successfully!" << endl;

}


int main() {
    MyApplication();
    return 0;
}

// g++ Applications.cpp -o application
