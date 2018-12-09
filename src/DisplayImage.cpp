#include <iostream>

#include <string>

#include <exception>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "DisplayImage.hpp"

using namespace cv;

int FFTProject::DisplayImage(std::string filename){
    cv::Mat image;
    image = cv::imread(filename, cv::IMREAD_GRAYSCALE);   // Read the file

    if(!image.data ){
        std::cerr <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
    cv::imshow( "Display window", image);                   // Show our image inside it.

    cv::waitKey(0);                                          // Wait for a keystroke in the window

    return 0;
}

int FFTProject::DisplayImage(cv::Mat& img, const char* windowName){
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE );// Create a window for display.
    cv::imshow(windowName, img);                   // Show our image inside it.

    cv::waitKey(0);                                          // Wait for a keystroke in the window

    return 0;
}

double FFTProject::getPSNR(const Mat& I1, const Mat& I2)
{
    Mat s1;
    absdiff(I1, I2, s1);       // |I1 - I2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |I1 - I2|^2

    Scalar s = sum(s1);        // sum elements per channel

    double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

    if( sse <= 1e-10) // for small values return zero
        return 0;
    else
    {
        double mse  = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }
}

double** FFTProject::Invert(double** a, unsigned long rowSize){
    double** b = static_cast<double**>(malloc(sizeof(double*) * rowSize));

    for(int i = 0; static_cast<unsigned long>(i) < rowSize; i++){
        b[i] = static_cast<double*>(malloc(sizeof(double) * rowSize));
        for(int j = 0; static_cast<unsigned long>(j) < rowSize; j++){
            b[i][j] = a[j][i];
        }
    }

    return b;
}

void FFTProject::CompareOutputs(FFTProject::Image& data1, FFTProject::Image& data2, char *detectLog){
    FILE *fp;

    for(int i = 0; i < data1.cols; i++){
        for(int j = 0; j < data1.rows; j++){
            if(abs(data1.at(j, i) - data2.at(j, i)) < 0.01){
                if((fp = fopen(detectLog, "a"))){
                    fprintf(fp, "[%d, %d]: %f, %f", j, i, data1.at(j, i), data2.at(j, i));
                    fclose(fp);
                }
            }
        }
    }
}
