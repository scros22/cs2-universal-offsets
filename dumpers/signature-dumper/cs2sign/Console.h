#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <iomanip>

class Console {
public:
    enum Color {
        BLACK = 0,
        DARK_BLUE = 1,
        DARK_GREEN = 2,
        DARK_CYAN = 3,
        DARK_RED = 4,
        DARK_MAGENTA = 5,
        DARK_YELLOW = 6,
        LIGHT_GRAY = 7,
        DARK_GRAY = 8,
        BLUE = 9,
        GREEN = 10,
        CYAN = 11,
        RED = 12,
        MAGENTA = 13,
        YELLOW = 14,
        WHITE = 15
    };

    static void Init() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        originalAttributes = csbi.wAttributes;
    }

    static void SetColor(Color foreground, Color background = BLACK) {
        if (hConsole == INVALID_HANDLE_VALUE) Init();
        WORD color = (background << 4) | foreground;
        SetConsoleTextAttribute(hConsole, color);
    }

    static void ResetColor() {
        if (hConsole == INVALID_HANDLE_VALUE) Init();
        SetConsoleTextAttribute(hConsole, originalAttributes);
    }

    static void PrintBanner() {
        SetColor(CYAN);
        std::cout << "\n";
        std::cout << "================================================================\n";
        std::cout << "                                                                \n";
        std::cout << "     ____  ____  __  __  _______  _______  ______               \n";
        std::cout << "    |  _ \\|  _ \\|  \\/  ||__   __||__   __||  ____|              \n";
        std::cout << "    | |_) | |_) | \\  / |   | |      | |   | |__                 \n";
        std::cout << "    |  _ <|  _ <| |\\/| |   | |      | |   |  __|                \n";
        std::cout << "    | |_) | |_) | |  | |   | |      | |   | |____               \n";
        std::cout << "    |____/|____/|_|  |_|   |_|      |_|   |______|              \n";
        std::cout << "                                                                \n";
        std::cout << "          CS2 Signature Dumper v2.0 - Professional Edition      \n";
        std::cout << "                    Developed by bcwtf                           \n";
        std::cout << "                                                                \n";
        std::cout << "================================================================\n";
        ResetColor();
        std::cout << "\n";
    }

    static std::string WStringToString(const std::wstring& wstr) {
        if (wstr.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    static void PrintHeader(const std::wstring& text) {
        std::string textStr = WStringToString(text);
        SetColor(CYAN);
        std::cout << "\n[ " << textStr;
        int padding = 60 - (int)textStr.length();
        for (int i = 0; i < padding; i++) std::cout << " ";
        std::cout << " ]\n";
        ResetColor();
    }

    static void PrintFooter() {
        SetColor(CYAN);
        std::cout << std::string(65, '=') << "\n";
        ResetColor();
    }

    static void PrintSuccess(const std::wstring& text) {
        std::string textStr = WStringToString(text);
        SetColor(GREEN);
        std::cout << "  [+] " << textStr << "\n";
        ResetColor();
    }

    static void PrintError(const std::wstring& text) {
        std::string textStr = WStringToString(text);
        SetColor(RED);
        std::cout << "  [-] " << textStr << "\n";
        ResetColor();
    }

    static void PrintInfo(const std::wstring& text) {
        std::string textStr = WStringToString(text);
        SetColor(CYAN);
        std::cout << "  [*] " << textStr << "\n";
        ResetColor();
    }

    static void PrintWarning(const std::wstring& text) {
        std::string textStr = WStringToString(text);
        SetColor(YELLOW);
        std::cout << "  [!] " << textStr << "\n";
        ResetColor();
    }

    static void PrintProgress(size_t current, size_t total, const std::wstring& name) {
        double percent = (double)current / total * 100.0;
        int barWidth = 40;
        int filled = (int)(barWidth * percent / 100.0);
        std::string nameStr = WStringToString(name);
        if (nameStr.length() > 25) nameStr = nameStr.substr(0, 22) + "...";
        
        SetColor(CYAN);
        std::cout << "\r  [";
        ResetColor();
        
        SetColor(GREEN);
        for (int i = 0; i < filled; i++) std::cout << "=";
        ResetColor();
        
        SetColor(DARK_GRAY);
        for (int i = filled; i < barWidth; i++) std::cout << "-";
        ResetColor();
        
        SetColor(CYAN);
        std::cout << "] " << std::fixed << std::setprecision(1) << std::setw(5) << percent << "% ";
        std::cout << "(" << std::setw(3) << current << "/" << total << ") ";
        ResetColor();
        
        SetColor(WHITE);
        std::cout << std::setw(25) << std::left << nameStr;
        ResetColor();
        
        std::cout.flush();
    }

    static void PrintFound(const std::wstring& name, uintptr_t address, size_t regions, size_t bytes) {
        std::string nameStr = WStringToString(name);
        SetColor(GREEN);
        std::cout << "\r  [+] FOUND: ";
        ResetColor();
        SetColor(WHITE);
        std::cout << nameStr;
        ResetColor();
        SetColor(CYAN);
        std::cout << " -> ";
        ResetColor();
        SetColor(YELLOW);
        std::cout << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << address;
        ResetColor();
        SetColor(DARK_GRAY);
        std::cout << " [" << std::dec << regions << " regions, " << bytes << " bytes]";
        ResetColor();
        std::cout << "\n";
    }

    static void PrintNotFound(const std::wstring& name, const std::string& error) {
        std::string nameStr = WStringToString(name);
        SetColor(RED);
        std::cout << "\r  [-] NOT FOUND: ";
        ResetColor();
        SetColor(WHITE);
        std::cout << nameStr;
        ResetColor();
        SetColor(DARK_GRAY);
        std::cout << " - " << error;
        ResetColor();
        std::cout << "\n";
    }

    static void PrintErrorMsg(const std::wstring& name, const std::string& error) {
        std::string nameStr = WStringToString(name);
        SetColor(RED);
        std::cout << "\r  [-] ERROR: ";
        ResetColor();
        SetColor(WHITE);
        std::cout << nameStr;
        ResetColor();
        SetColor(DARK_GRAY);
        std::cout << " - " << error;
        ResetColor();
        std::cout << "\n";
    }

    static void ClearLine() {
        std::cout << "\r" << std::string(80, ' ') << "\r";
    }

private:
    static HANDLE hConsole;
    static WORD originalAttributes;
};

