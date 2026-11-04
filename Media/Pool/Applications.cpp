#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

void MyApplication() {

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
    source[3] = Point2f(38, 472);    // Bottom left

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

    // Display results
    imwrite("transformed.jpg", transformed);
    imwrite("detected_circles.jpg", output);

    cout << "Images saved successfully!" << endl;
    
}

int main() {
    MyApplication();
    return 0;
}