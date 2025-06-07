#pragma once

#include <stdio.h>

#include <string>
#include <vector>

class NSF {
public:
    NSF() = default;
    ~NSF() = default;

    [[nodiscard]] bool valid() const { return mValid; }

    bool load(const std::string& path) {
        FILE* fp = fopen(path.c_str(), "rb");
        if(!fp) return false;

        fseek(fp, 0, SEEK_END);
        size_t fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        std::vector<char> buffer(fileSize);
        fileSize = fread(&buffer[0], fileSize, 1, fp);
        fclose(fp);

        // check if header is NSF(2) or NSFe
        if(buffer.size() < 5) return false;
        //if(0 == memcmp(buffer.data(), ))

        return true;
    }

private:
    enum NSF_Type {
        INVALID = 0,
        NSF1,
        NSF2,
        NSFE
    };

    bool validate_nsf(const std::vector<char>& bytes) {
        return true;
    }

    bool validate_nsfe(const std::vector<char>& bytes) {
        return true;
    }

    bool mValid = false;
};