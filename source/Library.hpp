// Driftfield — wavetable + sample library locations, scanning and portable path encoding.
//
// Factory/user library:  <Documents>/Sydalesis/Wavetables/<Category>/<Name>.wav  (5.1.2; was Documents/Driftfield)
// Serum tables (if found): <Documents>/Xfer/Serum Presets/Tables
// Extra folders: one absolute path per line in <Documents>/Sydalesis/folders.txt
//
// Tables inside the Driftfield library are stored in sessions as "lib:<Category>/<Name>.wav"
// so projects survive moving to another machine; anything else is stored as an absolute path.
#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cstdlib>

// Implemented in Platform.cpp (keeps <windows.h> out of headers shared with DGL)
std::string driftPlatformDocumentsDir();

namespace drift { namespace lib {

namespace fs = std::filesystem;

inline std::string u8(const fs::path& p) {
    const auto s = p.u8string();
    return std::string(s.begin(), s.end());
}
inline fs::path fromU8(const std::string& s) { return fs::u8path(s); }

inline std::string documentsDir() { return driftPlatformDocumentsDir(); }

// 5.1.2: everything the plug-in reads and writes lives in <Documents>/Sydalesis. The first time it runs, whatever is
// missing there is copied from <Documents>/Driftfield (presets, favourites, rejects, recent, options, MIDI learn,
// captures, wavetables, samples, tunings); nothing is moved, so Driftfield keeps working for old sessions.
// A marker file makes this happen once. Thread-safe: the first caller does it, later callers wait.
inline std::string dataDir() {
    static const std::string dir = [] {
        const fs::path docs = fromU8(documentsDir());
        const fs::path nu = docs / "Sydalesis", old = docs / "Driftfield", mark = nu / ".copied-from-driftfield";
        std::error_code ec;
        fs::create_directories(nu, ec);
        if (!fs::exists(mark, ec) && fs::is_directory(old, ec)) {
            fs::copy(old, nu, fs::copy_options::recursive | fs::copy_options::skip_existing, ec);
            std::ofstream(mark) << "Sydalesis copied the Driftfield folder here once (missing files only).\n";
        }
        return u8(nu);
    }();
    return dir;
}

inline std::string libraryDir() { return u8(fromU8(dataDir()) / "Wavetables"); }
inline std::string samplesDir() { return u8(fromU8(dataDir()) / "Samples"); }

struct Root { std::string name, path; };

inline std::vector<Root> roots() {
    std::vector<Root> r;
    r.push_back({ "Sydalesis", libraryDir() });
    std::error_code ec;
    const fs::path serum = fromU8(documentsDir()) / "Xfer" / "Serum Presets" / "Tables";
    if (fs::is_directory(serum, ec)) r.push_back({ "Serum", u8(serum) });
    std::ifstream in(fromU8(dataDir()) / "folders.txt");
    std::string line;
    while (std::getline(in, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        if (fs::is_directory(fromU8(line), ec)) r.push_back({ u8(fromU8(line).filename()), line });
    }
    return r;
}

struct Entry {
    std::string category;   // "<root> / <subfolder>"
    std::string name;       // file stem
    std::string path;       // absolute, UTF-8
};

inline std::vector<Entry> scanRoots(const std::vector<Root>& rs);
inline std::vector<Entry> scan() { return scanRoots(roots()); }
inline std::vector<Entry> scanSamples() { return scanRoots({ { "Samples", samplesDir() } }); }
inline std::vector<Entry> scanRoots(const std::vector<Root>& rs) {
    std::vector<Entry> out;
    for (const Root& root : rs) {
        std::error_code ec;
        const fs::path base = fromU8(root.path);
        if (!fs::is_directory(base, ec)) continue;
        std::vector<Entry> part;
        for (fs::recursive_directory_iterator it(base, fs::directory_options::skip_permission_denied, ec), end;
             it != end; it.increment(ec)) {
            if (ec) break;
            if (!it->is_regular_file(ec)) continue;
            std::string ext = u8(it->path().extension());
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            if (ext != ".wav") continue;
            const fs::path rel = it->path().parent_path().lexically_relative(base);
            std::string cat = root.name;
            if (!rel.empty() && rel != ".") cat += " / " + u8(rel);
            for (auto& ch : cat) if (ch == '\\') ch = '/';
            part.push_back({ cat, u8(it->path().stem()), u8(it->path()) });
            if (part.size() > 20000) break;
        }
        std::sort(part.begin(), part.end(), [](const Entry& a, const Entry& b) {
            return a.category != b.category ? a.category < b.category : a.name < b.name; });
        out.insert(out.end(), part.begin(), part.end());
    }
    return out;
}

// Absolute path -> session string ("lib:" = wavetable library, "smp:" = sample library)
inline std::string encodeIn(const std::string& absPath, const std::string& dir, const char* prefix) {
    std::error_code ec;
    const fs::path base = fs::weakly_canonical(fromU8(dir), ec);
    const fs::path p = fs::weakly_canonical(fromU8(absPath), ec);
    const fs::path rel = p.lexically_relative(base);
    const std::string r = u8(rel);
    if (!r.empty() && r.rfind("..", 0) != 0 && !rel.is_absolute()) {
        std::string s = r;
        for (auto& ch : s) if (ch == '\\') ch = '/';
        return prefix + s;
    }
    return "";
}
inline std::string encode(const std::string& absPath) {
    std::string e = encodeIn(absPath, libraryDir(), "lib:");
    if (e.empty()) e = encodeIn(absPath, samplesDir(), "smp:");
    return e.empty() ? absPath : e;
}

// Session string -> absolute path (may not exist)
inline std::string resolve(const std::string& stored) {
    if (stored.rfind("lib:", 0) == 0) return u8(fromU8(libraryDir()) / fromU8(stored.substr(4)));
    if (stored.rfind("smp:", 0) == 0) return u8(fromU8(samplesDir()) / fromU8(stored.substr(4)));
    std::error_code ec;
    if (fs::exists(fromU8(stored), ec)) return stored;
    // Absolute path from another machine: try the same file name inside the library.
    const std::string stem = u8(fromU8(stored).filename());
    for (const Entry& e : scan()) if (u8(fromU8(e.path).filename()) == stem) return e.path;
    return stored;
}

inline std::string displayName(const std::string& stored) {
    if (stored.empty()) return "";
    std::string s = stored;
    for (auto& ch : s) if (ch == '\\') ch = '/';
    const size_t sl = s.find_last_of('/');
    std::string n = sl == std::string::npos ? s : s.substr(sl + 1);
    if (n.rfind("lib:", 0) == 0 || n.rfind("smp:", 0) == 0) n = n.substr(4);
    const size_t d = n.find_last_of('.');
    return d == std::string::npos ? n : n.substr(0, d);
}

}} // namespace drift::lib
