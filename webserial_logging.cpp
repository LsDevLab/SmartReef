#include "webserial_logging.h"
#include "configuration.h"
#include <FS.h>
#include <SPIFFS.h>

const size_t MAX_LOG_SIZE = 10 * 1024; // 10 KB

bool loggingEnabled = true;
void logToFile(const String& message, bool nl) {
    if (!loggingEnabled) return;

    File logFile = SPIFFS.open(LOG_FILE_PATH, FILE_APPEND);
    if (!logFile) {
        Serial.println("Failed to open log file for writing.");
        return; 
    }

    logFile.print(message);
    if (nl) logFile.print("\n");
    logFile.close();

    // Check file size and clear if over limit
    logFile = SPIFFS.open(LOG_FILE_PATH, FILE_READ);
    if (!logFile) return;

    size_t fileSize = logFile.size();
    logFile.close();

    if (fileSize > MAX_LOG_SIZE) {
        logFile = SPIFFS.open(LOG_FILE_PATH, FILE_WRITE); // This clears the file
        if (logFile) logFile.close();
        else Serial.println("Failed to clear log file.");
    }
}



// Base: String
void logPrint(const String& message) {
    Serial.print(message);
    logToFile(message, false);
}

void logPrintln(const String& message) {
    Serial.println(message);
    logToFile(message, true);
}

// const char*
void logPrint(const char* message) {
    Serial.print(message);
    logToFile(String(message), false);
}

void logPrintln(const char* message) {
    Serial.println(message);
    logToFile(String(message), true);
}

// char
void logPrint(char value) {
    Serial.print(value);
    logToFile(String(value), false);
}

void logPrintln(char value) {
    Serial.println(value);
    logToFile(String(value), true);
}

// int
void logPrint(int value) {
    Serial.print(value);
    logToFile(String(value), false);
}

void logPrintln(int value) {
    Serial.println(value);
    logToFile(String(value), true);
}

// unsigned int
void logPrint(unsigned int value) {
    Serial.print(value);
    logToFile(String(value), false);
}

void logPrintln(unsigned int value) {
    Serial.println(value);
    logToFile(String(value), true);
}

// long
void logPrint(long value) {
    Serial.print(value);
    logToFile(String(value), false);
}

void logPrintln(long value) {
    Serial.println(value);
    logToFile(String(value), true);
}

// unsigned long
void logPrint(unsigned long value) {
    Serial.print(value);
    logToFile(String(value), false);
}

void logPrintln(unsigned long value) {
    Serial.println(value);
    logToFile(String(value), true);
}

// float
void logPrint(float value) {
    Serial.print(value);
    logToFile(String(value), false);
}

void logPrintln(float value) {
    Serial.println(value);
    logToFile(String(value), true);
}

// double
void logPrint(double value) {
    Serial.print(value);
    logToFile(String(value), false);
}

void logPrintln(double value) {
    Serial.println(value);
    logToFile(String(value), true);
}

// bool
void logPrint(bool value) {
    Serial.print(value ? "true" : "false");
    logToFile(value ? "true" : "false", false);
}

void logPrintln(bool value) {
    Serial.println(value ? "true" : "false");
    logToFile(value ? "true" : "false", true);
}

void logPrintf(const char* format, ...) {
    // Create a buffer for the formatted string
    char buffer[256]; // Adjust size based on the expected format

    // Process variable arguments
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Convert to String for consistency with other log methods
    String formattedMessage = String(buffer);

    // Print the formatted message to Serial and log file
    Serial.print(formattedMessage);
    logToFile(formattedMessage, false);
}
