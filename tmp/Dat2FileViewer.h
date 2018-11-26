#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../klamath-utils/src/lib/optional.h"
#include "../klamath-utils/src/lib/Mmap.h"

namespace klamath {

    typedef struct Dat2EntryMetadata {
        uint32_t offset;
        uint32_t packed_size;
        uint32_t decompressed_size;
    } Dat2EntryMetadata;

    class Dat2FileViewer {

    public:
        static Dat2FileViewer from_file(const std::string& path);

        Dat2FileViewer(Dat2FileViewer&&) noexcept;

        Dat2FileViewer(const Dat2FileViewer&) = delete;  // We don't want accidental copies

        tl::optional<std::vector<uint8_t>> read_entry(const std::string& path) const;

        std::vector<std::string> entry_names() const;

    private:
        Dat2FileViewer(Mmap mmap, std::unordered_map<std::string, Dat2EntryMetadata> entries);

        Mmap _mmap;
        std::unordered_map<std::string, Dat2EntryMetadata> _entries;
    };
}