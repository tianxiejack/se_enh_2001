################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/OSD_CR/osd_cv.cpp 

OBJS += \
./src/OSD_CR/osd_cv.o 

CPP_DEPS += \
./src/OSD_CR/osd_cv.d 


# Each subdirectory must supply rules for building sources it contributes
src/OSD_CR/%.o: ../src/OSD_CR/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I../src/GST/compress -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/aarch64-linux-gnu/gstreamer-1.0/include -I/usr/lib/aarch64-linux-gnu/glib-2.0/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -O3 -ccbin aarch64-linux-gnu-g++ -gencode arch=compute_20,code=sm_20 -m64 -odir "src/OSD_CR" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__TRACK__=1 -D__MOVE_DETECT__=1 -I/usr/lib/aarch64-linux-gnu/include -I../src/GST/compress -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/aarch64-linux-gnu/gstreamer-1.0/include -I/usr/lib/aarch64-linux-gnu/glib-2.0/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -O3 --compile -m64 -ccbin aarch64-linux-gnu-g++  -x c++ -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


