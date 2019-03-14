################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CcCamCalibra.cpp \
../src/Displayer.cpp \
../src/Ipc.cpp \
../src/MultiChVideo.cpp \
../src/VideoProcess.cpp \
../src/WorkThread.cpp \
../src/app_ctrl.cpp \
../src/cuda_mem.cpp \
../src/process51.cpp \
../src/sceneProcess.cpp \
../src/test_videoProcess.cpp \
../src/v4l2camera.cpp 

CU_SRCS += \
../src/cuda.cu 

CU_DEPS += \
./src/cuda.d 

OBJS += \
./src/CcCamCalibra.o \
./src/Displayer.o \
./src/Ipc.o \
./src/MultiChVideo.o \
./src/VideoProcess.o \
./src/WorkThread.o \
./src/app_ctrl.o \
./src/cuda.o \
./src/cuda_mem.o \
./src/process51.o \
./src/sceneProcess.o \
./src/test_videoProcess.o \
./src/v4l2camera.o 

CPP_DEPS += \
./src/CcCamCalibra.d \
./src/Displayer.d \
./src/Ipc.d \
./src/MultiChVideo.d \
./src/VideoProcess.d \
./src/WorkThread.d \
./src/app_ctrl.d \
./src/cuda_mem.d \
./src/process51.d \
./src/sceneProcess.d \
./src/test_videoProcess.d \
./src/v4l2camera.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I../src/GST/compress -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/aarch64-linux-gnu/gstreamer-1.0/include -I/usr/lib/aarch64-linux-gnu/glib-2.0/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -O3 -ccbin aarch64-linux-gnu-g++ -gencode arch=compute_20,code=sm_20 -m64 -odir "src" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I../src/GST/compress -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/aarch64-linux-gnu/gstreamer-1.0/include -I/usr/lib/aarch64-linux-gnu/glib-2.0/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -O3 --compile -m64 -ccbin aarch64-linux-gnu-g++  -x c++ -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cu
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I../src/GST/compress -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/aarch64-linux-gnu/gstreamer-1.0/include -I/usr/lib/aarch64-linux-gnu/glib-2.0/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -O3 -ccbin aarch64-linux-gnu-g++ -gencode arch=compute_20,code=sm_20 -m64 -odir "src" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I../src/GST/compress -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/aarch64-linux-gnu/gstreamer-1.0/include -I/usr/lib/aarch64-linux-gnu/glib-2.0/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -O3 --compile --relocatable-device-code=false -gencode arch=compute_20,code=compute_20 -gencode arch=compute_20,code=sm_20 -m64 -ccbin aarch64-linux-gnu-g++  -x cu -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


