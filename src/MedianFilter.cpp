#include <iostream>

#include <cstdarg>

#include <thread>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "DisplayImage.hpp"

void OrderNeighbourhood(double** neighbourhood){
    for(int i = 0; i < 225; i++){
        double minVal = (*neighbourhood)[i];
        int minIndex = i;

        // Finding the minimum form i until 121
        for(int j = i + 1; j < 225; j++){
            if((*neighbourhood)[j] < minVal){
                minVal = (*neighbourhood)[j];
                minIndex = j;
            }
        }

        // Swaping values, if the minimum value is not in place
        if(minIndex != i){
            double aux = (*neighbourhood)[i];
            (*neighbourhood)[i] = minVal;
            (*neighbourhood)[minIndex] = aux;
        }
    }
}

void CopyNeighbourhood(FFTProject::Image& image, double** neighbourhood, int i, int j){
    // Copying the neighbourhood to the array
    int index = 0;
    for(int x = i - 7; x <= i + 7; x++){
        for(int y = j - 7; y <= j + 7; y++){
            int indexX = (x >= 0) ? x : 0;
            int indexY = (y >= 0) ? y : 0;
            indexX = (indexX < image.cols) ? indexX : image.cols - 1;
            indexY = (indexY < image.rows) ? indexY : image.rows - 1;
            (*neighbourhood)[index] = image.at(indexX, indexY);
            index++;
        }
    }
}

FFTProject::Image MedianFilter(FFTProject::Image& image){
    FFTProject::Image out(image.rows, image.cols);
    double* neighbourhood;
    neighbourhood = static_cast<double*>(malloc(sizeof(double) * 225));

    for(int i = 0; i < image.cols; i++){
        for(int j = 0; j < image.rows; j++){

            CopyNeighbourhood(image, &neighbourhood, i, j);
            OrderNeighbourhood(&neighbourhood);

            out.at(i, j) = static_cast<double>(neighbourhood[24]);
        }
    }

    // For some reason this causes SIGABRT
    free(neighbourhood);

    return out;
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
    cv::resize(image, image, cv::Size(), 4.0, 4.0); // This method is way faster

    cv::Mat outImage(image.rows, image.cols, image.type());
    FFTProject::Image result1, result2;

    std::thread t1 = std::thread([&] {
        FFTProject::Image img(image);
        result1 = MedianFilter(img);
    });

    std::thread t2 = std::thread([&] {
        FFTProject::Image img(image);
        result2 = MedianFilter(img);
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

    cv::resize(outImage, outImage, cv::Size(), 0.25, 0.25); // This method is way faster
    //FFTProject::DisplayImage(image);
    cv::imwrite(outputfile, outImage);

    return 0;
}
