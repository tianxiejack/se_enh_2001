################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/GST/GstEncTrans.cpp 

OBJS += \
./src/GST/GstEncTrans.o 

CPP_DEPS += \
./src/GST/GstEncTrans.d 


# Each subdirectory must supply rules for building sources it contributes
src/GST/%.o: ../src/GST/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/osa -I../src/GST -G -g -O0 -Xcompiler -fopenmp -ccbin aarch64-linux-gnu-g++ -gencode arch=compute_53,code=sm_53 -m64 -odir "src/GST" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/osa -I../src/GST -G -g -O0 -Xcompiler -fopenmp --compile -m64 -ccbin aarch64-linux-gnu-g++  -x c++ -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


