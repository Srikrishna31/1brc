//
// Created by coolk on 29-10-2024.
//

#include <windows.h>
#include <fileapi.h>
#include <stdexcept>
#include <string_view>

#include "FileMmapWin.h"

using namespace std::string_literals;

struct Data {
HANDLE hFile {};
HANDLE hMapFile {};
LPVOID lpBase {};
size_t fileSize{};
};

FileMmapWin::FileMmapWin(std::string f)
 : file(std::move(f))
 , data(std::make_unique<Data>())
{
    data->hFile = CreateFile(file.c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            nullptr,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            nullptr);

    if (data->hFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Could not open file. Error " + GetLastErrorAsString());
    }

    data->hMapFile = CreateFileMappingA(data->hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);

    if (data->hMapFile == nullptr) {
        CloseHandle(data->hFile);
        throw std::runtime_error("Could not create file mapping object. Error: " + GetLastErrorAsString());
    }

    data->lpBase = MapViewOfFile(data->hMapFile, FILE_MAP_READ, 0,0,0);

    if (data->lpBase == nullptr) {
        CloseHandle(data->hMapFile);
        CloseHandle(data->hFile);
        throw std::runtime_error("Couldnot map view of file. Error: " + GetLastErrorAsString());
    }

    data->fileSize = GetFileSize(data->hFile, nullptr);
    if (data->fileSize == INVALID_FILE_SIZE) {
        cleanup();
        throw std::runtime_error("Could not get file size. Error: " + GetLastErrorAsString());
    }
}

FileMmapWin::~FileMmapWin() {
    cleanup();
}

// Taken from here: https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
std::string FileMmapWin::GetLastErrorAsString() {
    auto errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return ""s;
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID;
    auto size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               nullptr, errorMessageID,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPSTR)&messageBuffer, 0, nullptr);

    std::string message(messageBuffer, size);

    LocalFree(messageBuffer);

    return message;
}

void FileMmapWin::cleanup() {
    UnmapViewOfFile(data->lpBase);
    CloseHandle(data->hMapFile);
    CloseHandle(data->hFile);
}

std::optional<std::string_view> FileMmapWin::getLine() {
    return std::string_view();
}

FileMmapWin::LineIterator FileMmapWin::begin() {
    return {*this};
}

FileMmapWin::LineIterator FileMmapWin::end() {
    return {*this, data->fileSize, static_cast<decltype(LineIterator::lineLength)>(data->fileSize)};
}

std::string_view FileMmapWin::LineIterator::operator*() {
    if (pos < f.data->fileSize) {
        auto data = static_cast<char*>(f.data->lpBase);
        return {data + pos, lineLength - 2}; // exclude the \r\n which is 2 characters.
    }

    return {};
}

FileMmapWin::LineIterator &FileMmapWin::LineIterator::operator++() {
    if (pos < f.data->fileSize - 1) {
        auto data = static_cast<char*>(f.data->lpBase);
        auto i = 0;
        for (;i < f.data->fileSize - 1 && data[pos + lineLength + i] != '\n'; ++i);
        pos += lineLength;
        lineLength = i + 1; // point to the next row.
    }
    return *this;
}

FileMmapWin::LineIterator &FileMmapWin::LineIterator::operator++(int) {
    return *this;
}

bool FileMmapWin::LineIterator::operator==(const FileMmapWin::LineIterator &other) const {
    return &f == &other.f && pos == other.pos && lineLength == other.lineLength;
}

bool FileMmapWin::LineIterator::operator!=(const FileMmapWin::LineIterator &other) const {
    return !(*this == other);
}

FileMmapWin::LineIterator::LineIterator(FileMmapWin &f, uint64_t pos, uint32_t lineLength)
: f{f}
, pos{pos}
, lineLength{lineLength}
{
    //Make sure the first line is already read, when the iterator is created.
    operator++();
}
