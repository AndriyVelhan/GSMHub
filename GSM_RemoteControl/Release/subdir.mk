################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sys_timer.c \
../keyboard.c \
../lcd_display.c \
../main.c \
../ow_termo.c \
../sim900.c 

OBJS += \
./Sys_timer.o \
./keyboard.o \
./lcd_display.o \
./main.o \
./ow_termo.o \
./sim900.o 

C_DEPS += \
./Sys_timer.d \
./keyboard.d \
./lcd_display.d \
./main.d \
./ow_termo.d \
./sim900.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega328 -DF_CPU=7372800UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


