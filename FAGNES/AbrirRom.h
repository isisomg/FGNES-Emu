#pragma once
#include <Windows.h> // mudar para ser compativel com linux tambem
#include <commdlg.h>
#include <string>
#include <iostream>

std::string AbrirArquivo() {
    char caminhoArquivo[MAX_PATH] = "";

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr; // Janela principal (pode ser null)
    ofn.lpstrFile = caminhoArquivo;
    ofn.nMaxFile = sizeof(caminhoArquivo);
    ofn.lpstrFilter = "ROM\0*.nes\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        return std::string(caminhoArquivo);
    }

    return "";
}