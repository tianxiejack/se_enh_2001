################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/LASERBLOB/BlobDetector.cpp 

OBJS += \
./src/LASERBLOB/BlobDetector.o 

CPP_DEPS += \
./src/LASERBLOB/BlobDetector.d 


# Each subdirectory must supply rules for building sources it contributes
src/LASERBLOB/%.o: ../src/LASERBLOB/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-9.1/bin/nvcc -D__IPC__=1 -D__MOVE_DETECT__=1 -D__TRACK__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -G -g -O0 -Xcompiler -fopenmp -gencode arch=compute_30,code=sm_30  -odir "src/LASERBLOB" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-9.1/bin/nvcc -D__IPC__=1 -D__MOVE_DETECT__=1 -D__TRACK__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -G -g -O0 -Xcompiler -fopenmp --compile  -x c++ -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


