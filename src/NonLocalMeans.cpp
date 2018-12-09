#include <iostream>

#include <thread>

#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "DisplayImage.hpp"

unsigned int B(FFTProject::Image& img, int x, int y){
    unsigned int neighborhoodSize = 0;  // Is incremented after each iteration
    unsigned int sum = 0;

    for(int i = -2; i <= 2; i++){
        for(int j = -2; j <= 2; j++){
            if(x + i < img.cols && x + i >= 0){
                if(y + j < img.rows && y + j >= 0){
                    sum += img.at(x + i, y + j);
                    neighborhoodSize++;
                }
            }
        }
    }

    return sum / neighborhoodSize;
}

double f(FFTProject::Image& bs, int x1, int y1, int x2, int y2){
    double exponent = pow(bs.at(x1, y1) - bs.at(x2, y2), 2) / 25.0;

    return exp(-exponent);
}

float C(FFTProject::Image& img, FFTProject::Image& bs, int x, int y){
    float sum = 0.0;

    for(int i = 0; i < img.cols; i++){
        for(int j = 0; j < img.rows; j++){
            sum += f(bs, x, y, i, j);
        }
    }

    return sum;
}

float u(FFTProject::Image& img, FFTProject::Image& bs, int x, int y){
    float sum = 0.0;

    for(int i = 0; i < img.cols; i++){
        for(int j = 0; j < img.rows; j++){
            sum += img.at(i, j) * f(bs, x, y, i, j);
        }
    }

    return sum / C(img, bs, x, y);
}


FFTProject::Image NonLocalMeans(FFTProject::Image& img, FFTProject::Image& bs){
    FFTProject::Image out(img.rows, img.cols);
    for(int x = 0; x < img.cols; x++){
        for(int y = 0; y < img.rows; y++){
            out.at(x, y) = static_cast<unsigned char>(u(img, bs, x, y));
        }
    }

    return out;
}

///
/// \brief AddNoiseToImage adds white noise to img
/// \param img
///
void AddNoiseToImage(cv::Mat& img){
    int totalPoints = 8192;

    srand(time(NULL));

    for(int i = 0; i < totalPoints; i++){
        int xCoord = rand() % img.cols;
        int yCoord = rand() % img.rows;
        int noiseVal = rand() % 48;

        img.at<unsigned char>(xCoord, yCoord) += noiseVal;
    }

    cv::imwrite("noisyImg-" + std::to_string(totalPoints) + ".tif", img);
}

// && because I'm lazy to figure out the best way to copy "local bs" to "main bs"
FFTProject::Image&& CalculateBs(FFTProject::Image& img){
    FFTProject::Image bs(img.rows, img.cols);
    for(int i = 0; i < img.cols; i++){
        for(int j = 0; j < img.rows; j++){
            bs.at(i, j) = static_cast<unsigned char>(B(img, i, j));
        }
    }

    return std::move(bs);
}

int main(int argc, char** argv){
    char* inputFile;
    char* outputfile;

    if(argc == 3){
        inputFile = argv[1];
        outputfile = argv[2];
    } else{
        std::cerr << "Wrong number of arguments" << std::endl;
        return -1;
    }

    cv::Mat image;
    image = cv::imread(inputFile, cv::IMREAD_GRAYSCALE);
    image.convertTo(image, CV_8U);
    cv::Mat outImage(image.rows, image.cols, image.type());

    FFTProject::Image result1, result2;

    std::thread t1 = std::thread([&]{
        FFTProject::Image img(image);
        FFTProject::Image bs(CalculateBs(img));
        result1 = NonLocalMeans(img, bs);
    });

    std::thread t2 = std::thread([&]{
        FFTProject::Image img(image);
        FFTProject::Image bs(CalculateBs(img));
        result2 = NonLocalMeans(img, bs);
    });

    t1.join(); t2.join();

    int choice1, choice2;

    std::thread t3 = std::thread([&]{
        cv::Mat res = image;
        double psnr11, psnr12;

        result1.toMat(res);
        psnr11 = FFTProject::getPSNR(image, res);

        result2.toMat(res);
        psnr12 = FFTProject::getPSNR(image, res);

        choice1 = (psnr11 >= psnr12) ? 1 : 2;
    });

    std::thread t4 = std::thread([&]{
        cv::Mat res = image;
        double psnr21, psnr22;

        result1.toMat(res);
        psnr21 = FFTProject::getPSNR(image, res);
        result2.toMat(res);
        psnr22 = FFTProject::getPSNR(image, res);

        choice2 = (psnr21 >= psnr22) ? 1 : 2;
    });

    t3.join(); t4.join();

    if(choice1 != choice2){
        // Compares the PSNRs for a third time
        cv::Mat res = image;
        double psnr31, psnr32;

        result1.toMat(res);
        psnr31 = FFTProject::getPSNR(image, res);
        result2.toMat(res);
        psnr32 = FFTProject::getPSNR(image, res);

        if(psnr31 >= psnr32){
            result1.toMat(outImage);
        } else{
            result2.toMat(outImage);
        }
    } else if(choice1 == 1){
        result1.toMat(outImage);
    } else{
        result2.toMat(outImage);
    }

    cv::imwrite(outputfile, outImage);

    return 0;
}
