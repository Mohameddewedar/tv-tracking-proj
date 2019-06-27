# ######################################
# # INSTALL OPENCV ON UBUNTU OR DEBIAN #
# ######################################

# # |          THIS SCRIPT IS TESTED CORRECTLY ON          |
# # |------------------------------------------------------|
# # | OS               | OpenCV       | Test | Last test   |
# # |------------------|--------------|------|-------------|
# # | Ubuntu 18.04 LTS | OpenCV 3.4.2 | OK   | 18 Jul 2018 |
# # | Debian 9.5       | OpenCV 3.4.2 | OK   | 18 Jul 2018 |
# # |----------------------------------------------------- |
# # | Debian 9.0       | OpenCV 3.2.0 | OK   | 25 Jun 2017 |
# # | Debian 8.8       | OpenCV 3.2.0 | OK   | 20 May 2017 |
# # | Ubuntu 16.04 LTS | OpenCV 3.2.0 | OK   | 20 May 2017 |


# # VERSION TO BE INSTALLED

OPENCV_VERSION='3.4.3'


# # 1. KEEP UBUNTU OR DEBIAN UP TO DATE

# sudo apt -y update
# sudo apt -y upgrade
# sudo apt -y dist-upgrade
# sudo apt -y autoremove


# # 2. INSTALL THE DEPENDENCIES

# # Build tools:
# sudo apt install -y build-essential cmake

# # GUI (if you want to use GTK instead of Qt, replace 'qt5-default' with 'libgtkglext1-dev' and remove '-DWITH_QT=ON' option in CMake):
# sudo apt install -y qt5-default libvtk6-dev

# # Media I/O:
# sudo apt install -y zlib1g-dev libjpeg-dev libwebp-dev libpng-dev libtiff5-dev libjasper-dev libopenexr-dev libgdal-dev

# # Video I/O:
# sudo apt install -y libdc1394-22-dev libavcodec-dev libavformat-dev libswscale-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev yasm libopencore-amrnb-dev libopencore-amrwb-dev libv4l-dev libxine2-dev

# # Parallelism and linear algebra libraries:
# sudo apt install -y libtbb-dev libeigen3-dev

# # Python:
# sudo apt install -y python-dev python-tk python-numpy python3-dev python3-tk python3-numpy

# # Java:
# sudo apt install -y ant default-jdk

# # Documentation:
# sudo apt install -y doxygen


# 3. INSTALL THE LIBRARY

# sudo apt install -y unzip wget
# wget https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip
unzip ${OPENCV_VERSION}.zip
# rm ${OPENCV_VERSION}.zip
mv opencv-${OPENCV_VERSION} OpenCV
cd OpenCV
mkdir build
cd build
cmake -DWITH_QT=ON -DWITH_OPENGL=ON -DFORCE_VTK=ON -DWITH_TBB=ON -DWITH_GDAL=ON -DWITH_XINE=ON -DBUILD_EXAMPLES=ON -DENABLE_PRECOMPILED_HEADERS=OFF ..
make -j4
sudo make install
sudo ldconfig


# 4. EXECUTE SOME OPENCV EXAMPLES AND COMPILE A DEMONSTRATION

# To complete this step, please visit 'http://milq.github.io/install-opencv-ubuntu-debian'.