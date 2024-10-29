//
// Created by coolk on 29-10-2024.
//

#pragma once

#include <string>
#include <memory>
#include <string_view>
#include <optional>

class FileMmapWin {
    explicit FileMmapWin(std::string f);
    ~FileMmapWin();

    std::optional<std::string_view> getLine();

    class LineIterator {
        FileMmapWin& f;
        uint64_t pos;
    public:
        LineIterator(FileMmapWin& f, uint64_t  pos = 0);
        std::string_view operator*();
        LineIterator& operator++();
        LineIterator& operator++(int);
        bool operator==(const LineIterator& other) const;
        bool operator!=(const LineIterator& other) const;

    };

    LineIterator begin();
    LineIterator end();
private:

    static std::string GetLastErrorAsString();

    void cleanup();
    std::string file;
    std::unique_ptr<struct Data> data;
};
