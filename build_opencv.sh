# !/bin/bash

if [ $# -ne 1 ]; then
    printf "Wrong script usage! You must enter the number of threads to execute make.\nExample, using 4 threads: %s 4\n" "$0"
else
    # Installing dependencies (from http://www.codebind.com/cpp-tutorial/install-opencv-ubuntu-cpp/ )
    sudo apt-get install build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
    sudo apt-get install python3.5-dev python3-numpy libtbb2 libtbb-dev
    sudo apt-get install libjpeg-dev libpng-dev libtiff5-dev libjasper-dev libdc1394-22-dev libeigen3-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev sphinx-common libtbb-dev yasm libfaac-dev libopencore-amrnb-dev libopencore-amrwb-dev libopenexr-dev libgstreamer-plugins-base1.0-dev libavutil-dev libavfilter-dev libavresample-dev

    # Creates opencv directory if it does not exists
    if [ ! -d "opencv" ]; then
        mkdir opencv
    fi
    cd opencv

    # Cloning contrib repo; not sure if necessary
    # if [ ! -d "opencv_contrib" ]; then
    #     git clone https://github.com/opencv/opencv_contrib.git
    # fi
        
    # Clones the opencv sources if the opencv directory does not exists
    if [ ! -d "opencv" ]; then
        git clone https://github.com/opencv/opencv.git
        cd opencv
        git checkout 2.4
    else
        cd opencv
    fi
    
    
    # Only runs cmake if build directory does not exists
    if [ ! -d "build" ]; then
        mkdir build && cd build
        cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
    else
        cd build
    fi

    # Actually compilling the source; there's probably a better way to get the amount of threads...
    if [ $1 -eq 1 ]; then
        sudo make install
    elif [ $1 -eq 2 ]; then
        sudo make install -j2
    elif [ $1 -eq 3 ]; then
        sudo make install -j3 
    elif [ $1 -eq 4 ]; then
        sudo make install -j4
    elif [ $1 -eq 5 ]; then
        sudo make install -j5 
    elif [ $1 -eq 6 ]; then
        sudo make install -j6 
    elif [ $1 -eq 7 ]; then
        sudo make install -j7
    elif [ $1 -eq 8 ]; then
        sudo make install -j8
    else
        printf "Please enter a number of threads between 1 and 8\n"
    fi
fi

