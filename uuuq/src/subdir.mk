################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/BlackCore.cpp \
../src/BlackUART.cpp \
../src/main.cpp 

OBJS += \
./src/BlackCore.o \
./src/BlackUART.o \
./src/main.o 

CPP_DEPS += \
./src/BlackCore.d \
./src/BlackUART.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	arm-linux-gnueabihf-g++ -I/home/quangdo/workspace/test/src -I/usr/arm-linux-gnueabihf/include/c++/4.8.4 -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


