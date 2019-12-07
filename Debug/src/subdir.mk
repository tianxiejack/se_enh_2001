################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CcCamCalibra.cpp \
../src/Displayer.cpp \
../src/MultiChVideo.cpp \
../src/VideoProcess.cpp \
../src/WorkThread.cpp \
../src/cuda_mem.cpp \
../src/main.cpp \
../src/process51.cpp \
../src/sceneProcess.cpp \
../src/v4l2camera.cpp 

CU_SRCS += \
../src/cuda.cu \
../src/cuda_convert.cu 

CU_DEPS += \
./src/cuda.d \
./src/cuda_convert.d 

OBJS += \
./src/CcCamCalibra.o \
./src/Displayer.o \
./src/MultiChVideo.o \
./src/VideoProcess.o \
./src/WorkThread.o \
./src/cuda.o \
./src/cuda_convert.o \
./src/cuda_mem.o \
./src/main.o \
./src/process51.o \
./src/sceneProcess.o \
./src/v4l2camera.o 

CPP_DEPS += \
./src/CcCamCalibra.d \
./src/Displayer.d \
./src/MultiChVideo.d \
./src/VideoProcess.d \
./src/WorkThread.d \
./src/cuda_mem.d \
./src/main.d \
./src/process51.d \
./src/sceneProcess.d \
./src/v4l2camera.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/osa -I../src/GST -G -g -O0 -Xcompiler -fopenmp -ccbin aarch64-linux-gnu-g++ -gencode arch=compute_53,code=sm_53 -m64 -odir "src" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/osa -I../src/GST -G -g -O0 -Xcompiler -fopenmp --compile -m64 -ccbin aarch64-linux-gnu-g++  -x c++ -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cu
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/osa -I../src/GST -G -g -O0 -Xcompiler -fopenmp -ccbin aarch64-linux-gnu-g++ -gencode arch=compute_53,code=sm_53 -m64 -odir "src" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/osa -I../src/GST -G -g -O0 -Xcompiler -fopenmp --compile --relocatable-device-code=false -gencode arch=compute_53,code=compute_53 -gencode arch=compute_53,code=sm_53 -m64 -ccbin aarch64-linux-gnu-g++  -x cu -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


