// ===================================================================================== Presets, browsers, undo, A/B, randomize
// (included inside class DriftfieldUI)

    // ------------------------------------------------------------------ snapshots (undo / A-B)
    struct Snap { std::vector<float> v; std::string st[kStateCount]; bool valid = false; };
    std::vector<Snap> undoStack, redoStack;
    uint32_t lastUndoTime = 0; int lastUndoParam = -2;
    Snap abSlot[2];
    int abCur = 0;

    Snap snapshot() const { Snap s; s.v.assign(hostVals, hostVals + kParamCount); for (int k = 0; k < kStateCount; ++k) s.st[k] = hostStates[k]; s.valid = true; return s; }
    void pushUndo(int param) {
        markEdited();
        undoStack.push_back(snapshot());
        if (undoStack.size() > 120) undoStack.erase(undoStack.begin());
        redoStack.clear();
        lastUndoTime = nowMs(); lastUndoParam = param;
    }
    void pushUndoCoalesced(uint32_t p) {
        if ((int)p == lastUndoParam && nowMs() - lastUndoTime < 700) { lastUndoTime = nowMs(); return; }
        pushUndo((int)p);
    }
    void applySnap(const Snap& s) {
        for (uint32_t i = 0; i < kParamCount; ++i) if (s.v[i] != hostVals[i]) hostSetParam(i, s.v[i]);
        for (int k = 0; k < kStateCount; ++k) if (s.st[k] != hostStates[k]) hostSetState(k, s.st[k]);
        repaint();
    }
    void undo() { if (undoStack.empty()) return; redoStack.push_back(snapshot()); Snap s = undoStack.back(); undoStack.pop_back(); applySnap(s); lastUndoParam = -2; }
    void redo() { if (redoStack.empty()) return; undoStack.push_back(snapshot()); Snap s = redoStack.back(); redoStack.pop_back(); applySnap(s); lastUndoParam = -2; }
    void switchAB(int to) {
        if (to == abCur) {   // clicking the active slot copies it into the other one
            abSlot[1 - to] = snapshot();
            return;
        }
        abSlot[abCur] = snapshot();
        if (!abSlot[to].valid) abSlot[to] = abSlot[abCur];
        pushUndo(-1);
        applySnap(abSlot[to]);
        abCur = to;
    }

    // ------------------------------------------------------------------ randomize
    bool randOpen = false;
    float randAmt = 0.35f;
    bool randScope[5] = { true, true, true, true, false };   // osc, filter, env, mod, fx
    Rect randRect, randAmtBar, randScopeBtn[5], randGo, randInitBtn[kOscTypes];
    void drawRandomize() {
        randRect = { randBtn.x - 200, 60, 420, 236 };
        const Rect& b = randRect;
        beginPath(); roundedRect(b.x + 3, b.y + 4, b.w, b.h, 8); fillColor(Color(0, 0, 0, 0.4f)); fill();
        beginPath(); roundedRect(b.x, b.y, b.w, b.h, 8); fillColor(shade(26.f)); fill(); strokeColor(cPanelEdge); stroke();
        label(b.x + 16, b.y + 20, "RANDOMIZE / MUTATE", 10.5f, cWarm, ALIGN_LEFT | ALIGN_MIDDLE);
        label(b.x + 16, b.y + 46, "amount", 9, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        randAmtBar = { b.x + 70, b.y + 38, 270, 16 };
        beginPath(); roundedRect(randAmtBar.x, randAmtBar.y, randAmtBar.w, randAmtBar.h, 4); fillColor(cInset); fill();
        beginPath(); roundedRect(randAmtBar.x, randAmtBar.y, randAmtBar.w * randAmt, randAmtBar.h, 4); fillColor(withA(cWarm, 0.7f)); fill();
        char buf[32]; std::snprintf(buf, sizeof(buf), randAmt < 0.25f ? "%.0f%% subtle" : (randAmt < 0.6f ? "%.0f%% mutate" : "%.0f%% wild"), randAmt * 100.f);
        label(b.x + b.w - 16, b.y + 46, buf, 9, cText, ALIGN_RIGHT | ALIGN_MIDDLE);
        static const char* const sc[5] = { "Oscillators", "Filters", "Envelopes", "Modulation", "FX" };
        for (int i = 0; i < 5; ++i) { randScopeBtn[i] = { b.x + 16 + i * 79.f, b.y + 68, 75, 24 }; button(randScopeBtn[i], sc[i], randScope[i], randScopeBtn[i].contains(hoverX, hoverY), 9, &cWarm); }
        randGo = { b.x + 16, b.y + 104, b.w - 32, 32 };
        button(randGo, "MUTATE CURRENT SOUND", false, randGo.contains(hoverX, hoverY), 11, &cWarm);
        label(b.x + 16, b.y + 156, "INIT PATCH", 9, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        static const char* const en[kOscTypes] = { "Wavetable", "Analog", "Additive", "Physical", "Granular", "Spectral" };
        for (int t = 0; t < kOscTypes; ++t) { randInitBtn[t] = { b.x + 16 + t * 66.f, b.y + 170, 62, 26 }; button(randInitBtn[t], en[t], false, randInitBtn[t].contains(hoverX, hoverY), 9); }
        label(b.x + 16, b.y + 214, "Mutations are undoable (↶). Master, quality and delay stay as they are.", 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }
    void randomizeClick(float x, float y) {
        if (!randRect.contains(x, y)) { randOpen = false; return; }
        if (randAmtBar.contains(x, y) || (y >= randAmtBar.y - 4 && y <= randAmtBar.y + randAmtBar.h + 4 && x >= randAmtBar.x - 6 && x <= randAmtBar.x + randAmtBar.w + 6)) {
            randAmt = std::max(0.02f, std::min(1.f, (x - randAmtBar.x) / randAmtBar.w)); return;
        }
        for (int i = 0; i < 5; ++i) if (randScopeBtn[i].contains(x, y)) { randScope[i] = !randScope[i]; return; }
        if (randGo.contains(x, y)) { mutate(); return; }
        for (int t = 0; t < kOscTypes; ++t) if (randInitBtn[t].contains(x, y)) { initPatch(t); randOpen = false; return; }
    }
    int paramGroup(uint32_t p) const {
        if (p <= pOscEnd) return 0;
        if (p >= pSubLevel && p <= pRingSource) return 0;
        if (p >= pF1Type && p <= pFBalance) return 1;
        if (p >= pFEnvA && p <= pMEnvR) return 2;
        if (p >= pL1Rate && p <= pMLast) return 3;
        if (p >= pDrvOn && p < kLayerParams) return 4;
        return -1;
    }
    void mutate() {
        pushUndo(-1);
        uint32_t seed = nowMs() * 2654435761u + 12345u;
        auto rnd = [&]() { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return (float)(seed >> 8) * (1.f / 16777216.f); };
        for (uint32_t p = 0; p < kLayerParams; ++p) {      // the edited layer only
            const int g = paramGroup(p);
            if (g < 0 || !randScope[g]) continue;
            if (p == pMaster || p == pQuality || p == pPolyphony || p == pDlOn || p == pDlMix || p == pPitch) continue;   // 5.1.6: never transpose
            if (p <= pOscEnd && (p % kOscFields == ofOn || p % kOscFields == ofLevel)) continue;
            if (p >= pM1Src && p <= pMLast && (p - pM1Src) % kSlotFields == sfSrc && values[p] < 0.5f && rnd() > randAmt * 0.5f) continue;
            const ParamDef& d = P(p);
            float v;
            if (d.flags & (fInt | fBool)) {
                if (rnd() > randAmt * 0.6f) continue;
                v = std::round(d.min + rnd() * (d.max - d.min));
            } else {
                const float n = toNorm(p, values[p]);
                v = fromNorm(p, n + (rnd() * 2.f - 1.f) * randAmt * 0.6f);
            }
            if (p == pF1Cutoff || p == pF2Cutoff) v = std::max(v, 180.f);
            editParameter(p, true); setParam(p, v); editParameter(p, false);
        }
        bool any = false;
        for (int o = 0; o < 3; ++o) any = any || values[oscP(o, ofOn)] >= 0.5f;
        if (!any) setParamUser(oscP(0, ofOn), 1.f);
        currentPreset = -1;
        publishPresetId(false);
    }
    void initPatch(int type) {
        pushUndo(-1);
        for (uint32_t p = 0; p < kLayerParams; ++p) if (values[p] != P(p).def) { editParameter(p, true); setParam(p, P(p).def); editParameter(p, false); }   // the edited layer only
        setParamUser(oscP(0, ofType), (float)type);
        setParamUser(oscP(1, ofOn), 0.f);
        for (int k = 0; k < kLayerStates; ++k) if (!stateValue[k].empty()) pushState(k, "", false);
        if (type == otSpectral) {   // something to hear: a library drone recording
            ensureLibrary();
            for (const auto& e : samples) if (e.name == "Shruti Box") { selectFileKind(0, e.path, bkSpec); break; }
        }
        currentPreset = -1;
        publishPresetId(false);
    }

    // ------------------------------------------------------------------ presets
    int currentPreset = 0;
    struct UserPreset { std::string name, path, tags; };
    std::vector<UserPreset> userPresets;
    bool presetOpen = false;
    int pbCat = 0;
    float pbScroll = 0.f;
    Rect pbRect, pbCatRect, pbListRect, pbCloseBtn, pbSaveBtn, pbSearch, pbIntoBtn;
    int pbInto = 0;   // 0 = whole patch, 2 = layer 2 only
    std::vector<std::string> pbCats;    // "★ Favorites", "All", factory categories..., "User"
    // 5.1: the bank has ~6000 presets: filtered lists, counts and search text are cached, not rebuilt per frame
    std::vector<int> pbEnts, pbCatCount;
    std::string pbEntsKey;
    std::vector<std::string> pbHay;     // lower-case "name category tags" per entry
    std::vector<int> pbCatOf;           // entry -> index in pbCats
    int favVersion = 0;
    float pbCatScroll = 0.f, pbBarGrab = 0.f;
    Rect pbBarRect, pbThumbRect;
    std::string searchText;
    std::set<std::string> favorites;
    bool saveOpen = false;
    std::string saveName, saveMsg, saveTags;
    Rect saveRect, saveOkBtn, saveCancelBtn;

    static std::string userPresetDir() { return driftDir() + "/Presets3"; }
    static std::string favFile() { return driftDir() + "/favorites-v3.txt"; }
    std::string presetKey(int idx) const {
        if (idx < 0) return "";
        if (idx < (int)kFactoryPresetCount) return std::string("F:") + kFactoryPresets[idx].category + "/" + kFactoryPresets[idx].name;
        const int u = idx - (int)kFactoryPresetCount;
        return u < (int)userPresets.size() ? "U:" + userPresets[u].name : "";
    }
    bool isFavorite(const std::string& k) const { return !k.empty() && favorites.count(k) > 0; }
    // 5.1.1: rejected presets (hidden from browsing, listed under "✗ Rejected"), recent presets, tag chips
    std::set<std::string> rejects;
    std::vector<std::pair<std::string, long long>> recent;    // key, unix time; newest first, max 30
    std::vector<std::string> pbChips;                           // active tag filters (AND)
    Rect pbChipR[16], pbExportBtn; int pbChipN = 0;
    static std::string rejFile() { return driftDir() + "/rejected-v3.txt"; }
    static std::string recentFile() { return driftDir() + "/recent-v3.txt"; }
    bool isRejected(const std::string& k) const { return !k.empty() && rejects.count(k) > 0; }
    void loadRejects() {
        rejects.clear();
        std::ifstream in(drift::lib::fromU8(rejFile())); std::string line;
        while (std::getline(in, line)) { if (!line.empty() && line.back() == '\r') line.pop_back(); if (!line.empty()) rejects.insert(line); }
    }
    void saveRejects() {
        std::error_code ec; std::filesystem::create_directories(drift::lib::fromU8(driftDir()), ec);
        std::ofstream out(drift::lib::fromU8(rejFile())); for (const auto& f : rejects) out << f << "\n";
    }
    void loadRecent() {
        recent.clear();
        std::ifstream in(drift::lib::fromU8(recentFile())); std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            const size_t t = line.find('\t'); if (t == std::string::npos) continue;
            recent.push_back({ line.substr(t + 1), std::atoll(line.substr(0, t).c_str()) });
        }
    }
    void noteRecent(int idx) {
        const std::string k = presetKey(idx); if (k.empty()) return;
        if (recent.empty()) loadRecent();
        for (size_t i = 0; i < recent.size(); ++i) if (recent[i].first == k) { recent.erase(recent.begin() + (long)i); break; }
        recent.insert(recent.begin(), { k, (long long)std::time(nullptr) });
        if (recent.size() > 30) recent.resize(30);
        std::error_code ec; std::filesystem::create_directories(drift::lib::fromU8(driftDir()), ec);
        std::ofstream out(drift::lib::fromU8(recentFile())); for (const auto& r : recent) out << r.second << "\t" << r.first << "\n";
        ++favVersion; pbCatCount.clear();
    }
    static std::string agoText(long long t) {
        const long long d = (long long)std::time(nullptr) - t;
        char b[32];
        if (d < 60) return "just now";
        if (d < 3600) { std::snprintf(b, sizeof b, "%lld min ago", d / 60); return b; }
        if (d < 86400) { std::snprintf(b, sizeof b, "%lld h ago", d / 3600); return b; }
        std::snprintf(b, sizeof b, "%lld days ago", d / 86400); return b;
    }
    // tags meant for search only (engines, internal markers) are not shown under the name
    static std::string visibleTags(const std::string& tags) {
        static const char* const hidden[] = { "mpe", "mseg", "scenes", "motion", "wavetable", "analog", "additive", "physical", "granular", "spectral",
            "layered51", "resonator", "spectral-fx", "macro", "lfo", "seq", "arp", "slow", "weighty" };
        std::string out; size_t p = 0;
        while (p <= tags.size()) {
            size_t e = tags.find(',', p); if (e == std::string::npos) e = tags.size();
            const std::string t = tags.substr(p, e - p);
            bool hide = t.empty();
            for (const char* h : hidden) if (t == h) hide = true;
            if (!hide) { if (!out.empty()) out += " · "; out += t; }
            p = e + 1;
        }
        return out;
    }
    int presetCpu(int idx) const { return idx >= 0 && idx < (int)kFactoryPresetCount ? (int)kPresetCpu[idx] : 0; }
    void exportRatings() {
        std::error_code ec; std::filesystem::create_directories(drift::lib::fromU8(driftDir()), ec);
        char nm[64]; const std::time_t now = std::time(nullptr); std::strftime(nm, sizeof nm, "/ratings-%Y-%m-%d-%H%M.txt", std::localtime(&now));
        const std::string path = driftDir() + nm;
        std::ofstream out(drift::lib::fromU8(path));
        out << "# Driftfield preset ratings: + favourite, - rejected\n";
        for (const auto& f : favorites) out << "+ " << f << "\n";
        for (const auto& f : rejects) out << "- " << f << "\n";
        flash(("Ratings written to " + path).c_str());
    }
    void loadFavorites() {
        favorites.clear();
        std::ifstream in(drift::lib::fromU8(favFile()));
        std::string line;
        while (std::getline(in, line)) { if (!line.empty() && line.back() == '\r') line.pop_back(); if (!line.empty()) favorites.insert(line); }
        ++favVersion; pbCatCount.clear();
        loadRejects(); loadRecent();
    }
    void saveFavorites() {
        std::error_code ec;
        std::filesystem::create_directories(drift::lib::fromU8(driftDir()), ec);
        std::ofstream out(drift::lib::fromU8(favFile()));
        for (const auto& f : favorites) out << f << "\n";
    }
    void scanUserPresets() {
        userPresets.clear();
        std::error_code ec;
        const auto dir = drift::lib::fromU8(userPresetDir());
        if (std::filesystem::is_directory(dir, ec))
            for (const auto& e : std::filesystem::directory_iterator(dir, ec))
                if (e.is_regular_file(ec) && e.path().extension() == ".df3preset") {
                    UserPreset up{ drift::lib::u8(e.path().stem()), drift::lib::u8(e.path()), "" };
                    std::ifstream in(e.path());
                    std::string line;
                    for (int i = 0; i < 5 && std::getline(in, line); ++i) if (line.rfind("tags ", 0) == 0) up.tags = line.substr(5);
                    userPresets.push_back(up);
                }
        std::sort(userPresets.begin(), userPresets.end(), [](const UserPreset& a, const UserPreset& b) { return a.name < b.name; });
        pbCats.clear();
        pbCats.push_back("★ Favorites");
        pbCats.push_back("All");
        pbCats.push_back("◷ Recent");       // 5.1.1
        pbCats.push_back("✗ Rejected");     // 5.1.1
        for (uint32_t i = 0; i < kFactoryPresetCount; ++i)
            if (std::find(pbCats.begin(), pbCats.end(), kFactoryPresets[i].category) == pbCats.end()) pbCats.push_back(kFactoryPresets[i].category);
        pbCats.push_back("User");
        // per-entry caches
        const int n = totalPresets();
        pbHay.resize((size_t)n); pbCatOf.resize((size_t)n);
        for (int i = 0; i < n; ++i) {
            pbHay[(size_t)i] = lower(entryName(i) + " " + entryCat(i) + " " + entryTags(i));
            const std::string c = entryCat(i);
            pbCatOf[(size_t)i] = (int)(std::find(pbCats.begin(), pbCats.end(), c) - pbCats.begin());
        }
        pbEntsKey.clear();
        rebuildCatCounts();
    }
    void rebuildCatCounts() {
        pbCatCount.assign(pbCats.size(), 0);
        if (pbCats.size() < 2) return;
        const int n = std::min(totalPresets(), (int)pbCatOf.size());
        int rej = 0;
        for (int i = 0; i < n; ++i) {
            const std::string k = presetKey(i);
            if (isRejected(k)) { ++rej; continue; }
            const int c = pbCatOf[(size_t)i];
            if (c >= 0 && c < (int)pbCatCount.size()) ++pbCatCount[(size_t)c];
            if (isFavorite(k)) ++pbCatCount[0];
            ++pbCatCount[1];
        }
        if (pbCatCount.size() > 3) { pbCatCount[2] = (int)recent.size(); pbCatCount[3] = rej; }
        pbEntsKey.clear();
    }
    int totalPresets() const { return (int)kFactoryPresetCount + (int)userPresets.size(); }
    void stepPreset(int dir) {   // 5.1.1: the arrows skip rejected presets
        const int n = totalPresets(); int i = currentPreset < 0 ? 0 : currentPreset;
        for (int k = 0; k < n; ++k) { i = (i + dir + n) % n; if (!isRejected(presetKey(i))) break; }
        loadAnyPreset(i);
    }
    std::string entryName(int idx) const { return idx < (int)kFactoryPresetCount ? std::string(kFactoryPresets[idx].name) : userPresets[idx - kFactoryPresetCount].name; }
    std::string entryCat(int idx) const { return idx < (int)kFactoryPresetCount ? std::string(kFactoryPresets[idx].category) : std::string("User"); }
    std::string entryTags(int idx) const { return idx < (int)kFactoryPresetCount ? std::string(kFactoryPresets[idx].tags) : userPresets[idx - kFactoryPresetCount].tags; }

    void loadAnyPreset(int idx) {
        float pv[kParamCount];
        std::string st[kStateCount];
        if (idx < (int)kFactoryPresetCount) { presetValues((uint32_t)idx, pv); presetStates((uint32_t)idx, st); }
        else {
            const UserPreset& up = userPresets[idx - kFactoryPresetCount];
            std::ifstream in(drift::lib::fromU8(up.path), std::ios::binary);
            std::stringstream ss; ss << in.rdbuf();
            std::string name, cat, tags;
            if (!presetFromText(ss.str(), pv, st, &name, &cat, &tags)) return;
        }
        pushUndo(-1);
        currentPreset = idx;
        Snap s; s.v.assign(pv, pv + kParamCount); for (int k = 0; k < kStateCount; ++k) s.st[k] = st[k]; s.valid = true;
        applySnap(s);
        {   // layered factory presets name their two sources
            std::string ln;
            if (idx < (int)kFactoryPresetCount)
                for (const PresetState* q = kFactoryPresets[idx].st; q && q->key; ++q) if (std::strcmp(q->key, "layerNames") == 0) ln = q->value;
            applyLayerNames(ln.c_str()); publishLayerNames();
        }
        publishPresetId(false);
        setEnvClassic(false);   // presets always use the 3.1+ envelopes
        noteRecent(idx);
    }
    // ---- which preset is loaded: stored in the plugin ("presetId" state) so a reopened editor and a
    //      reloaded session show the right name.
    bool presetEdited = false;
    std::string orphanPresetName;      // stored id that no longer matches any preset (renamed / deleted)
    void publishPresetId(bool edited) {
        presetEdited = edited && currentPreset >= 0;
        orphanPresetName.clear();
        const std::string k = presetKey(currentPreset);
        setState("presetId", (presetEdited ? "*" + k : k).c_str());
    }
    void markEdited() {
        if (presetEdited || currentPreset < 0) return;
        publishPresetId(true);
    }
    void applyPresetId(const std::string& v) {
        presetEdited = !v.empty() && v[0] == '*';
        const std::string k = presetEdited ? v.substr(1) : v;
        currentPreset = -1; orphanPresetName.clear();
        if (k.empty()) return;
        for (int i = 0; i < totalPresets(); ++i) if (presetKey(i) == k) { currentPreset = i; return; }
        // not found: show what was stored ("F:Pads/Name" -> "Pads  /  Name")
        std::string n = k.size() > 2 ? k.substr(2) : k;
        const size_t sl = n.find('/');
        if (sl != std::string::npos) n = n.substr(0, sl) + "  /  " + n.substr(sl + 1);
        orphanPresetName = (k[0] == 'U' ? "User  /  " : "") + n;
    }
    void openPresetBrowser() {
        scanUserPresets();
        presetOpen = true;
        pbScroll = 0.f;
        pbCat = 1;
        if (currentPreset >= 0) {
            const std::string c = entryCat(currentPreset);
            for (size_t i = 0; i < pbCats.size(); ++i) if (pbCats[i] == c) pbCat = (int)i;
        }
        repaint();
    }
    const std::vector<int>& presetEntries() {
        if ((int)pbHay.size() != totalPresets()) scanUserPresets();
        std::string chipKey; for (const auto& c : pbChips) chipKey += c + ",";
        const std::string key = std::to_string(pbCat) + "|" + std::to_string(favVersion) + "|" + std::to_string(totalPresets()) + "|" + searchText + "|" + chipKey + "|" + std::to_string(rejects.size());
        if (key == pbEntsKey) return pbEnts;
        pbEntsKey = key;
        std::vector<int>& out = pbEnts;
        out.clear();
        const std::string q = lower(searchText);
        const int catIdx = std::max(0, std::min((int)pbCats.size() - 1, pbCat));
        auto chipsOk = [&](int i) {
            for (const auto& c : pbChips) if (pbHay[(size_t)i].find(c) == std::string::npos) return false;
            return true;
        };
        if (pbCat == 2 && q.empty()) {   // recent: newest first
            if (recent.empty()) loadRecent();
            for (const auto& r : recent) for (int i = 0; i < totalPresets(); ++i) if (presetKey(i) == r.first) { if (chipsOk(i)) out.push_back(i); break; }
            return out;
        }
        for (int i = 0; i < totalPresets(); ++i) {
            if (!chipsOk(i)) continue;
            const bool rj = isRejected(presetKey(i));
            if (pbCat == 3) { if (!rj) continue; out.push_back(i); continue; }
            if (rj) continue;
            if (!q.empty()) {
                const std::string& hay = pbHay[(size_t)i];
                bool all = true;
                size_t pos = 0;
                while (pos < q.size()) {
                    size_t e = q.find(' ', pos); if (e == std::string::npos) e = q.size();
                    if (e > pos && hay.find(q.substr(pos, e - pos)) == std::string::npos) { all = false; break; }
                    pos = e + 1;
                }
                if (!all) continue;
            }
            if (pbCat == 0) { if (!isFavorite(presetKey(i))) continue; }
            else if (pbCat == 1 || pbCat == 2 || !q.empty()) {}          // a search looks through the whole bank
            else if (pbCatOf[(size_t)i] != catIdx) continue;
            out.push_back(i);
        }
        return out;
    }
    void drawPresetBrowser() {
        beginPath(); rect(0, 0, kW, kHT); fillColor(Color(0, 0, 0, 0.55f)); fill();
        pbRect = { 180, 70, 1240, 860 };
        const Rect& b = pbRect;
        beginPath(); roundedRect(b.x, b.y, b.w, b.h, 9); fillColor(shade(23.f)); fill(); strokeColor(cPanelEdge); stroke();
        letterSp(2.f);
        label(b.x + 20, b.y + 26, "PRESETS", 12, cAccent, ALIGN_LEFT | ALIGN_MIDDLE);
        letterSp(0.f);
        char cnt[64]; std::snprintf(cnt, sizeof(cnt), "%u factory · %u user · %u ★", kFactoryPresetCount, (unsigned)userPresets.size(), (unsigned)favorites.size());
        label(b.x + 120, b.y + 26, cnt, 10, cDim, ALIGN_LEFT | ALIGN_MIDDLE);
        pbSearch = { b.x + 380, b.y + 12, 440, 28 };
        const bool searching = textTarget == ttSearch;
        beginPath(); roundedRect(pbSearch.x, pbSearch.y, pbSearch.w, pbSearch.h, 5); fillColor(cInset); fill(); strokeColor(searching ? cAccent : cPanelEdge); stroke();
        labelS(pbSearch.x + 10, pbSearch.cy() + 1, searchText.empty() && !searching ? std::string("search name, category or tag (e.g. roach drone bowed)") : searchText + (searching ? "_" : ""), 11,
               searchText.empty() && !searching ? cFaint : cText, ALIGN_LEFT | ALIGN_MIDDLE);
        pbIntoBtn  = { b.x + 832, b.y + 12, 198, 28 };
        button(pbIntoBtn, pbInto == 2 ? "Loads into: LAYER 2" : "Loads into: whole patch", pbInto == 2, pbIntoBtn.contains(hoverX, hoverY), 10, &cViolet);
        pbSaveBtn  = { b.x + b.w - 200, b.y + 12, 90, 28 };
        pbCloseBtn = { b.x + b.w - 100, b.y + 12, 84, 28 };
        button(pbSaveBtn, "Save…", false, pbSaveBtn.contains(hoverX, hoverY), 11);
        button(pbCloseBtn, "Close", false, pbCloseBtn.contains(hoverX, hoverY), 11);
        // 5.1.1: tag chips (AND filter) and ratings export
        {
            static const char* const chips[] = { "dark", "cosmic", "ritual", "drone", "choir", "bells", "field-recording", "just-intonation", "freeze",
                                                 "sequencer", "roach", "corbacho", "sydalesis" };
            pbChipN = (int)(sizeof(chips) / sizeof(chips[0]));
            float x = b.x + 264;
            for (int i = 0; i < pbChipN; ++i) {
                uiFontSize(9.f); Rectangle<float> bb; textBounds(0, 0, chips[i], nullptr, bb);
                const float w = bb.getWidth() + 18;
                pbChipR[i] = { x, b.y + 50, w, 20 };
                const bool on = std::find(pbChips.begin(), pbChips.end(), chips[i]) != pbChips.end();
                button(pbChipR[i], chips[i], on, pbChipR[i].contains(hoverX, hoverY), 9.f);
                x += w + 5;
            }
            pbExportBtn = { b.x + b.w - 130, b.y + 50, 114, 20 };
            button(pbExportBtn, "Export ratings", false, pbExportBtn.contains(hoverX, hoverY), 9.f);
        }
        pbCatRect = { b.x + 12, b.y + 52, 240, b.h - 64 };
        beginPath(); roundedRect(pbCatRect.x, pbCatRect.y, pbCatRect.w, pbCatRect.h, 6); fillColor(cInset); fill();
        if (pbCatCount.size() != pbCats.size()) rebuildCatCounts();
        pbCatScroll = std::min(pbCatScroll, std::max(0.f, pbCats.size() * 28.f - pbCatRect.h + 12));
        save(); scissor(pbCatRect.x, pbCatRect.y + 4, pbCatRect.w, pbCatRect.h - 8);
        for (size_t c = 0; c < pbCats.size(); ++c) {
            const Rect row = { pbCatRect.x + 6, pbCatRect.y + 6 + c * 28.f - pbCatScroll, pbCatRect.w - 12, 26 };
            if (row.y + row.h < pbCatRect.y || row.y > pbCatRect.y + pbCatRect.h) continue;
            const bool sel = searchText.empty() || c == 0 ? (int)c == pbCat : (c == 1 && pbCat != 0), hot = row.contains(hoverX, hoverY);
            if (sel || hot) { beginPath(); roundedRect(row.x, row.y, row.w, row.h, 4); fillColor(sel ? tint(cAccent, 0.20f) : shade(33.f)); fill(); }
            labelS(row.x + 10, row.y + 13.5f, pbCats[c], 12, sel ? cAccent : (c == 0 ? cWarm : cText), ALIGN_LEFT | ALIGN_MIDDLE);
            char nb[16]; std::snprintf(nb, sizeof(nb), "%d", pbCatCount[c]);
            label(row.x + row.w - 10, row.y + 13.5f, nb, 10, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
        }
        restore();
        pbListRect = { b.x + 264, b.y + 78, b.w - 276, b.h - 90 };
        const Rect& lr = pbListRect;
        beginPath(); roundedRect(lr.x, lr.y, lr.w, lr.h, 6); fillColor(cInset); fill();
        const std::vector<int>& ents = presetEntries();
        const int cols = 3;
        const float colW = (lr.w - 26) / cols, rowH = 30.f;
        const int rowsN = ((int)ents.size() + cols - 1) / cols;
        const float maxScroll = std::max(0.f, rowsN * rowH - lr.h + 12);
        pbScroll = std::min(pbScroll, maxScroll);
        // scrollbar (drag the thumb, click the track to page)
        pbBarRect = { lr.x + lr.w - 14, lr.y + 8, 8, lr.h - 16 };
        pbThumbRect = { 0, 0, 0, 0 };
        if (maxScroll > 0.f) {
            const float vis = lr.h / (rowsN * rowH + 12), th = std::max(28.f, pbBarRect.h * vis);
            const float ty = pbBarRect.y + (pbBarRect.h - th) * (pbScroll / maxScroll);
            pbThumbRect = { pbBarRect.x, ty, pbBarRect.w, th };
            beginPath(); roundedRect(pbBarRect.x, pbBarRect.y, pbBarRect.w, pbBarRect.h, 4); fillColor(shade(30.f)); fill();
            beginPath(); roundedRect(pbThumbRect.x, pbThumbRect.y, pbThumbRect.w, pbThumbRect.h, 4);
            fillColor(sd == sdPbBar || pbThumbRect.contains(hoverX, hoverY) ? cAccent : shade(70.f)); fill();
            char pos[48]; std::snprintf(pos, sizeof(pos), "%d presets", (int)ents.size());
            label(lr.x + lr.w - 20, lr.y + lr.h + 7, pos, 9, cFaint, ALIGN_RIGHT | ALIGN_MIDDLE);
        }
        save(); scissor(lr.x, lr.y + 4, lr.w, lr.h - 8);
        for (size_t i = 0; i < ents.size(); ++i) {
            const int col = (int)i % cols, row = (int)i / cols;
            const Rect rr = { lr.x + 6 + col * colW, lr.y + 6 + row * rowH - pbScroll, colW - 4, rowH - 2 };
            if (rr.y + rr.h < lr.y || rr.y > lr.y + lr.h) continue;
            const bool sel = ents[i] == currentPreset, hot = rr.contains(hoverX, hoverY) && lr.contains(hoverX, hoverY);
            if (sel || hot) { beginPath(); roundedRect(rr.x, rr.y, rr.w, rr.h, 4); fillColor(sel ? tint(cAccent, 0.20f) : shade(33.f)); fill(); }
            const bool fav = isFavorite(presetKey(ents[i]));
            label(rr.x + 12, rr.y + rr.h * 0.5f, fav ? "★" : "☆", 11, fav ? cWarm : (hot ? cDim : cFaint), ALIGN_CENTER | ALIGN_MIDDLE);
            labelS(rr.x + 26, rr.y + 10, entryName(ents[i]), 11.5f, sel ? cAccent : cText, ALIGN_LEFT | ALIGN_MIDDLE);
            // 5.1 presets carry many tags: clip name and tag line to the column
            save(); intersectScissor(rr.x, rr.y, rr.w - 6, rr.h);
            std::string sub = (pbCat <= 2 || !searchText.empty() || !pbChips.empty() ? entryCat(ents[i]) + " · " : std::string());
            if (pbCat == 2) { for (const auto& r : recent) if (r.first == presetKey(ents[i])) { sub = agoText(r.second) + " · " + sub; break; } }
            labelS(rr.x + 26, rr.y + 22, sub + visibleTags(entryTags(ents[i])), 8, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
            restore();
            // 5.1.1: CPU badge for heavy presets, reject toggle
            const int cpu = presetCpu(ents[i]);
            if (cpu >= 30) { char cb[16]; std::snprintf(cb, sizeof cb, "CPU %d%%", cpu); label(rr.x + rr.w - 30, rr.y + 10, cb, 7.5f, cpu >= 60 ? cWarn : cWarm, ALIGN_RIGHT | ALIGN_MIDDLE); }
            const bool rj = isRejected(presetKey(ents[i]));
            if (hot || rj) label(rr.x + rr.w - 14, rr.y + rr.h * 0.5f, "✗", 11, rj ? cWarn : cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        }
        restore();
        if (ents.empty()) label(lr.cx(), lr.y + 40, pbCat == 0 ? "No favourites yet: click ☆ next to a preset." : pbCat == 2 ? "Nothing loaded yet." : pbCat == 3 ? "Nothing rejected: click ✗ at the right of a preset to hide it from browsing." : (pbCats[pbCat] == "User" ? "No user presets yet: press Save… to store the current sound." : "Nothing matches."), 12, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
    }
    void pbBarDrag(float y) {
        const std::vector<int>& ents = presetEntries();
        const int rowsN = ((int)ents.size() + 2) / 3;
        const float maxScroll = std::max(0.f, rowsN * 30.f - pbListRect.h + 12), span = pbBarRect.h - pbThumbRect.h;
        if (span > 0.f) pbScroll = std::max(0.f, std::min(maxScroll, (y - pbBarGrab - pbBarRect.y) / span * maxScroll));
        repaint();
    }
    void presetBrowserClick(float x, float y, uint32_t t) {
        if (pbSearch.contains(x, y)) { textTarget = ttSearch; return; }
        if (textTarget == ttSearch) textTarget = ttNone;
        if (!pbRect.contains(x, y) || pbCloseBtn.contains(x, y)) { presetOpen = false; return; }
        if (pbSaveBtn.contains(x, y)) { presetOpen = false; openSaveDialog(); return; }
        if (pbIntoBtn.contains(x, y)) { pbInto = pbInto == 2 ? 0 : 2; return; }
        if (pbExportBtn.contains(x, y)) { exportRatings(); return; }
        for (int i = 0; i < pbChipN; ++i) if (pbChipR[i].contains(x, y)) {
            static const char* const chips[] = { "dark", "cosmic", "ritual", "drone", "choir", "bells", "field-recording", "just-intonation", "freeze",
                                                 "sequencer", "roach", "corbacho", "sydalesis" };
            auto it = std::find(pbChips.begin(), pbChips.end(), chips[i]);
            if (it != pbChips.end()) pbChips.erase(it); else pbChips.push_back(chips[i]);
            pbScroll = 0.f; return;
        }
        if (pbCatRect.contains(x, y)) {
            const int c = (int)((y - pbCatRect.y - 6 + pbCatScroll) / 28.f);
            if (c >= 0 && c < (int)pbCats.size()) { pbCat = c; pbScroll = 0.f; if (c != 0) searchText.clear(); }
            return;
        }
        if (pbBarRect.contains(x, y) && pbThumbRect.h > 0.f) {
            if (pbThumbRect.contains(x, y)) { sd = sdPbBar; pbBarGrab = y - pbThumbRect.y; }
            else pbScroll = std::max(0.f, pbScroll + (y < pbThumbRect.y ? -1.f : 1.f) * (pbListRect.h - 40.f));
            return;
        }
        if (pbListRect.contains(x, y)) {
            const std::vector<int>& ents = presetEntries();
            const int cols = 3;
            const float colW = (pbListRect.w - 26) / cols;
            const int col = (int)((x - pbListRect.x - 6) / colW), row = (int)((y - pbListRect.y - 6 + pbScroll) / 30.f);   // (list starts below the chips)
            const int i = row * cols + col;
            if (row < 0 || col < 0 || col >= cols || i >= (int)ents.size()) return;
            const float lx = x - (pbListRect.x + 6 + col * colW);
            if (lx > colW - 4 - 26) {   // 5.1.1: reject toggle
                const std::string k = presetKey(ents[i]);
                if (isRejected(k)) rejects.erase(k); else rejects.insert(k);
                ++favVersion; rebuildCatCounts(); saveRejects();
                return;
            }
            if (lx < 22) {
                const std::string k = presetKey(ents[i]);
                if (isFavorite(k)) favorites.erase(k); else favorites.insert(k);
                ++favVersion; rebuildCatCounts();
                saveFavorites();
                return;
            }
            if (pbInto == 2) loadPresetIntoLayer(ents[i], 1);
            else loadAnyPreset(ents[i]);
            if (doubleClick(5000 + ents[i], t)) presetOpen = false;
        }
    }
    void openSaveDialog() {
        saveOpen = true;
        saveMsg.clear();
        textTarget = ttPresetName;
        saveName = currentPreset >= 0 && currentPreset < (int)kFactoryPresetCount ? std::string(kFactoryPresets[currentPreset].name) + " (edit)"
                 : (currentPreset >= 0 ? entryName(currentPreset) : std::string("New Sound"));
        saveTags = currentPreset >= 0 ? entryTags(currentPreset) : std::string("user");
        repaint();
    }
    void drawSaveDialog() {
        beginPath(); rect(0, 0, kW, kHT); fillColor(Color(0, 0, 0, 0.5f)); fill();
        saveRect = { kW * 0.5f - 260, 300, 520, 170 };
        const Rect& b = saveRect;
        beginPath(); roundedRect(b.x, b.y, b.w, b.h, 9); fillColor(shade(23.f)); fill(); strokeColor(cPanelEdge); stroke();
        label(b.x + 20, b.y + 28, "SAVE USER PRESET", 12, cAccent, ALIGN_LEFT | ALIGN_MIDDLE);
        const Rect f = { b.x + 20, b.y + 50, b.w - 40, 34 };
        beginPath(); roundedRect(f.x, f.y, f.w, f.h, 5); fillColor(cInset); fill(); strokeColor(cAccent); stroke();
        labelS(f.x + 12, f.cy() + 1, saveName + "_", 13, cText, ALIGN_LEFT | ALIGN_MIDDLE);
        const std::string where = "→ " + userPresetDir() + "   tags: " + saveTags;
        labelS(b.x + 20, b.y + 100, saveMsg.empty() ? where : saveMsg, 9, saveMsg.empty() ? cFaint : cWarn, ALIGN_LEFT | ALIGN_MIDDLE);
        saveOkBtn = { b.x + b.w - 200, b.y + 122, 84, 30 };
        saveCancelBtn = { b.x + b.w - 104, b.y + 122, 84, 30 };
        button(saveOkBtn, "Save", true, saveOkBtn.contains(hoverX, hoverY), 11);
        button(saveCancelBtn, "Cancel", false, saveCancelBtn.contains(hoverX, hoverY), 11);
        label(b.x + 20, b.y + 137, "type a name · Enter saves · Esc cancels", 9, cFaint, ALIGN_LEFT | ALIGN_MIDDLE);
    }
    void saveClick(float x, float y) {
        if (saveOkBtn.contains(x, y)) { doSave(); return; }
        if (saveCancelBtn.contains(x, y) || !saveRect.contains(x, y)) { saveOpen = false; textTarget = ttNone; return; }
    }
    void doSave() {
        std::string name = saveName;
        while (!name.empty() && name.back() == ' ') name.pop_back();
        if (name.empty()) { saveMsg = "Enter a name first."; return; }
        std::error_code ec;
        std::filesystem::create_directories(drift::lib::fromU8(userPresetDir()), ec);
        const auto path = drift::lib::fromU8(userPresetDir()) / drift::lib::fromU8(name + ".df3preset");
        std::ofstream out(path, std::ios::binary);
        if (!out) { saveMsg = "Could not write the preset file."; return; }
        out << presetToText(name, "User", saveTags, hostVals, hostStates);
        out.close();
        scanUserPresets();
        for (size_t i = 0; i < userPresets.size(); ++i) if (userPresets[i].name == name) currentPreset = (int)kFactoryPresetCount + (int)i;
        publishPresetId(false);
        saveOpen = false;
        textTarget = ttNone;
    }

    // ------------------------------------------------------------------ wavetable / sample browser
    enum BrowserKind { bkTable, bkSample, bkImport, bkTuning, bkAnalyse, bkSpec, bkSpecX };
    bool sampleKind() const { return browserKind == bkSample || browserKind == bkSpec || browserKind == bkSpecX; }
    int  kindKey(int kind) const { return kind == bkSample ? skSmA : (kind == bkSpec ? skSrcA : (kind == bkSpecX ? skXsrcA : skWtA)); }
    int browserKind = bkTable;
    bool browserOpen = false;
    int browserTarget = -1;
    std::vector<drift::lib::Entry> library, samples;
    bool libraryScanned = false, samplesScanned = false;
    std::vector<std::string> categories;
    int browserCat = 0;
    float browserScroll = 0.f, catScroll = 0.f;
    Rect browserRect, browserListRect, browserCatRect, browserLoadBtn, browserCloseBtn, browserRescanBtn;

    void ensureLibrary(bool force = false) {
        if (!libraryScanned || force) { library = drift::lib::scan(); libraryScanned = true; }
        if (!samplesScanned || force) { samples = drift::lib::scanSamples(); samplesScanned = true; }
    }
    const std::vector<drift::lib::Entry>& browserList() const { return sampleKind() ? samples : library; }
    void rebuildCategories() {
        categories.clear();
        if (browserKind == bkTable) categories.push_back("Built-in");
        for (const auto& e : browserList()) if (categories.empty() || categories.back() != e.category) categories.push_back(e.category);
        if (categories.empty()) categories.push_back(sampleKind() ? "Samples" : "Built-in");
    }
    void selectBuiltin(int o, int bank) {
        if (!stateValue[skWtA + o].empty()) pushState(skWtA + o, "", true); else pushUndo(-1);
        editParameter(oscP(o, ofTable), true); setParam(oscP(o, ofTable), (float)bank); editParameter(oscP(o, ofTable), false);
    }
    void selectFile(int o, const std::string& absPath, bool sample) { pushState((sample ? skSmA : skWtA) + o, drift::lib::encode(absPath)); }
    void selectFileKind(int o, const std::string& absPath, int kind) { pushState(kindKey(kind) + o, drift::lib::encode(absPath)); }
    // 5.0: step through the Samples library for a spectral source / partner
    void stepSpec(int o, int dir, int kind) {
        ensureLibrary();
        if (samples.empty()) { openBrowserKind(o, kind); return; }
        const int k = kindKey(kind) + o;
        int cur = -1;
        if (!stateValue[k].empty()) { const std::string abs = drift::lib::resolve(stateValue[k]); for (size_t i = 0; i < samples.size(); ++i) if (samples[i].path == abs) { cur = (int)i; break; } }
        const int n = (int)samples.size();
        const int next = cur < 0 ? (dir > 0 ? 0 : n - 1) : (cur + dir + n) % n;
        selectFileKind(o, samples[next].path, kind);
    }
    void stepTable(int o, int dir) {
        ensureLibrary();
        const int total = drift::kWtBanks + (int)library.size();
        int cur = -1;
        if (stateValue[skWtA + o].empty()) cur = ival(oscP(o, ofTable));
        else {
            const std::string abs = drift::lib::resolve(stateValue[skWtA + o]);
            for (size_t i = 0; i < library.size(); ++i) if (library[i].path == abs) { cur = drift::kWtBanks + (int)i; break; }
            if (cur < 0) cur = drift::kWtBanks - 1;
        }
        const int next = (cur + dir + total) % total;
        if (next < drift::kWtBanks) selectBuiltin(o, next);
        else selectFile(o, library[next - drift::kWtBanks].path, false);
    }
    void stepSample(int o, int dir) {
        ensureLibrary();
        if (samples.empty()) { openBrowser(o, true); return; }
        int cur = -1;
        if (!stateValue[skSmA + o].empty()) {
            const std::string abs = drift::lib::resolve(stateValue[skSmA + o]);
            for (size_t i = 0; i < samples.size(); ++i) if (samples[i].path == abs) { cur = (int)i; break; }
        }
        const int n = (int)samples.size();
        const int next = cur < 0 ? (dir > 0 ? 0 : n - 1) : (cur + dir + n) % n;
        selectFile(o, samples[next].path, true);
    }
    void openBrowser(int o, bool sample) { openBrowserKind(o, sample ? bkSample : bkTable); }
    void openBrowserKind(int o, int kind) {
        ensureLibrary();
        browserKind = kind;
        rebuildCategories();
        browserTarget = o; browserOpen = true; browserScroll = 0.f; browserCat = 0; catScroll = 0.f;
        const std::string& st = stateValue[kindKey(kind) + o];
        if (!st.empty()) {
            const std::string abs = drift::lib::resolve(st);
            for (const auto& e : browserList()) if (e.path == abs) for (size_t c = 0; c < categories.size(); ++c) if (categories[c] == e.category) browserCat = (int)c;
        }
        repaint();
    }
    void entriesFor(int cat, std::vector<int>& out) const {
        out.clear();
        if (browserKind == bkTable && cat == 0) { for (int i = 0; i < drift::kWtBanks; ++i) out.push_back(-1 - i); return; }
        const auto& L = browserList();
        for (size_t i = 0; i < L.size(); ++i) if (L[i].category == categories[cat]) out.push_back((int)i);
    }
    void drawBrowser() {
        beginPath(); rect(0, 0, kW, kHT); fillColor(Color(0, 0, 0, 0.55f)); fill();
        browserRect = { 300, 80, 1000, 780 };
        const Rect& b = browserRect;
        beginPath(); roundedRect(b.x, b.y, b.w, b.h, 9); fillColor(shade(23.f)); fill(); strokeColor(cPanelEdge); stroke();
        static const char* const names[3] = { "OSC A", "OSC B", "OSC C" };
        letterSp(2.f);
        label(b.x + 20, b.y + 26, names[browserTarget], 12, cAccent, ALIGN_LEFT | ALIGN_MIDDLE);
        letterSp(0.f);
        std::string cur;
        if (browserKind == bkSample) cur = smpName[browserTarget].empty() ? std::string("none") : smpName[browserTarget];
        else if (sampleKind()) { const std::string& v = stateValue[kindKey(browserKind) + browserTarget]; cur = v.empty() ? std::string("none") : drift::lib::displayName(v); }
        else cur = tableName(browserTarget);
        const char* what = browserKind == bkSample ? "GRAIN SAMPLE  —  " : (browserKind == bkSpec ? "SPECTRAL SOURCE  —  " : (browserKind == bkSpecX ? "SPECTRAL CROSS PARTNER  —  " : "WAVETABLE  —  "));
        labelS(b.x + 90, b.y + 26, std::string(what) + cur, 12, cText, ALIGN_LEFT | ALIGN_MIDDLE);
        browserLoadBtn   = { b.x + b.w - 330, b.y + 12, 120, 28 };
        browserRescanBtn = { b.x + b.w - 200, b.y + 12, 90, 28 };
        browserCloseBtn  = { b.x + b.w - 100, b.y + 12, 84, 28 };
        button(browserLoadBtn, "Load file…", false, browserLoadBtn.contains(hoverX, hoverY), 11);
        button(browserRescanBtn, "Rescan", false, browserRescanBtn.contains(hoverX, hoverY), 11);
        button(browserCloseBtn, "Close", false, browserCloseBtn.contains(hoverX, hoverY), 11);
        browserCatRect = { b.x + 12, b.y + 52, 270, b.h - 64 };
        const Rect& cr = browserCatRect;
        beginPath(); roundedRect(cr.x, cr.y, cr.w, cr.h, 6); fillColor(cInset); fill();
        const float rowH = 26.f;
        catScroll = std::min(catScroll, std::max(0.f, categories.size() * rowH - cr.h + 12));
        save(); scissor(cr.x, cr.y + 4, cr.w, cr.h - 8);
        for (size_t c = 0; c < categories.size(); ++c) {
            const Rect row = { cr.x + 6, cr.y + 6 + c * rowH - catScroll, cr.w - 12, rowH - 2 };
            const bool sel = (int)c == browserCat, hot = row.contains(hoverX, hoverY);
            if (sel || hot) { beginPath(); roundedRect(row.x, row.y, row.w, row.h, 4); fillColor(sel ? tint(cAccent, 0.20f) : shade(33.f)); fill(); }
            labelS(row.x + 10, row.cy(), categories[c], 11.5f, sel ? cAccent : cText, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        restore();
        browserListRect = { b.x + 294, b.y + 52, b.w - 306, b.h - 64 };
        const Rect& lr = browserListRect;
        beginPath(); roundedRect(lr.x, lr.y, lr.w, lr.h, 6); fillColor(cInset); fill();
        std::vector<int> ents; entriesFor(browserCat, ents);
        const int cols = 2;
        const float colW = (lr.w - 12) / cols;
        const int rowsN = ((int)ents.size() + cols - 1) / cols;
        browserScroll = std::min(browserScroll, std::max(0.f, rowsN * rowH - lr.h + 12));
        const int sk = kindKey(browserKind) + browserTarget;
        const std::string curAbs = stateValue[sk].empty() ? "" : drift::lib::resolve(stateValue[sk]);
        save(); scissor(lr.x, lr.y + 4, lr.w, lr.h - 8);
        const auto& L = browserList();
        for (size_t i = 0; i < ents.size(); ++i) {
            const int col = (int)i % cols, row = (int)i / cols;
            const Rect rr = { lr.x + 6 + col * colW, lr.y + 6 + row * rowH - browserScroll, colW - 4, rowH - 2 };
            if (rr.y + rr.h < lr.y || rr.y > lr.y + lr.h) continue;
            const int e = ents[i];
            const char* nm; bool sel;
            if (e < 0) { nm = kTableNames[-1 - e]; sel = stateValue[sk].empty() && ival(oscP(browserTarget, ofTable)) == -1 - e; }
            else { nm = L[e].name.c_str(); sel = L[e].path == curAbs; }
            const bool hot = rr.contains(hoverX, hoverY) && lr.contains(hoverX, hoverY);
            if (sel || hot) { beginPath(); roundedRect(rr.x, rr.y, rr.w, rr.h, 4); fillColor(sel ? tint(cAccent, 0.20f) : shade(33.f)); fill(); }
            label(rr.x + 10, rr.cy(), nm, 11.5f, sel ? cAccent : cText, ALIGN_LEFT | ALIGN_MIDDLE);
        }
        restore();
        if (ents.empty()) {
            const std::string dir = sampleKind() ? drift::lib::samplesDir() : drift::lib::libraryDir();
            label(lr.cx(), lr.y + 40, "Nothing here.", 12, cDim, ALIGN_CENTER | ALIGN_MIDDLE);
            labelS(lr.cx(), lr.y + 62, "Folder: " + dir, 10, cFaint, ALIGN_CENTER | ALIGN_MIDDLE);
        }
    }
    void browserClick(float x, float y) {
        if (!browserRect.contains(x, y) || browserCloseBtn.contains(x, y)) { browserOpen = false; return; }
        if (browserRescanBtn.contains(x, y)) { ensureLibrary(true); rebuildCategories(); browserCat = std::min(browserCat, (int)categories.size() - 1); return; }
        if (browserLoadBtn.contains(x, y)) {
            FileBrowserOptions opts;
            opts.title = browserKind == bkSample ? "Load grain sample (.wav)" : (sampleKind() ? "Load a recording for the spectral oscillator (.wav)" : "Load wavetable (.wav)");
            const std::string dir = sampleKind() ? drift::lib::samplesDir() : drift::lib::libraryDir();
            opts.startDir = dir.c_str();
            openFileBrowser(opts);
            return;
        }
        const float rowH = 26.f;
        if (browserCatRect.contains(x, y)) {
            const int c = (int)((y - browserCatRect.y - 6 + catScroll) / rowH);
            if (c >= 0 && c < (int)categories.size()) { browserCat = c; browserScroll = 0.f; }
            return;
        }
        if (browserListRect.contains(x, y)) {
            std::vector<int> ents; entriesFor(browserCat, ents);
            const float colW = (browserListRect.w - 12) / 2;
            const int col = (int)((x - browserListRect.x - 6) / colW);
            const int row = (int)((y - browserListRect.y - 6 + browserScroll) / rowH);
            const int i = row * 2 + std::max(0, std::min(1, col));
            if (i < 0 || i >= (int)ents.size()) return;
            const int e = ents[i];
            if (e < 0) selectBuiltin(browserTarget, -1 - e);
            else selectFileKind(browserTarget, browserList()[e].path, browserKind);
        }
    }
    void uiFileBrowserSelected(const char* filename) override {
        if (filename == nullptr) return;
        if (browserKind == bkTuning) { tuningFileSelected(filename); browserKind = bkTable; repaint(); return; }
        if (browserKind == bkAnalyse) { analyseSelected(filename); browserKind = bkTable; repaint(); return; }
        if (browserKind == bkImport) { if (importTarget >= 0) importSpectrum(importTarget, filename); browserKind = bkTable; repaint(); return; }
        if (browserTarget < 0) return;
        selectFileKind(browserTarget, filename, browserKind);
        browserOpen = false;
        repaint();
    }
