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

    if (!file.is_open()) {
        cout << "Could not open CSV file!" << endl;
        return locations;
    }

    string line;
    getline(file, line);
    getline(file, line);

    vector<string> ballNames = {
        "1", "2", "3", "4", "5",
        "6", "7", "8", "9", "10",
        "11", "12", "13", "14", "15",
        "White"
    };

    while (getline(file, line)) {
        stringstream ss(line);
        vector<string> values;
        string value;

        while (getline(ss, value, ',')) {
            values.push_back(value);
        }

        if (values.empty() || values[0].empty()) {
            continue;
        }

        string imageName = values[0];

        for (int i = 0; i < ballNames.size(); i++) {
            int rowIndex = 1 + (i * 2);
            int columnIndex = rowIndex + 1;
            if (columnIndex >= values.size()) {
                continue;
            }

            string rowText = values[rowIndex];
            string columnText = values[columnIndex];

            if (rowText.empty() || columnText.empty()) {
                continue;
            }

            int row = stoi(rowText);
            int column = stoi(columnText);
            locations[imageName][ballNames[i]] =
                Point(column, row);
        }
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

    auto tableData = groundTruth.find("Table6.jpeg");

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

    vector<Point> detectedCentres;

    for (const Vec3f& c : circles) {

        Point centre(cvRound(c[0]), cvRound(c[1]));

        detectedCentres.push_back(centre);
    }


    vector<Point> actualCentres;

    for (const auto& ball : tableData->second) {

        actualCentres.push_back(ball.second);
    }

    cout << "\nDetected centres:" << endl;

    for (const Point& p : detectedCentres) {
        cout << "(" << p.x << ", " << p.y << ")" << endl;
    }

    cout << "\nActual centres:" << endl;
    for (const Point& p : actualCentres) {
        cout << "(" << p.x << ", " << p.y << ")" << endl;
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
// ./myprogram