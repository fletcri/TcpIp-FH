################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ClientUtils.c \
../src/SimpleMessageClient.c \
../src/simple_message_client_commandline_handling.c 

OBJS += \
./src/ClientUtils.o \
./src/SimpleMessageClient.o \
./src/simple_message_client_commandline_handling.o 

C_DEPS += \
./src/ClientUtils.d \
./src/SimpleMessageClient.d \
./src/simple_message_client_commandline_handling.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


