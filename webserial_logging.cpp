#include "webserial_logging.h"
#include "configuration.h"
#include <FS.h>
#include <SPIFFS.h>

const size_t MAX_LOG_SIZE = 10 * 1024; // 10 KB

void logToFile(const String& message) {
    File logFile = SPIFFS.open(LOG_FILE_PATH, FILE_APPEND);
    if (!logFile) {
        Serial.println("Failed to open log file for writing.");
        return;
    }

    logFile.println(message);
    logFile.close();

    // Check if the file size exceeds the max log size
    if (SPIFFS.exists(LOG_FILE_PATH)) {
        File logFile = SPIFFS.open(LOG_FILE_PATH, FILE_READ);
        if (logFile.size() > MAX_LOG_SIZE) {
            // File is too large, let's trim the oldest entries
            String logContent = "";
            String line;

            // Read the log file content into memory
            while (logFile.available()) {
                line = logFile.readStringUntil('\n');
                logContent += line + "\n";
            }
            logFile.close();

            // Calculate the number of lines we need to keep
            int totalLines = logContent.length() / 256; // Approximate line length
            int linesToKeep = totalLines - (MAX_LOG_SIZE / 256); // Adjust based on the size you want to keep

            // Keep only the latest lines
            String trimmedLogContent = "";
            int currentLine = 0;
            int lineStartPos = 0;

            while (currentLine < linesToKeep && (lineStartPos = logContent.indexOf('\n', lineStartPos)) != -1) {
                trimmedLogContent += logContent.substring(0, lineStartPos + 1);
                lineStartPos++;
                currentLine++;
            }

            // Rewrite the file with the trimmed content
            File newLogFile = SPIFFS.open(LOG_FILE_PATH, FILE_WRITE);
            if (newLogFile) {
                newLogFile.print(trimmedLogContent); // Keep the last log entries
                newLogFile.close();
            }
        }
    }
}


// Base: String
void logPrint(const String& message) {
    Serial.print(message);
    logToFile(message);
}

void logPrintln(const String& message) {
    Serial.println(message);
    logToFile(message);
}

// const char*
void logPrint(const char* message) {
    Serial.print(message);
    logToFile(String(message));
}

void logPrintln(const char* message) {
    Serial.println(message);
    logToFile(String(message));
}

// char
void logPrint(char value) {
    Serial.print(value);
    logToFile(String(value));
}

void logPrintln(char value) {
    Serial.println(value);
    logToFile(String(value));
}

// int
void logPrint(int value) {
    Serial.print(value);
    logToFile(String(value));
}

void logPrintln(int value) {
    Serial.println(value);
    logToFile(String(value));
}

// unsigned int
void logPrint(unsigned int value) {
    Serial.print(value);
    logToFile(String(value));
}

void logPrintln(unsigned int value) {
    Serial.println(value);
    logToFile(String(value));
}

// long
void logPrint(long value) {
    Serial.print(value);
    logToFile(String(value));
}

void logPrintln(long value) {
    Serial.println(value);
    logToFile(String(value));
}

// unsigned long
void logPrint(unsigned long value) {
    Serial.print(value);
    logToFile(String(value));
}

void logPrintln(unsigned long value) {
    Serial.println(value);
    logToFile(String(value));
}

// float
void logPrint(float value) {
    Serial.print(value);
    logToFile(String(value));
}

void logPrintln(float value) {
    Serial.println(value);
    logToFile(String(value));
}

// double
void logPrint(double value) {
    Serial.print(value);
    logToFile(String(value));
}

void logPrintln(double value) {
    Serial.println(value);
    logToFile(String(value));
}

// bool
void logPrint(bool value) {
    Serial.print(value ? "true" : "false");
    logToFile(value ? "true" : "false");
}

void logPrintln(bool value) {
    Serial.println(value ? "true" : "false");
    logToFile(value ? "true" : "false");
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
    logToFile(formattedMessage);
}
