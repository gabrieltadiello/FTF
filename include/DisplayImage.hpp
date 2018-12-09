#pragma once

#include <string>
#include <exception>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <memory>

using namespace cv;

namespace FFTProject{

int DisplayImage(std::string filename);
int DisplayImage(cv::Mat& img, const char* windowName = "Display window");
double getPSNR(const Mat& I1, const Mat& I2);
double** Invert(double** a, unsigned long rowSize);

struct Img{
    double* data;
    int rows;
    int cols;
};

struct ImgCols{
    double** data;
    int rows;
    int cols;
};

class Image{

private:
    double* data;
    double** colsData;
    bool organizedPerCol;

public:
    int rows;
    int cols;

    Image(void){
        rows = 0;
        cols = 0;

        data = NULL;
        colsData = NULL;
    }

    Image(int _rows, int _cols){
        rows = _rows;
        cols = _cols;
        organizedPerCol = false;

        data = static_cast<double*>(malloc(sizeof(double) * rows * cols));
        colsData = NULL;

        // memcpy() would be faster, but I'm lazy
        for(int i = 0; i < rows * cols; i++){
            data[i] = 0.0;
        }
    }

    Image(double** _data, int _rows, int _cols){
        rows = _rows;
        cols = _cols;
        organizedPerCol = false;

        data = static_cast<double*>(malloc(sizeof(double) * rows * cols));
        colsData = NULL;

        for(int i = 0; i < cols; i++){
            for(int j = 0; j < rows; j++){
                data[i * cols + j] = _data[j][i];
            }
        }
    }

    Image(cv::Mat& img, bool isDouble = false){
        rows = img.rows;
        cols = img.cols;
        organizedPerCol = false;

        data = static_cast<double*>(malloc(sizeof(double) * rows * cols));
        colsData = NULL;

        // memcpy() would be faster, but I'm lazy
        for(int i = 0; i < cols; i++){
            for(int j = 0; j < rows; j++){
                if(isDouble){
                    at(i, j) = img.at<double>(i, j);
                } else{
                    at(i, j) = img.at<unsigned char>(i, j);
                }
            }
        }
    }

    Image(Image& img){
        rows = img.rows;
        cols = img.cols;
        organizedPerCol = img.organizedPerCol;

        if(organizedPerCol){
            colsData = static_cast<double**>(malloc(sizeof(double*) * cols));

            for(int i = 0; i < cols; i++){
                colsData[i] = static_cast<double*>(malloc(sizeof(double) * rows));
            }

            data = NULL;
        } else{
            data = static_cast<double*>(malloc(sizeof(double) * rows * cols));
            colsData = NULL;
        }

        for(int i = 0; i < cols; i++){
            for(int j = 0; j < rows; j++){
                at(i, j) = img.at(i, j);
            }
        }
    }

    Image(Image&& img){
        rows = img.rows;
        cols = img.cols;
        data = img.data;
        colsData = img.colsData;
        organizedPerCol = img.organizedPerCol;
    }

    Image& operator=(Image img){
        rows = img.rows;
        cols = img.cols;
        organizedPerCol = img.organizedPerCol;

        if(organizedPerCol){
            colsData = static_cast<double**>(malloc(sizeof(double*) * cols));

            for(int i = 0; i < cols; i++){
                colsData[i] = static_cast<double*>(malloc(sizeof(double) * rows));
            }

            data = NULL;
        } else{
            data = static_cast<double*>(malloc(sizeof(double) * rows * cols));
            colsData = NULL;
        }

        for(int i = 0; i < cols; i++){
            for(int j = 0; j < rows; j++){
                at(i, j) = img.at(i, j);
            }
        }
    }

    Image& operator=(Img img){
        ReleaseMemory();
        rows = img.rows;
        cols = img.cols;
        organizedPerCol = false;
        data = img.data;
        return *this;
    }

    Image& operator=(ImgCols img){
        ReleaseMemory();
        rows = img.rows;
        cols = img.cols;
        organizedPerCol = true;
        colsData = img.data;
    }

    ~Image(void){
        // Why should we care about memory leak in a FFT project? : )
        // This would cause the application to crash when it tries to execute Image(Image&& img).
        // free(data);
    }

    void ReleaseMemory(void){
        if(data){
            free(data);
        }

        if(colsData){
            for(int i = 0; i < rows; i++){
                free(colsData[i]);
            }

            free(colsData);
        }

        data = NULL;
        colsData = NULL;
    }

    double* getData(void){
        return data;
    }

    double** getColData(void){
        return colsData;
    }

    double& at(int x, int y){
        if(organizedPerCol){
            return colsData[x][y];
        } else{
            return data[x * cols + y];
        }
    }

    double** everyColumn(void){
        double** result = static_cast<double**>(malloc(sizeof(double*) * cols));

        for(int i = 0; i < cols; i++){
            result[i] = static_cast<double*>(malloc(sizeof(double) * rows));

            if(organizedPerCol){
                memcpy(result[i], colsData[i], rows);
            } else{
                for(int j = 0; j < rows; j++){
                    result[i][j] = at(j, i);
                }
            }
        }

        return result;
    }

    double** everyRow(void){
        double** result = static_cast<double**>(malloc(sizeof(double*) * rows));

        for(int i = 0; i < rows; i++){
            result[i] = static_cast<double*>(malloc(sizeof(double) * cols));

            if(organizedPerCol){
                double* d = data;
                for(int j = 0; j < cols; j++, d += cols){
                    result[i][j] = *d;
                }
            } else{
                for(int j = 0; j < cols; j++){
                    double val = data[j * cols + i];
                    result[i][j] = val;
                }
            }
        }

        return result;
    }

    // Expects an CV_8U image
    void toMat(cv::Mat& img){
        for(int i = 0; i < cols; i++){
            for(int j = 0; j < rows; j++){
                unsigned char pixelVal = static_cast<unsigned char>(at(i, j));

                img.at<unsigned char>(i, j) = pixelVal;
            }
        }
    }

    void toDoubleMat(cv::Mat& img){
        for(int i = 0; i < cols; i++){
            for(int j = 0; j < rows; j++){
                double val = at(i, j);
                img.at<double>(i, j) = val;
            }
        }
    }

};

void CompareOutputs(Image& data1, Image& data2, char *detectLog);


}
