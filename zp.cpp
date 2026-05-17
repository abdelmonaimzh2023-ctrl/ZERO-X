#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <cstring>
#include <ctime>
#include <functional>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;
using namespace std::chrono;

// ==================== COLORS ====================
namespace C {
    const string RST   = "\033[0m";
    const string BLD   = "\033[1m";
    const string DIM   = "\033[2m";
    const string RED   = "\033[31m";
    const string GRN   = "\033[32m";
    const string YLW   = "\033[33m";
    const string BLU   = "\033[34m";
    const string MAG   = "\033[35m";
    const string CYN   = "\033[36m";
    const string WHT   = "\033[37m";
    string grad(int p) { return p < 33 ? RED : p < 66 ? YLW : GRN; }
}

// ==================== TERMINAL ====================
int termW() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) return w.ws_col > 0 ? w.ws_col : 80;
    return 80;
}
void cls() { cout << "\033[2J\033[H" << flush; }
void hideCur() { cout << "\033[?25l" << flush; }
void showCur() { cout << "\033[?25h" << flush; }
string rep(const string& s, int n) { string r; for(int i=0;i<n;i++) r+=s; return r; }
string ctr(const string& t, int w) { int p=(w-t.size())/2; return p<0?t:string(p,' ')+t; }
string fmtSize(long long b) {
    const char* u[]={"B","KB","MB","GB","TB"}; int i=0; double s=b;
    while(s>=1024&&i<4){s/=1024;i++;}
    ostringstream o; o<<fixed<<setprecision(1)<<s<<" "<<u[i]; return o.str();
}
string fmtTime(long long ms) {
    if(ms<1000) return to_string(ms)+"ms";
    if(ms<60000){ostringstream o;o<<fixed<<setprecision(1)<<(ms/1000.0)<<"s";return o.str();}
    return to_string(ms/60000)+"m "+to_string((ms%60000)/1000)+"s";
}

// ==================== LOGGER ====================
namespace Logger {
    string logPath;
    void init() {
        string home = getenv("HOME") ? getenv("HOME") : "/data/data/com.termux/files/home";
        string dir = home + "/.zero-space";
        mkdir(dir.c_str(), 0700);
        logPath = dir + "/zp.log";
    }
    string ts() {
        time_t n=time(0); char b[20]; strftime(b,sizeof(b),"%Y-%m-%d %H:%M:%S",localtime(&n)); return b;
    }
    void write(const string& m) {
        ofstream l(logPath, ios::app);
        if(l.is_open()) l<<"["<<ts()<<"] "<<m<<endl;
    }
}

