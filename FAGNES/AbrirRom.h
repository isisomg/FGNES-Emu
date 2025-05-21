#pragma once
#include <Windows.h> // mudar para ser compativel com linux tambem
#include <commdlg.h>
#include <string>
#include <iostream>

std::string ConverterWideParaUtf8(const std::wstring& wstr) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, 0); // -1 para ignorar o null terminator
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
}


std::string AbrirArquivo() {
    wchar_t caminhoArquivo[MAX_PATH] = L"";

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = caminhoArquivo;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"ROM\0*.nes\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        return ConverterWideParaUtf8(std::wstring(caminhoArquivo));
    }

    return "";
}

