#include <iostream>
#include <fstream>

#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include <thread>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "DisplayImage.hpp"

void CopyNeighbourhood(FFTProject::Image& image, double** neighbourhood, int i, int j){
    // Copying the neighbourhood to the array
    int index = 0;
    int n = 18;
    for(int x = i - n; x <= i + n; x++){
        for(int y = j - n; y <= j + n; y++){
            int indexX = (x >= 0) ? x : 0;
            int indexY = (y >= 0) ? y : 0;
            indexX = (indexX < image.cols) ? indexX : image.cols - 1;
            indexY = (indexY < image.rows) ? indexY : image.rows - 1;
            (*neighbourhood)[index] = image.at(indexX, indexY);
            index++;
        }
    }
}

double Mean(double* neighbourhood, int neighbourhoodSize){
    double mean = 0.0;

    for(int i = 0; i < neighbourhoodSize; i++){
        mean += neighbourhood[i];
    }

    return mean / neighbourhoodSize;
}

double Variance(double* neighbourhood, double mean, int neighbourhoodSize){
    double variance = 0.0;
    double meanSquared = mean * mean;

    for(int i = 0; i < neighbourhoodSize; i++){
        variance += (neighbourhood[i] * neighbourhood[i]) - meanSquared;
    }

    return variance / neighbourhoodSize;
}

void AdaptiveWienerFilter(FFTProject::Image& image, FFTProject::Image& outImage, double noiseVariance){
    FFTProject::Img result;
    result.cols = image.cols;
    result.rows = image.rows;
    int neighbourhoodSize = 1369;
    double* neighbourhood = static_cast<double*>(malloc(sizeof(double) * static_cast<unsigned long>(neighbourhoodSize)));

    result.data = static_cast<double*>(malloc(sizeof(double) * static_cast<unsigned long>(image.rows * image.cols)));

    for(int i = 0; i < image.cols; i++){
        for(int j = 0; j < image.rows; j++){
            CopyNeighbourhood(image, &neighbourhood, i, j);
            double mean = Mean(neighbourhood, neighbourhoodSize);
            double variance = Variance(neighbourhood, mean, neighbourhoodSize);
            double numerator = (variance - noiseVariance > 0) ? variance - noiseVariance : 0;
            double denominator = (variance > noiseVariance) ? variance : noiseVariance;

            result.data[i * image.rows + j] = mean + (numerator / denominator) * (image.at(i, j) - mean);
        }
    }

    free(neighbourhood);

    outImage = result;
}

void AdaptiveWienerFilter(FFTProject::Image& image, FFTProject::Image& out){
    //int size = image.rows * image.cols;
    //double mean = Mean(image.getData(), size);

    /*
    if(newVariance){
        srand(time(NULL));
        double randNums[4096];

        for(int i = 0; i < 4096; i++){
            randNums[i] = static_cast<double>(rand() % 48);
        }

        double mean = Mean(randNums, 4096);
        variance = Variance(randNums, mean, 4096);

        std::cout << variance << std::endl;

        throw;
        std::ofstream var("var.txt", std::ofstream::out);
        var << variance;
        var.close();
    } else{
        std::ifstream var("var.txt", std::ofstream::out);
        var >> variance;
        var.close();
    }
    */
    double variance = 188.899;
    AdaptiveWienerFilter(image, out, variance);
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
    cv::resize(image, image, cv::Size(), 8.0, 8.0);

    cv::Mat outImage(image.rows, image.cols, image.type());
    FFTProject::Image result1(image.rows, image.cols), result2(image.rows, image.cols);

    std::thread t1 = std::thread([&] {
        FFTProject::Image img(image);
        AdaptiveWienerFilter(img, result1);
    });

    std::thread t2 = std::thread([&] {
        FFTProject::Image img(image);
        AdaptiveWienerFilter(img, result2);
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

    cv::resize(outImage, outImage, cv::Size(), 0.125, 0.125);
    cv::imwrite(outputfile, outImage);

    return 0;
}