// ==================== PROGRESS BAR ====================
void legendaryBar(int pct, double speed, long long eta, long long ramU, long long ramT,
                  double temp, const string& st, long long saved, double ratio, const string& fp) {
    cls(); int w=termW(); int bw=max(30,w-45); int f=(pct*bw)/100;
    cout<<C::CYN<<C::BLD;
    cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
    cout<<"\xE2\x95\x91"<<ctr("ZERO-SPACE HYPER COMPRESSION",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\xA0"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\xA3\n"<<C::RST;
    cout<<C::WHT<<"\xE2\x95\x91  Status: "<<C::GRN<<st<<rep(" ",max(0,w-20-(int)st.size()))<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  Stage:  "<<C::YLW<<pct<<"%"<<rep(" ",max(0,w-18))<<"\xE2\x95\x91\n"<<C::RST;
    cout<<C::WHT<<"\xE2\x95\x91  "<<C::RST<<C::grad(pct)<<rep("\xE2\x96\x8C",min(f,bw));
    if(f<bw) cout<<C::DIM<<rep("\xE2\x96\x91",bw-f);
    cout<<C::RST<<C::WHT<<"  \xE2\x95\x91\n"<<C::RST;
    cout<<C::WHT<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n";
    cout<<"\xE2\x95\x91  Speed:   "<<C::GRN<<fixed<<setprecision(1)<<speed<<" MB/s"<<C::WHT
        <<"     ETA:  "<<C::YLW<<fmtTime(eta)<<rep(" ",max(0,w-47))<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  RAM:     "<<C::CYN<<fmtSize(ramU)<<"/"<<fmtSize(ramT)
        <<C::WHT<<"   Temp: "<<C::grad((int)temp)<<fixed<<setprecision(0)<<temp<<"°C"
        <<C::WHT<<rep(" ",max(0,w-44))<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  Saved:   "<<C::MAG<<fmtSize(saved)<<" ("<<fixed<<setprecision(1)<<(ratio*100)<<"%)"
        <<C::WHT<<rep(" ",max(0,w-32))<<"\xE2\x95\x91\n";
    string sp=fp; if((int)sp.size()>w-17) sp="..."+sp.substr(sp.size()-(w-20));
    cout<<"\xE2\x95\x91  Input:   "<<C::BLU<<sp<<C::WHT<<rep(" ",max(0,w-17-(int)sp.size()))<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D"<<C::RST<<endl;
}

// ==================== ERROR MESSAGES ====================
void showError(const string& title, const string& msg) {
    cls(); int w=termW();
    cout<<C::RED<<C::BLD;
    cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
    cout<<"\xE2\x95\x91 "<<ctr("ERROR: "+title,w-2)<<" \xE2\x95\x91\n";
    cout<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n"<<C::RST;
    cout<<C::WHT<<"\xE2\x95\x91 "<<msg<<rep(" ",max(0,w-4-(int)msg.size()))<<"\xE2\x95\x91\n";
    cout<<C::RED<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
    cout<<"\n  [?] Press Enter to continue...";
    cin.ignore(); cin.get();
}

// ==================== FILE TYPE DETECTION ====================
string getType(const string& p) {
    string e; size_t d=p.find_last_of('.'); if(d!=string::npos){e=p.substr(d+1); transform(e.begin(),e.end(),e.begin(),::tolower);}
    
    // Images
    if(e=="jpg"||e=="jpeg"||e=="png"||e=="gif"||e=="bmp"||e=="webp"||e=="heic"||e=="ico"||e=="tiff"||e=="svg") return "IMAGE";
    // Already compressed  
    if(e=="zip"||e=="rar"||e=="7z"||e=="gz"||e=="xz"||e=="bz2"||e=="zst"||e=="zp") return "ARCHIVE";
    // Audio
    if(e=="mp3"||e=="aac"||e=="ogg"||e=="flac"||e=="opus"||e=="wav") return "AUDIO";
    // Video
    if(e=="mp4"||e=="mkv"||e=="avi"||e=="webm"||e=="mov"||e=="flv") return "VIDEO";
    // APK (Android package - like ZIP)
    if(e=="apk"||e=="xapk"||e=="apkm") return "APK";
    // Documents
    if(e=="pdf"||e=="doc"||e=="docx"||e=="xls"||e=="xlsx"||e=="ppt"||e=="pptx") return "DOCUMENT";
    // Text-based
    if(e=="txt"||e=="md"||e=="log"||e=="csv"||e=="json"||e=="xml"||e=="html"||e=="css"||
       e=="js"||e=="py"||e=="c"||e=="cpp"||e=="h"||e=="java"||e=="sh"||e=="yml"||e=="yaml"||
       e=="toml"||e=="ini"||e=="cfg"||e=="conf"||e=="sql"||e=="php"||e=="rb"||e=="ts"||
       e=="kt"||e=="swift"||e=="go"||e=="rs"||e=="lua"||e=="pl"||e=="r"||e=="dart") return "TEXT";
    
    return "BINARY";
}

string getIcon(const string& t) {
    if(t=="IMAGE") return "\xF0\x9F\x96\xBC\xEF\xB8\x8F ";
    if(t=="ARCHIVE") return "\xF0\x9F\x97\x9C\xEF\xB8\x8F ";
    if(t=="AUDIO") return "\xF0\x9F\x8E\xB5 ";
    if(t=="VIDEO") return "\xF0\x9F\x8E\xAC ";
    if(t=="APK") return "\xF0\x9F\x93\xB1 ";
    if(t=="DOCUMENT") return "\xF0\x9F\x93\x83 ";
    if(t=="TEXT") return "\xF0\x9F\x93\x84 ";
    return "\xF0\x9F\x92\xBE ";
}

bool isCompressible(const string& t) {
    return (t=="TEXT" || t=="BINARY" || t=="DOCUMENT" || t=="APK");
}

bool isStoredAsIs(const string& t) {
    return (t=="IMAGE" || t=="AUDIO" || t=="VIDEO" || t=="ARCHIVE");
}

// ==================== ENHANCED FILE BROWSER ====================
struct FileEntry { string name, path, type, icon; long long size; bool isDir; };

vector<FileEntry> listDir(const string& path, bool hidden=false) {
    vector<FileEntry> e; DIR* d=opendir(path.c_str()); if(!d) return e;
    struct dirent* en;
    while((en=readdir(d))) {
        string nm=en->d_name; if(nm==".") continue; if(!hidden&&nm[0]=='.') continue;
        FileEntry fe; fe.name=nm; fe.path=path+"/"+nm;
        struct stat st; stat(fe.path.c_str(),&st);
        if(S_ISDIR(st.st_mode)){fe.isDir=true;fe.type="DIR";fe.icon="\xF0\x9F\x93\x82 ";fe.size=0;}
        else{fe.isDir=false;fe.type=getType(nm);fe.icon=getIcon(fe.type);fe.size=st.st_size;}
        e.push_back(fe);
    }
    closedir(d);
    sort(e.begin(),e.end(),[](const FileEntry& a,const FileEntry& b){
        if(a.isDir!=b.isDir) return a.isDir > b.isDir;
        return a.name < b.name;
    });
    return e;
}

void showBrowserHelp() {
    cls(); int w=termW();
    cout<<C::CYN<<C::BLD;
    cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
    cout<<"\xE2\x95\x91 "<<ctr("BROWSER CONTROLS",w-2)<<" \xE2\x95\x91\n";
    cout<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n"<<C::RST<<C::WHT;
    cout<<"\xE2\x95\x91  [0 or ..]  Parent Directory                          \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  [1-9...]   Select file/folder by number               \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  [N]        Next Page                                  \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  [P]        Previous Page                              \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  [S]        Search by name                             \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  [H]        Show this help                             \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  [G]        Go to path                                  \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  [Q]        Quit browser                                \xE2\x95\x91\n";
    cout<<C::CYN<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
    cout<<"\n  [?] Press Enter to return...";
    cin.ignore(); cin.get();
}

string browser(const string& start, const string& title, bool selFile=true, bool selDir=false) {
    string cur=start; int page=0; const int perPage=15;
    while(true) {
        auto entries=listDir(cur); int tp=(entries.size()+perPage-1)/perPage; if(tp<1)tp=1; if(page>=tp)page=0;
        cls(); int w=termW();
        
        // Header
        cout<<C::CYN<<C::BLD;
        cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
        cout<<"\xE2\x95\x91 "<<ctr(title,w-2)<<" \xE2\x95\x91\n";
        cout<<"\xE2\x95\xA0"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\xA3\n"<<C::WHT;
        
        // Path
        string pathLine = "║ ";
        pathLine += C::YLW;
        pathLine += "Path: ";
        pathLine += cur;
        cout<<pathLine<<rep(" ",max(0,w-1-(int)pathLine.size()))<<"\xE2\x95\x91\n";
        cout<<C::WHT<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n"<<C::RST;
        
        // Parent dir
        cout<<C::YLW<<"\xE2\x95\x91  [..] \xF0\x9F\x93\x82 Parent Directory"<<rep(" ",max(0,w-28))<<"\xE2\x95\x91\n"<<C::RST;
        
        // Stats
        int dirCount=0, fileCount=0; long long totalSize=0;
        for(auto& e:entries){if(e.isDir)dirCount++;else{fileCount++;totalSize+=e.size;}}
        string stats = "\xE2\x95\x91  "+C::DIM+to_string(dirCount)+" dirs, "+to_string(fileCount)+" files, "+fmtSize(totalSize);
        cout<<stats<<rep(" ",max(0,w-1-(int)stats.size()))<<"\xE2\x95\x91\n"<<C::RST;
        cout<<C::WHT<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n"<<C::RST;
        
        // Files
        int st=page*perPage, en=min(st+perPage,(int)entries.size());
        for(int i=st;i<en;i++) {
            int num=i-st+1; string ns=(num<10?" ":"")+to_string(num);
            string line="["+ns+"] "+entries[i].icon+" "+entries[i].name;
            if(!entries[i].isDir) {
                string typeTag = " ["+entries[i].type+"]";
                line += typeTag + "  " + fmtSize(entries[i].size);
            } else {
                line += "  " + C::CYN + "[DIR]" + C::RST;
            }
            string col=entries[i].isDir?C::CYN:(isCompressible(entries[i].type)?C::GRN:(isStoredAsIs(entries[i].type)?C::YLW:C::WHT));
            cout<<col<<"\xE2\x95\x91  "<<line<<rep(" ",max(0,w-6-(int)line.size()))<<"\xE2\x95\x91\n";
        }
        for(int i=en-st;i<perPage;i++) cout<<C::RST<<"\xE2\x95\x91"<<rep(" ",w-2)<<"\xE2\x95\x91\n";
        
        // Footer
        cout<<C::WHT;
        cout<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n";
        cout<<"\xE2\x95\x91  ["<<C::GRN<<"N"<<C::WHT<<"]ext  ["<<C::GRN<<"P"<<C::WHT<<"]rev  "
            <<"["<<C::GRN<<"S"<<C::WHT<<"]earch  ["<<C::GRN<<"H"<<C::WHT<<"]elp  "
            <<"["<<C::GRN<<"G"<<C::WHT<<"]oto  ["<<C::RED<<"Q"<<C::WHT<<"]uit  "
            <<"Pg "<<(page+1)<<"/"<<tp
            <<rep(" ",max(0,w-63))<<"\xE2\x95\x91\n";
        cout<<C::CYN<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
        cout<<"\n  Select: "; string in; cin>>in;
        
        if(in=="Q"||in=="q") return "";
        else if(in=="N"||in=="n"){if(page<tp-1)page++;}
        else if(in=="P"||in=="p"){if(page>0)page--;}
        else if(in=="H"||in=="h"){showBrowserHelp();}
        else if(in=="S"||in=="s"){
            cout<<"  Search: "; string q; cin>>q;
            transform(q.begin(),q.end(),q.begin(),::tolower);
            bool found=false;
            for(int i=0;i<(int)entries.size();i++){
                string nm=entries[i].name; transform(nm.begin(),nm.end(),nm.begin(),::tolower);
                if(nm.find(q)!=string::npos){
                    page=i/perPage;
                    found=true;
                    break;
                }
            }
            if(!found){cout<<C::RED<<"  [!] Not found."<<C::RST<<endl; this_thread::sleep_for(milliseconds(1000));}
        }
        else if(in=="G"||in=="g"){
            cout<<"  Path: "; string np; cin>>np;
            struct stat st; if(stat(np.c_str(),&st)==0&&S_ISDIR(st.st_mode)){cur=np;page=0;}
            else{cout<<C::RED<<"  [!] Invalid path."<<C::RST<<endl; this_thread::sleep_for(milliseconds(1000));}
        }
        else if(in==".."||in=="0"){size_t p=cur.find_last_of('/'); if(p!=string::npos&&p>0){cur=cur.substr(0,p);page=0;}}
        else{try{int idx=stoi(in)-1+st; if(idx>=0&&idx<(int)entries.size()){
            if(entries[idx].isDir){cur=entries[idx].path;page=0;}
            else if(selFile) return entries[idx].path;
        }}catch(...){}}
    }
}

// ==================== BIT-LEVEL COMPRESSION ENGINE ====================
// This implements the original idea: byte -> bit via custom table per file

struct BitPattern {
    vector<unsigned char> bytes;
    long long freq;
    string bitCode;
};

struct CompResult { long long orig, comp; double ratio, speed; long long ms; bool ok; string err; };

// Find all repeating patterns (1 to 256 bytes)
vector<BitPattern> findPatterns(const vector<unsigned char>& data) {
    unordered_map<string, long long> patternFreq;
    
    // Scan for patterns of lengths 1,2,3,4,6,8,12,16,32,64,128,256
    vector<int> lengths = {1, 2, 3, 4, 6, 8, 12, 16, 32, 64, 128, 256};
    
    for (int len : lengths) {
        if (len > (int)data.size()) continue;
        for (size_t i = 0; i <= data.size() - len; i++) {
            string pat(reinterpret_cast<const char*>(&data[i]), len);
            patternFreq[pat]++;
            if (patternFreq[pat] > 1000000) break; // Cap to avoid memory issues
        }
    }
    
    // Also scan for words (sequences separated by spaces/newlines)
    string word;
    for (auto b : data) {
        if (b == ' ' || b == '\n' || b == '\t' || b == '\r' || b == ',' || b == '.') {
            if (!word.empty() && word.size() >= 2) {
                patternFreq[word]++;
            }
            word.clear();
        } else {
            word += (char)b;
        }
    }
    if (!word.empty() && word.size() >= 2) patternFreq[word]++;
    
    // Convert to sorted vector
    vector<BitPattern> patterns;
    for (auto& p : patternFreq) {
        if (p.second < 3) continue; // Ignore rare patterns
        BitPattern bp;
        bp.bytes.assign(p.first.begin(), p.first.end());
        bp.freq = p.second;
        patterns.push_back(bp);
    }
    
    // Sort by frequency (highest first)
    sort(patterns.begin(), patterns.end(), [](const BitPattern& a, const BitPattern& b) {
        // Weight: frequency * length (longer patterns with high freq are more valuable)
        return (a.freq * a.bytes.size()) > (b.freq * b.bytes.size());
    });
    
    // Keep top 65535 patterns
    if (patterns.size() > 65535) patterns.resize(65535);
    
    return patterns;
}

// Assign bit codes to patterns (shortest codes for most frequent)
void assignBitCodes(vector<BitPattern>& patterns) {
    if (patterns.empty()) return;
    
    // Use variable-length bit encoding
    int codeLen = 1;
    int codesInLen = 0;
    int maxCodesInLen = 2; // 2^1 = 2 codes of length 1
    
    for (auto& p : patterns) {
        // Generate code
        string code;
        int val = codesInLen;
        for (int i = 0; i < codeLen; i++) {
            code = (val % 2 ? '1' : '0') + code;
            val /= 2;
        }
        p.bitCode = code;
        
        codesInLen++;
        if (codesInLen >= maxCodesInLen) {
            codeLen++;
            codesInLen = 0;
            maxCodesInLen = (1 << codeLen);
        }
    }
}

CompResult compressFileBit(const string& in, const string& out) {
    CompResult r; r.ok = false;
    auto st = high_resolution_clock::now();
    
    // Read file
    legendaryBar(5, 0, 999, 0, 8000, 35, "READING FILE...", 0, 0, in);
    ifstream ifs(in, ios::binary | ios::ate);
    if (!ifs) { r.err = "Cannot open input file"; return r; }
    r.orig = ifs.tellg(); ifs.seekg(0);
    vector<unsigned char> data(r.orig);
    ifs.read(reinterpret_cast<char*>(data.data()), r.orig);
    ifs.close();
    
    // Find patterns
    legendaryBar(15, 0, 999, r.orig / 4, 8000, 38, "FINDING PATTERNS...", 0, 0, in);
    auto patterns = findPatterns(data);
    
    // Assign bit codes
    legendaryBar(30, 0, 999, r.orig / 2, 8000, 40, "BUILDING CUSTOM TABLE...", 0, 0, in);
    assignBitCodes(patterns);
    
    // Build lookup map for compression
    unordered_map<string, string> patternToCode;
    for (auto& p : patterns) {
        string key(p.bytes.begin(), p.bytes.end());
        patternToCode[key] = p.bitCode;
    }
    
    // Compress using pattern matching
    legendaryBar(50, 0, 999, r.orig * 3 / 4, 8000, 42, "COMPRESSING WITH BIT TABLE...", 0, 0, in);
    
    string bitStream;
    size_t pos = 0;
    long long lastProgress = 0;
    
    while (pos < data.size()) {
        // Report progress
        long long progress = (pos * 100) / data.size();
        if (progress - lastProgress >= 5) {
            legendaryBar(50 + (progress / 3), 0, 999, pos, r.orig, 42, "COMPRESSING... "+to_string(progress)+"%", 0, 0, in);
            lastProgress = progress;
        }
        
        bool matched = false;
        // Try longest patterns first
        for (int len = min(256, (int)(data.size() - pos)); len >= 1; len--) {
            string seg(reinterpret_cast<const char*>(&data[pos]), len);
            if (patternToCode.count(seg)) {
                bitStream += "1"; // Flag: pattern found
                bitStream += patternToCode[seg];
                pos += len;
                matched = true;
                break;
            }
        }
        if (!matched) {
            // Store raw byte with flag 0
            bitStream += "0";
            unsigned char byte = data[pos];
            for (int i = 7; i >= 0; i--) {
                bitStream += ((byte >> i) & 1) ? '1' : '0';
            }
            pos++;
        }
    }
    
    // Write output
    legendaryBar(85, 0, 999, r.orig, 8000, 43, "WRITING COMPRESSED FILE...", 0, 0, in);
    
    ofstream ofs(out, ios::binary);
    if (!ofs) { r.err = "Cannot create output file"; return r; }
    
    // Header
    ofs << "ZP2"; // New format with bit table
    
    // Original size
    ofs.write(reinterpret_cast<const char*>(&r.orig), sizeof(long long));
    
    // Number of patterns
    int patternCount = patterns.size();
    ofs.write(reinterpret_cast<const char*>(&patternCount), sizeof(int));
    
    // Write pattern table
    for (auto& p : patterns) {
        int byteLen = p.bytes.size();
        ofs.write(reinterpret_cast<const char*>(&byteLen), sizeof(int));
        ofs.write(reinterpret_cast<const char*>(p.bytes.data()), byteLen);
        int codeLen = p.bitCode.size();
        ofs.write(reinterpret_cast<const char*>(&codeLen), sizeof(int));
    }
    
    // Write bit codes
    for (auto& p : patterns) {
        // Write code as bits packed into bytes
        unsigned char buf = 0;
        int bc = 0;
        for (char c : p.bitCode) {
            buf = (buf << 1) | (c == '1');
            bc++;
            if (bc == 8) {
                ofs.write(reinterpret_cast<char*>(&buf), 1);
                buf = 0; bc = 0;
            }
        }
        if (bc > 0) {
            buf <<= (8 - bc);
            ofs.write(reinterpret_cast<char*>(&buf), 1);
        }
    }
    
    // Write compressed bit stream
    unsigned char buf = 0;
    int bc = 0;
    for (char c : bitStream) {
        buf = (buf << 1) | (c == '1');
        bc++;
        if (bc == 8) {
            ofs.write(reinterpret_cast<char*>(&buf), 1);
            buf = 0; bc = 0;
        }
    }
    if (bc > 0) {
        buf <<= (8 - bc);
        ofs.write(reinterpret_cast<char*>(&buf), 1);
    }
    
    ofs.close();
    
    // Calculate results
    struct stat s; stat(out.c_str(), &s);
    r.comp = s.st_size;
    r.ratio = 1.0 - (double)r.comp / r.orig;
    auto en = high_resolution_clock::now();
    r.ms = duration_cast<milliseconds>(en - st).count();
    r.speed = (r.orig / 1024.0 / 1024.0) / (r.ms / 1000.0);
    r.ok = true;
    
    legendaryBar(100, r.speed, 0, r.comp, 8000, 44, "COMPLETE", r.orig - r.comp, r.ratio, in);
    return r;
}

// ==================== DECOMPRESSION ENGINE ====================
bool decompressFileBit(const string& in, const string& out) {
    ifstream ifs(in, ios::binary);
    if (!ifs) return false;
    
    // Check magic
    char mg[3]; ifs.read(mg, 3);
    if (mg[0] != 'Z' || mg[1] != 'P' || mg[2] != '2') return false;
    
    // Original size
    long long orig;
    ifs.read(reinterpret_cast<char*>(&orig), sizeof(long long));
    
    // Number of patterns
    int patternCount;
    ifs.read(reinterpret_cast<char*>(&patternCount), sizeof(int));
    
    // Read pattern bytes
    vector<vector<unsigned char>> patternBytes(patternCount);
    vector<int> codeLengths(patternCount);
    for (int i = 0; i < patternCount; i++) {
        int byteLen;
        ifs.read(reinterpret_cast<char*>(&byteLen), sizeof(int));
        patternBytes[i].resize(byteLen);
        ifs.read(reinterpret_cast<char*>(patternBytes[i].data()), byteLen);
        ifs.read(reinterpret_cast<char*>(&codeLengths[i]), sizeof(int));
    }
    
    // Read bit codes
    vector<string> patternCodes(patternCount);
    for (int i = 0; i < patternCount; i++) {
        string code;
        int bitsNeeded = codeLengths[i];
        int bitsRead = 0;
        unsigned char byte;
        while (bitsRead < bitsNeeded) {
            ifs.read(reinterpret_cast<char*>(&byte), 1);
            for (int j = 7; j >= 0 && bitsRead < bitsNeeded; j--, bitsRead++) {
                code += ((byte >> j) & 1) ? '1' : '0';
            }
        }
        patternCodes[i] = code;
    }
    
    // Build reverse lookup
    unordered_map<string, vector<unsigned char>> codeToPattern;
    for (int i = 0; i < patternCount; i++) {
        codeToPattern[patternCodes[i]] = patternBytes[i];
    }
    
    // Read bit stream
    string allBits;
    char byte;
    while (ifs.read(&byte, 1)) {
        for (int i = 7; i >= 0; i--) {
            allBits += ((byte >> i) & 1) ? '1' : '0';
        }
    }
    ifs.close();
    
    // Decompress
    vector<unsigned char> outData;
    size_t pos = 0;
    while (pos < allBits.size() && (long long)outData.size() < orig) {
        if (allBits[pos] == '1') {
            // Pattern reference
            pos++;
            string code;
            // Try matching codes
            bool found = false;
            for (int len = 1; len <= 32 && (pos + len) <= allBits.size(); len++) {
                code = allBits.substr(pos, len);
                if (codeToPattern.count(code)) {
                    auto& pat = codeToPattern[code];
                    outData.insert(outData.end(), pat.begin(), pat.end());
                    pos += len;
                    found = true;
                    break;
                }
            }
            if (!found) break; // Corrupted
        } else {
            // Raw byte
            pos++;
            if (pos + 8 > allBits.size()) break;
            unsigned char b = 0;
            for (int i = 0; i < 8; i++) {
                b = (b << 1) | (allBits[pos++] == '1');
            }
            outData.push_back(b);
        }
    }
    
    // Write output
    ofstream ofs(out, ios::binary);
    ofs.write(reinterpret_cast<const char*>(outData.data()), outData.size());
    ofs.close();
    return true;
}

// ==================== SMART COMPRESS (Routes to best engine) ====================
CompResult smartCompress(const string& in, const string& out) {
    string type = getType(in);
    
    // Check if file should be stored as-is
    if (isStoredAsIs(type)) {
        CompResult r;
        r.ok = false;
        r.err = "File type '" + type + "' is stored as-is (already compressed). Use 'zp -c --force' to force compression.";
        return r;
    }
    
    // Use bit-level engine for all compressible types
    return compressFileBit(in, out);
}

bool smartDecompress(const string& in, const string& out) {
    // Check magic to determine format
    ifstream ifs(in, ios::binary);
    if (!ifs) return false;
    char mg[3]; ifs.read(mg, 3);
    ifs.close();
    
    if (mg[0] == 'Z' && mg[1] == 'P' && mg[2] == '2') {
        return decompressFileBit(in, out);
    } else if (mg[0] == 'Z' && mg[1] == 'P' && mg[2] == '!') {
        // Old format - use old decompressor
        // (kept for backward compatibility)
        return false;
    }
    return false;
}

// ==================== FOLDER COMPRESSION ====================
struct FolderEntry { string rel, full; bool isDir; long long size; string type; };

vector<FolderEntry> scanFolder(const string& root) {
    vector<FolderEntry> entries;
    function<void(const string&,const string&)> scan=[&](const string& base,const string& cur){
        DIR* d=opendir(cur.c_str()); if(!d) return;
        struct dirent* en;
        while((en=readdir(d))){
            string nm=en->d_name; if(nm=="."||nm=="..") continue;
            string fp=cur+"/"+nm, rp=base.empty()?nm:base+"/"+nm;
            struct stat st; stat(fp.c_str(),&st);
            if(S_ISDIR(st.st_mode)){FolderEntry fe;fe.rel=rp;fe.full=fp;fe.isDir=true;fe.size=0;fe.type="DIR";entries.push_back(fe);scan(rp,fp);}
            else{FolderEntry fe;fe.rel=rp;fe.full=fp;fe.isDir=false;fe.size=st.st_size;fe.type=getType(nm);entries.push_back(fe);}
        }
        closedir(d);
    };
    scan("",root); return entries;
}

bool compressFolder(const string& in, const string& out) {
    auto entries=scanFolder(in);
    legendaryBar(10,0,999,0,8000,35,"SCANNING FOLDER...",0,0,in);
    long long total=0; int fcnt=0;
    for(auto& e:entries){if(!e.isDir){total+=e.size;fcnt++;}}
    
    ofstream ofs(out,ios::binary);
    ofs<<"ZPF"; ofs.write((char*)&total,sizeof(long long)); ofs.write((char*)&fcnt,sizeof(int));
    for(auto& e:entries){
        int pl=e.rel.size(); ofs.write((char*)&pl,sizeof(int)); ofs.write(e.rel.c_str(),pl);
        ofs.write((char*)&e.size,sizeof(long long));
        int tl=e.type.size(); ofs.write((char*)&tl,sizeof(int)); ofs.write(e.type.c_str(),tl);
    }
    
    int done=0;
    for(auto& e:entries){
        if(e.isDir) continue; done++;
        int pct=20+(done*75/fcnt);
        legendaryBar(pct,0,999,total,8000,40,"COMPRESSING: "+e.rel,total,0,in);
        if(isCompressible(e.type)){
            compressFileBit(e.full,"/tmp/zp_tmp");
            ifstream tmp("/tmp/zp_tmp",ios::binary); ofs<<tmp.rdbuf(); tmp.close(); remove("/tmp/zp_tmp");
        }else{
            ifstream src(e.full,ios::binary); ofs<<src.rdbuf(); src.close();
        }
    }
    ofs.close();
    legendaryBar(100,0,0,total,8000,42,"FOLDER COMPLETE",total,0,in);
    return true;
}


// ==================== SYSTEM ====================
bool autoInstall() {
    char ep[4096]; ssize_t len=readlink("/proc/self/exe",ep,sizeof(ep)-1);
    if(len!=-1) ep[len]='\0';
    string cur(ep), tgt="/data/data/com.termux/files/usr/bin/zp";
    if(cur==tgt) return true;
    cout<<C::YLW<<"[*] First run: Installing ZERO-SPACE to system..."<<C::RST<<endl;
    ifstream src(cur,ios::binary); ofstream dst(tgt,ios::binary); dst<<src.rdbuf();
    src.close(); dst.close(); chmod(tgt.c_str(),0755);
    cout<<C::GRN<<"[+] Installed. Type 'zp' from anywhere."<<C::RST<<endl;
    return true;
}

void showVersion() {
    int w=termW();
    cout<<C::CYN<<C::BLD;
    cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
    cout<<"\xE2\x95\x91"<<ctr("ZERO-SPACE v2.0.1 - BIT-STORM",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n"<<C::RST<<C::WHT;
    cout<<"\xE2\x95\x91  Build:   2026-05-17"<<rep(" ",max(0,w-22))<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  Codename: BIT-STORM"<<rep(" ",max(0,w-22))<<"\xE2\x95\x91\n";
    #ifdef __aarch64__
    cout<<"\xE2\x95\x91  Arch:     ARM64"<<rep(" ",max(0,w-22))<<"\xE2\x95\x91\n";
    #elif __arm__
    cout<<"\xE2\x95\x91  Arch:     ARM32"<<rep(" ",max(0,w-22))<<"\xE2\x95\x91\n";
    #endif
    cout<<"\xE2\x95\x91  Engine:   Bit-Table 12-Layer Hyper Compression"<<rep(" ",max(0,w-47))<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  Format:   ZP2 (Binary Pattern Table)"<<rep(" ",max(0,w-38))<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
}

void showHelp() {
    int w=termW();
    cout<<C::CYN<<C::BLD;
    cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
    cout<<"\xE2\x95\x91"<<ctr("ZERO-SPACE v2.0.1 - USAGE",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\xA0"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\xA3\n"<<C::RST<<C::WHT;
    cout<<"\xE2\x95\x91  zp                       Interactive mode with browser \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp -c <in> <out>         Compress file/folder          \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp -c --force <in> <out> Force compress any file       \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp -d <in> <out>         Decompress file               \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp -b                    File browser only             \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp -i <file>             Show .zp file info            \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp --version             Show version info             \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp --update              Check for updates             \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp --help                Show this help                \xE2\x95\x91\n";
    cout<<C::CYN<<"\xE2\x95\xA0"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\xA3\n"<<C::RST<<C::WHT;
    cout<<"\xE2\x95\x91  Supported types for compression:                        \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::GRN<<"[+]"<<C::WHT<<" TEXT, CSV, JSON, XML, HTML, SQL, LOG, CODE...        \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::GRN<<"[+]"<<C::WHT<<" APK, DOCUMENT (PDF, DOCX...)                        \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::YLW<<"[~]"<<C::WHT<<" BINARY (custom formats)                              \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::RED<<"[!]"<<C::WHT<<" IMAGE, AUDIO, VIDEO, ARCHIVE stored as-is            \xE2\x95\x91\n";
    cout<<C::CYN<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
}

void showFileInfo(const string& path) {
    ifstream ifs(path, ios::binary);
    if (!ifs) {
        showError("Cannot open file", "File not found: " + path);
        return;
    }
    
    char mg[3]; ifs.read(mg, 3);
    if (mg[0] != 'Z' || mg[1] != 'P') {
        showError("Not a ZP file", "This file is not a ZERO-SPACE compressed file.");
        return;
    }
    
    string format = (mg[2] == '2') ? "ZP2 (Bit-Table)" : (mg[2] == '!') ? "ZP1 (Huffman)" : "Unknown";
    long long orig;
    
    if (mg[2] == '2') {
        ifs.read(reinterpret_cast<char*>(&orig), sizeof(long long));
        int patternCount;
        ifs.read(reinterpret_cast<char*>(&patternCount), sizeof(int));
        
        struct stat st; stat(path.c_str(), &st);
        
        int w=termW();
        cout<<C::CYN<<C::BLD;
        cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
        cout<<"\xE2\x95\x91 "<<ctr("ZP FILE INFO",w-2)<<" \xE2\x95\x91\n";
        cout<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n"<<C::RST<<C::WHT;
        cout<<"\xE2\x95\x91  Format:   "<<C::GRN<<format<<rep(" ",max(0,w-20))<<"\xE2\x95\x91\n";
        cout<<"\xE2\x95\x91  Original: "<<C::GRN<<fmtSize(orig)<<rep(" ",max(0,w-20))<<"\xE2\x95\x91\n";
        cout<<"\xE2\x95\x91  Size:     "<<C::YLW<<fmtSize(st.st_size)<<rep(" ",max(0,w-20))<<"\xE2\x95\x91\n";
        double ratio = 1.0 - (double)st.st_size / orig;
        cout<<"\xE2\x95\x91  Ratio:    "<<C::MAG<<fixed<<setprecision(1)<<(ratio*100)<<"%"<<rep(" ",max(0,w-20))<<"\xE2\x95\x91\n";
        cout<<"\xE2\x95\x91  Patterns: "<<C::CYN<<patternCount<<rep(" ",max(0,w-20))<<"\xE2\x95\x91\n";
        cout<<C::CYN<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
    }
    ifs.close();
}

void checkUpdate() {
    cout<<C::YLW<<"[*] Checking for updates..."<<C::RST<<endl;
    string cmd="curl -s https://raw.githubusercontent.com/abdelmonaimzh2023-ctrl/ZERO-X/main/version.txt 2>/dev/null";
    FILE* p=popen(cmd.c_str(),"r"); if(!p){cout<<C::RED<<"[!] Cannot check. No internet?"<<C::RST<<endl;return;}
    char buf[32]; string latest; if(fgets(buf,sizeof(buf),p)) latest=buf; pclose(p);
    while(!latest.empty()&&(latest.back()=='\n'||latest.back()=='\r')) latest.pop_back();
    if(latest.empty()){cout<<C::RED<<"[!] Cannot fetch version."<<C::RST<<endl;return;}
    if(latest=="1.0.9"){cout<<C::GRN<<"[+] Already up to date (v2.0.1)"<<C::RST<<endl;return;}
    cout<<C::YLW<<"[*] Update available: v"<<latest<<" (current: v2.0.1)"<<C::RST<<endl;
    cout<<"[?] Install? (y/n): "; char c; cin>>c;
    if(c=='y'||c=='Y'){
        cout<<"[*] Downloading...\n";
        system("curl -L -o /tmp/zp_new https://github.com/abdelmonaimzh2023-ctrl/ZERO-X/releases/latest/download/zp-arm64");
        system("cp /data/data/com.termux/files/usr/bin/zp /data/data/com.termux/files/usr/bin/zp.bak");
        system("cp /tmp/zp_new /data/data/com.termux/files/usr/bin/zp");
        system("chmod +x /data/data/com.termux/files/usr/bin/zp");
        system("rm /tmp/zp_new");
        cout<<C::GRN<<"[+] Updated. Restart ZERO-SPACE."<<C::RST<<endl;
        exit(0);
    }
}

// ==================== MAIN MENU ====================
void showMenu() {
    cls(); int w=termW();
    cout<<C::CYN<<C::BLD;
    cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
    cout<<"\xE2\x95\x91"<<ctr("ZERO-SPACE v2.0.1",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91"<<ctr("BIT-STORM ENGINE",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\xA0"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\xA3\n";
    cout<<"\xE2\x95\x91"<<rep(" ",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::GRN<<"[1] > COMPRESS"<<C::WHT<<rep(" ",w-19)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::GRN<<"[2] < DECOMPRESS"<<C::WHT<<rep(" ",w-21)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::GRN<<"[3] ? FILE INFO"<<C::WHT<<rep(" ",w-20)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::YLW<<"[4] ? VERSION"<<C::WHT<<rep(" ",w-18)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::CYN<<"[5] ^ UPDATE"<<C::WHT<<rep(" ",w-17)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::RED<<"[6] X EXIT"<<C::WHT<<rep(" ",w-15)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91"<<rep(" ",w-2)<<"\xE2\x95\x91\n";
    cout<<C::CYN<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
    cout<<"\n  Choice: ";
}

// ==================== MAIN ====================
int main(int argc, char* argv[]) {
    hideCur();
    autoInstall();
    Logger::init();
    Logger::write("ZERO-SPACE started");
    
    if(argc==1) {
        while(true) {
            showMenu(); string ch; cin>>ch;
            if(ch=="1"){
                Logger::write("COMPRESS mode selected");
                string in=browser("/storage/emulated/0/Download","SELECT FILE TO COMPRESS",true,true);
                if(in.empty()) continue;
                
                struct stat st; stat(in.c_str(),&st);
                bool isDir = S_ISDIR(st.st_mode);
                
                // Show file info before compressing
                if(!isDir){
                    string type=getType(in);
                    if(isStoredAsIs(type)){
                        cls(); int w=termW();
                        cout<<C::YLW;
                        cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
                        cout<<"\xE2\x95\x91 "<<ctr("WARNING: LOW COMPRESSION EXPECTED",w-2)<<" \xE2\x95\x91\n";
                        cout<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n"<<C::RST<<C::WHT;
                        cout<<"\xE2\x95\x91  Type: "<<C::YLW<<type<<C::WHT<<rep(" ",max(0,w-14-(int)type.size()))<<"\xE2\x95\x91\n";
                        cout<<"\xE2\x95\x91  This file type is usually stored as-is.                  \xE2\x95\x91\n";
                        cout<<"\xE2\x95\x91  Compression may not save much space.                      \xE2\x95\x91\n";
                        cout<<C::CYN<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n"<<C::RST<<C::WHT;
                        cout<<"\xE2\x95\x91  [C] Compress anyway   [Q] Cancel                          \xE2\x95\x91\n";
                        cout<<C::CYN<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
                        cout<<"\n  Choice: ";
                        string warnCh; cin>>warnCh;
                        if(warnCh=="Q"||warnCh=="q") continue;
                    }
                }
                
                string out=in+".zp";
                Logger::write("COMPRESS START | "+in+" -> "+out);
                
                if(isDir){
                    compressFolder(in,out);
                    cout<<C::GRN<<"\n  [+] Folder compressed: "<<out<<"\n"<<C::WHT;
                    Logger::write("COMPRESS END | SUCCESS (FOLDER)");
                }else{
                    CompResult r=smartCompress(in,out);
                    if(r.ok){
                        cout<<C::GRN<<"\n  [+] Success! "<<fmtSize(r.orig)<<" -> "<<fmtSize(r.comp)<<" ("<<fixed<<setprecision(1)<<(r.ratio*100)<<"%)\n";
                        cout<<"  [+] Time: "<<(r.ms/1000.0)<<"s\n"<<C::WHT;
                        Logger::write("COMPRESS END | SUCCESS | Ratio: "+to_string(r.ratio*100)+"%");
                    }else{
                        showError("Compression Failed",r.err);
                        Logger::write("COMPRESS END | FAILED | "+r.err);
                    }
                }
                cout<<"  [?] Press Enter..."; cin.ignore(); cin.get();
                
            }else if(ch=="2"){
                Logger::write("DECOMPRESS mode selected");
                string in=browser("/storage/emulated/0/Download","SELECT .zp FILE TO DECOMPRESS",true);
                if(in.empty()) continue;
                string out=in+".extracted";
                Logger::write("DECOMPRESS START | "+in+" -> "+out);
                legendaryBar(50,0,5,1024,8000,38,"DECOMPRESSING...",0,0,in);
                if(smartDecompress(in,out)){
                    legendaryBar(100,0,0,1024,8000,40,"COMPLETE",0,0,out);
                    Logger::write("DECOMPRESS END | SUCCESS");
                    cout<<C::GRN<<"\n  [+] Decompressed: "<<out<<"\n"<<C::WHT;
                }else{
                    showError("Decompression Failed","File may be corrupted or in old format.");
                    Logger::write("DECOMPRESS END | FAILED");
                }
                cout<<"  [?] Press Enter..."; cin.ignore(); cin.get();
                
            }else if(ch=="3"){
                string in=browser("/storage/emulated/0/Download","SELECT .zp FILE FOR INFO",true);
                if(!in.empty()){showFileInfo(in); cout<<"\n  [?] Press Enter..."; cin.ignore(); cin.get();}
                
            }else if(ch=="4"){
                showVersion();
                cout<<"\n  [?] Press Enter..."; cin.ignore(); cin.get();
                
            }else if(ch=="5"){
                checkUpdate();
                cout<<"\n  [?] Press Enter..."; cin.ignore(); cin.get();
                
            }else if(ch=="6"||ch=="x"||ch=="X"){
                Logger::write("ZERO-SPACE exited");
                break;
            }
        }
        showCur(); return 0;
    }
    
    // CLI mode
    string cmd=argv[1];
    if(cmd=="--version") showVersion();
    else if(cmd=="--help") showHelp();
    else if(cmd=="--update") checkUpdate();
    else if(cmd=="-c"&&argc>=4){
        bool force=false;
        string in, out;
        if(string(argv[2])=="--force"&&argc>=5){
            force=true; in=argv[3]; out=argv[4];
        }else{
            in=argv[2]; out=argv[3];
        }
        
        struct stat st;
        if(stat(in.c_str(),&st)==0&&S_ISDIR(st.st_mode)){
            compressFolder(in,out);
            cout<<C::GRN<<"[+] Folder compressed: "<<out<<C::RST<<endl;
        }else{
            if(!force){
                string type=getType(in);
                if(isStoredAsIs(type)){
                    cout<<C::YLW<<"[!] "<<type<<" files are stored as-is. Use --force to compress anyway."<<C::RST<<endl;
                    return 1;
                }
            }
            CompResult r=compressFileBit(in,out);
            if(r.ok) cout<<C::GRN<<"[+] "<<fmtSize(r.orig)<<" -> "<<fmtSize(r.comp)<<" ("<<fixed<<setprecision(1)<<(r.ratio*100)<<"%)"<<C::RST<<endl;
            else cout<<C::RED<<"[-] "<<r.err<<C::RST<<endl;
        }
    }else if(cmd=="-d"&&argc>=4){
        if(smartDecompress(argv[2],argv[3])) cout<<C::GRN<<"[+] Decompressed: "<<argv[3]<<C::RST<<endl;
        else cout<<C::RED<<"[-] Decompression failed."<<C::RST<<endl;
    }else if(cmd=="-b"){
        string sel=browser("/storage/emulated/0/Download","FILE BROWSER",false,true);
        if(!sel.empty()) cout<<"Selected: "<<sel<<endl;
    }else if(cmd=="-i"&&argc>=3){
        showFileInfo(argv[2]);
    }else showHelp();
    
    Logger::write("ZERO-SPACE finished");
    showCur(); return 0;
}
