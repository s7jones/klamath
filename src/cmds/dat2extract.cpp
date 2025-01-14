#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>

#include "third_party/CLI11.hpp"

#include "src/formats/dat2_reader.hpp"
#include "src/utils/zlib.hpp"

#if _MSC_VER && !__INTEL_COMPILER
#include <direct.h>
#define MKDIR(a,b) _mkdir(a)
#else
#define MKDIR(a,b) mkdir(a,b)
#endif

namespace {
  using namespace klmth;

  bool file_exists(const std::string& filename) {
    struct stat st = {0};
    return stat(filename.c_str(), &st) != -1;
  }

  struct Config {
    Config(std::string _out_dir) : out_dir(std::move(_out_dir)) {
    }

    std::string out_dir;
  };

  class EntryExtractor {
  public:
    EntryExtractor(std::istream& _in, Config _cfg) : in(_in), cfg(std::move(_cfg)) {
    }

    void extract(const dat2::TreeEntry& entry) {
      static const char dat2_pth_sep = '\\';
      static const char os_pth_sep = '/';


      if (!file_exists(cfg.out_dir) && MKDIR(cfg.out_dir.c_str(), 0777) == -1) {
        throw std::runtime_error(cfg.out_dir + ": cannot create output directory");
      }

      std::string pth_el = cfg.out_dir + os_pth_sep;
      for (char c : entry.filename) {
        if (c == dat2_pth_sep) {
          if (!file_exists(pth_el) && MKDIR(pth_el.c_str(), 0777) == -1) {
            throw std::runtime_error(pth_el + ": cannot create output directory");
          }

          pth_el += os_pth_sep;
        } else {
          pth_el += c;
        }
      }

      std::fstream out;
      out.open(pth_el, std::ios::out | std::ios::binary);


      auto pos_before = in.tellg();
      in.seekg(entry.offset, std::ios::beg);

      if (entry.is_compressed) {
        zlib::decompress(in, entry.packed_size, out);
      } else {
        // TODO: raw entries
      }

      in.seekg(pos_before, std::ios::beg);
    }

  private:
    std::istream& in;
    Config cfg;

    std::string pth_el;
  };

  void extract_entries(std::istream& in, const Config& cfg) {
    dat2::Sections s = dat2::read_sections(in);

    in.seekg(s.tree_offset, std::ios::beg);

    EntryExtractor entry_extractor(in, cfg);
    for (size_t i = 0; i < s.num_files; ++i) {
      entry_extractor.extract(dat2::read_tree_entry(in));
    }
  }
}

int cmd_dat2extract(int argc, char** argv) {
  CLI::App app{"extract a DAT2 file into the current working directory"};
  std::string dat2_path;
  app.add_option("dat2_file", dat2_path, "path to a DAT2 file")->required();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& ex) {
    return app.exit(ex);
  }

  std::ifstream dat2_strm{dat2_path, std::ios::in | std::ios::binary};

  if (!dat2_strm.good()) {
    std::cerr << argv[0] << ": " << dat2_path << " error opening file" << std::endl;
    return 1;
  }

  try {
    Config cfg("out");
    extract_entries(dat2_strm, cfg);
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << argv[0] << ": " << dat2_path << ": " << ex.what() << std::endl;
    return 1;
  }
}
