################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/DXUTC/config_menu.cpp \
../src/DXUTC/dx.cpp \
../src/DXUTC/dxTimer.cpp \
../src/DXUTC/script.cpp \
../src/DXUTC/sys_config.cpp 

OBJS += \
./src/DXUTC/config_menu.o \
./src/DXUTC/dx.o \
./src/DXUTC/dxTimer.o \
./src/DXUTC/script.o \
./src/DXUTC/sys_config.o 

CPP_DEPS += \
./src/DXUTC/config_menu.d \
./src/DXUTC/dx.d \
./src/DXUTC/dxTimer.d \
./src/DXUTC/script.d \
./src/DXUTC/sys_config.d 


# Each subdirectory must supply rules for building sources it contributes
src/DXUTC/%.o: ../src/DXUTC/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__MOVE_DETECT__=1 -D__TRACK__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -G -g -O0 -Xcompiler -fopenmp -ccbin aarch64-linux-gnu-g++ -gencode arch=compute_20,code=sm_20 -m64 -odir "src/DXUTC" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-8.0/bin/nvcc -D__IPC__=1 -D__MOVE_DETECT__=1 -D__TRACK__=1 -I/usr/lib/aarch64-linux-gnu/include -I/usr/include/freetype2 -I../src/OSA_IPC/inc -I/usr/include/opencv -I/usr/include/opencv2 -I/usr/include/GL -I../include -I../include/APP -I../include/dxutc -I../src/OSA_CAP/inc -G -g -O0 -Xcompiler -fopenmp --compile -m64 -ccbin aarch64-linux-gnu-g++  -x c++ -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


