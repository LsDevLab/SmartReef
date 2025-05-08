#pragma once
#include <Arduino.h>

void logPrint(const String& message);
void logPrintln(const String& message);
void logPrint(const char* message);
void logPrintln(const char* message);
void logPrint(char value);
void logPrintln(char value);
void logPrint(int value);
void logPrintln(int value);
void logPrint(unsigned int value);
void logPrintln(unsigned int value);
void logPrint(long value);
void logPrintln(long value);
void logPrint(unsigned long value);
void logPrintln(unsigned long value);
void logPrint(float value);
void logPrintln(float value);
void logPrint(double value);
void logPrintln(double value);
void logPrint(bool value);
void logPrintln(bool value);
void logPrintf(const char* format, ...);                  
