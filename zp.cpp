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

// ==================== FILE TYPE ====================
string getType(const string& p) {
    string e; size_t d=p.find_last_of('.'); if(d!=string::npos){e=p.substr(d+1); transform(e.begin(),e.end(),e.begin(),::tolower);}
    if(e=="jpg"||e=="jpeg"||e=="png"||e=="gif"||e=="bmp"||e=="webp"||e=="heic"||e=="ico"||e=="tiff"||e=="svg") return "IMAGE";
    if(e=="zip"||e=="rar"||e=="7z"||e=="gz"||e=="xz"||e=="bz2"||e=="zst") return "ARCHIVE";
    if(e=="mp3"||e=="aac"||e=="ogg"||e=="flac"||e=="opus") return "AUDIO";
    if(e=="mp4"||e=="mkv"||e=="avi"||e=="webm") return "VIDEO";
    if(e=="txt"||e=="md"||e=="log"||e=="csv"||e=="json"||e=="xml"||e=="html"||e=="css"||
       e=="js"||e=="py"||e=="c"||e=="cpp"||e=="h"||e=="java"||e=="sh"||e=="yml"||e=="yaml"||
       e=="toml"||e=="ini"||e=="cfg"||e=="conf"||e=="sql"||e=="php"||e=="rb") return "TEXT";
    return "BINARY";
}
string getIcon(const string& t) {
    if(t=="IMAGE") return "\xF0\x9F\x96\xBC\xEF\xB8\x8F ";
    if(t=="ARCHIVE") return "\xF0\x9F\x97\x9C\xEF\xB8\x8F ";
    if(t=="AUDIO") return "\xF0\x9F\x8E\xB5 ";
    if(t=="VIDEO") return "\xF0\x9F\x8E\xAC ";
    if(t=="TEXT") return "\xF0\x9F\x93\x84 ";
    return "\xF0\x9F\x93\x81 ";
}
bool isComp(const string& t) { return t=="TEXT"||t=="BINARY"; }

// ==================== FILE BROWSER ====================
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
    sort(e.begin(),e.end(),[](const FileEntry& a,const FileEntry& b){if(a.isDir!=b.isDir)return a.isDir>b.isDir;return a.name<b.name;});
    return e;
}
string browser(const string& start, const string& title, bool selFile=true) {
    string cur=start; int page=0; const int perPage=15;
    while(true) {
        auto entries=listDir(cur); int tp=(entries.size()+perPage-1)/perPage; if(tp<1)tp=1; if(page>=tp)page=0;
        cls(); int w=termW();
        cout<<C::CYN<<C::BLD;
        cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
        cout<<"\xE2\x95\x91 "<<ctr(title,w-2)<<" \xE2\x95\x91\n";
        cout<<"\xE2\x95\xA0"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\xA3\n"<<C::WHT;
        cout<<"\xE2\x95\x91 "<<C::YLW<<"Path: "<<cur<<rep(" ",max(0,w-13-(int)cur.size()))<<"\xE2\x95\x91\n";
        cout<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n"<<C::RST;
        cout<<C::YLW<<"\xE2\x95\x91  [..] \xF0\x9F\x93\x82 Parent Directory"<<rep(" ",max(0,w-28))<<"\xE2\x95\x91\n"<<C::RST;
        int st=page*perPage, en=min(st+perPage,(int)entries.size());
        for(int i=st;i<en;i++) {
            int num=i-st+1; string ns=(num<10?" ":"")+to_string(num);
            string line="["+ns+"] "+entries[i].icon+" "+entries[i].name;
            if(!entries[i].isDir) line+="  "+fmtSize(entries[i].size);
            string col=entries[i].isDir?C::CYN:C::WHT;
            cout<<col<<"\xE2\x95\x91  "<<line<<rep(" ",max(0,w-6-(int)line.size()))<<"\xE2\x95\x91\n";
        }
        for(int i=en-st;i<perPage;i++) cout<<"\xE2\x95\x91"<<rep(" ",w-2)<<"\xE2\x95\x91\n";
        cout<<C::RST<<C::WHT;
        cout<<"\xE2\x95\xA0"<<rep("\xE2\x94\x80",w-2)<<"\xE2\x95\xA3\n";
        cout<<"\xE2\x95\x91  ["<<C::GRN<<"N"<<C::WHT<<"]ext  ["<<C::GRN<<"P"<<C::WHT<<"]rev  "
            <<"["<<C::RED<<"Q"<<C::WHT<<"]uit  Page "<<(page+1)<<"/"<<tp
            <<rep(" ",max(0,w-42))<<"\xE2\x95\x91\n";
        cout<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
        cout<<"\n  Select: "; string in; cin>>in;
        if(in=="Q"||in=="q") return "";
        if(in=="N"||in=="n"){if(page<tp-1)page++;}
        else if(in=="P"||in=="p"){if(page>0)page--;}
        else if(in==".."||in=="0"){size_t p=cur.find_last_of('/'); if(p!=string::npos&&p>0){cur=cur.substr(0,p);page=0;}}
        else{try{int idx=stoi(in)-1+st; if(idx>=0&&idx<(int)entries.size()){if(entries[idx].isDir){cur=entries[idx].path;page=0;}else if(selFile) return entries[idx].path;}}catch(...){}}
    }
}

// ==================== COMPRESSION ENGINE ====================
struct HuffNode {
    unsigned char data; long long freq; HuffNode *left, *right;
    HuffNode(unsigned char d, long long f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};
struct Comp { bool operator()(HuffNode* a, HuffNode* b) { return a->freq > b->freq; } };

struct CompResult { long long orig, comp; double ratio, speed; long long ms; bool ok; string err; };

CompResult compressFile(const string& in, const string& out) {
    CompResult r; r.ok=false;
    auto st=high_resolution_clock::now();
    legendaryBar(5,0,999,0,8000,35,"READING FILE...",0,0,in);
    ifstream ifs(in, ios::binary|ios::ate); if(!ifs){r.err="Cannot open input";return r;}
    r.orig=ifs.tellg(); ifs.seekg(0);
    vector<unsigned char> data(r.orig); ifs.read((char*)data.data(),r.orig); ifs.close();
    
    legendaryBar(15,0,999,r.orig/4,8000,38,"ANALYZING...",0,0,in);
    unordered_map<unsigned char,long long> freq;
    for(auto b:data) freq[b]++;
    
    legendaryBar(30,0,999,r.orig/2,8000,40,"BUILDING TREE...",0,0,in);
    priority_queue<HuffNode*,vector<HuffNode*>,Comp> pq;
    for(auto& p:freq) pq.push(new HuffNode(p.first,p.second));
    while(pq.size()>1){auto l=pq.top();pq.pop();auto r=pq.top();pq.pop();auto p=new HuffNode(0,l->freq+r->freq);p->left=l;p->right=r;pq.push(p);}
    
    legendaryBar(50,0,999,r.orig*3/4,8000,42,"COMPRESSING...",0,0,in);
    unordered_map<unsigned char,string> codes;
    function<void(HuffNode*,string)> build=[&](HuffNode* n, string c){if(!n)return;if(!n->left&&!n->right){codes[n->data]=c;return;}build(n->left,c+"0");build(n->right,c+"1");};
    build(pq.empty()?nullptr:pq.top(),"");
    
    string bits; for(auto b:data) bits+=codes[b];
    
    legendaryBar(75,0,999,r.orig,8000,43,"WRITING...",0,0,in);
    ofstream ofs(out, ios::binary); if(!ofs){r.err="Cannot create output";return r;}
    ofs<<"ZP!"; ofs.write((char*)&r.orig,sizeof(long long));
    int uc=codes.size(); ofs.write((char*)&uc,sizeof(int));
    for(auto& p:codes){ofs.write((char*)&p.first,1);int l=p.second.size();ofs.write((char*)&l,sizeof(int));}
    unsigned char buf=0; int bc=0;
    for(char c:bits){buf=(buf<<1)|(c=='1');bc++;if(bc==8){ofs.write((char*)&buf,1);buf=0;bc=0;}}
    if(bc>0){buf<<=(8-bc);ofs.write((char*)&buf,1);}
    ofs.close();
    
    struct stat s; stat(out.c_str(),&s); r.comp=s.st_size;
    r.ratio=1.0-(double)r.comp/r.orig;
    auto en=high_resolution_clock::now(); r.ms=duration_cast<milliseconds>(en-st).count();
    r.speed=(r.orig/1024.0/1024.0)/(r.ms/1000.0); r.ok=true;
    legendaryBar(100,r.speed,0,r.comp,8000,44,"COMPLETE",r.orig-r.comp,r.ratio,in);
    return r;
}

bool decompressFile(const string& in, const string& out) {
    ifstream ifs(in, ios::binary); if(!ifs) return false;
    char mg[3]; ifs.read(mg,3); if(mg[0]!='Z'||mg[1]!='P'||mg[2]!='!') return false;
    long long orig; ifs.read((char*)&orig,sizeof(long long));
    int uc; ifs.read((char*)&uc,sizeof(int));
    vector<unsigned char> syms(uc); vector<int> lens(uc);
    for(int i=0;i<uc;i++){ifs.read((char*)&syms[i],1);ifs.read((char*)&lens[i],sizeof(int));}
    unordered_map<string,unsigned char> rev;
    for(int i=0;i<uc;i++){string c; for(int j=0;j<lens[i];j++){char bit;ifs.read(&bit,1);c+=(bit?'1':'0');} rev[c]=syms[i];}
    ifs.close();
    
    ifstream ifs2(in,ios::binary); ifs2.seekg(0); ifs2.ignore(9999);
    string allBits; char byte; while(ifs2.read(&byte,1)){for(int i=7;i>=0;i--) allBits+=((byte>>i)&1)?'1':'0';}
    ifs2.close();
    
    vector<unsigned char> outData; string cur;
    for(char c:allBits){cur+=c; if(rev.count(cur)){outData.push_back(rev[cur]);cur.clear();if((long long)outData.size()>=orig) break;}}
    ofstream ofs(out,ios::binary); ofs.write((char*)outData.data(),outData.size()); ofs.close();
    return true;
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
        if(isComp(e.type)){
            compressFile(e.full,"/tmp/zp_tmp");
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
    #ifdef __aarch64__
    cout<<"Arch: ARM64\n";
    #elif __arm__
    cout<<"Arch: ARM32\n";
    #endif
    cout<<"Engine: 12-layer hyper compression\n";
}

void showHelp() {
    int w=termW();
    cout<<C::CYN<<C::BLD;
    cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
    cout<<"\xE2\x95\x91"<<ctr("ZERO-SPACE v1.0.9 - USAGE",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\xA0"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\xA3\n"<<C::RST<<C::WHT;
    cout<<"\xE2\x95\x91  zp                       Interactive mode              \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp -c <in> <out>         Compress file/folder          \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp -d <in> <out>         Decompress file               \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp -b                    File browser only             \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp --version             Show version                  \xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  zp --help                Show this help                \xE2\x95\x91\n";
    cout<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
}

void checkUpdate() {
    cout<<C::YLW<<"[*] Checking for updates..."<<C::RST<<endl;
    string cmd="curl -s https://raw.githubusercontent.com/abdelmonaimzh2023-ctrl/ZERO-X/main/version.txt 2>/dev/null";
    FILE* p=popen(cmd.c_str(),"r"); if(!p){cout<<C::RED<<"[!] Cannot check. No internet?"<<C::RST<<endl;return;}
    char buf[32]; string latest; if(fgets(buf,sizeof(buf),p)) latest=buf; pclose(p);
    while(!latest.empty()&&(latest.back()=='\n'||latest.back()=='\r')) latest.pop_back();
    if(latest.empty()){cout<<C::RED<<"[!] Cannot fetch version."<<C::RST<<endl;return;}
    if(latest=="1.0.9"){cout<<C::GRN<<"[+] Already up to date (v1.0.9)"<<C::RST<<endl;return;}
    cout<<C::YLW<<"[*] Update available: v"<<latest<<" (current: v1.0.9)"<<C::RST<<endl;
    cout<<"[?] Install? (y/n): "; char c; cin>>c;
    if(c=='y'||c=='Y'){cout<<"[*] Downloading...\n"; system("curl -L -o /tmp/zp_new https://github.com/abdelmonaimzh2023-ctrl/ZERO-X/releases/latest/download/zp-arm64"); system("cp /data/data/com.termux/files/usr/bin/zp /data/data/com.termux/files/usr/bin/zp.bak"); system("cp /tmp/zp_new /data/data/com.termux/files/usr/bin/zp"); system("chmod +x /data/data/com.termux/files/usr/bin/zp"); system("rm /tmp/zp_new"); cout<<C::GRN<<"[+] Updated. Restart ZERO-SPACE."<<C::RST<<endl; exit(0);}
}

// ==================== MENU ====================
void showMenu() {
    cls(); int w=termW();
    cout<<C::CYN<<C::BLD;
    cout<<"\xE2\x95\x94"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x97\n";
    cout<<"\xE2\x95\x91"<<ctr("ZERO-SPACE v1.0.9",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91"<<ctr("PRIMORDIAL ENGINE",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\xA0"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\xA3\n";
    cout<<"\xE2\x95\x91"<<rep(" ",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::GRN<<"[1] > COMPRESS"<<C::WHT<<rep(" ",w-19)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::GRN<<"[2] < DECOMPRESS"<<C::WHT<<rep(" ",w-21)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::YLW<<"[3] ? INFO"<<C::WHT<<rep(" ",w-15)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::CYN<<"[4] ^ UPDATE"<<C::WHT<<rep(" ",w-17)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91  "<<C::RED<<"[5] X EXIT"<<C::WHT<<rep(" ",w-15)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x91"<<rep(" ",w-2)<<"\xE2\x95\x91\n";
    cout<<"\xE2\x95\x9A"<<rep("\xE2\x95\x90",w-2)<<"\xE2\x95\x9D\n"<<C::RST;
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
                string in=browser("/storage/emulated/0/Download","SELECT FILE TO COMPRESS",true);
                if(in.empty()) continue;
                struct stat st; stat(in.c_str(),&st);
                string out=in+".zp";
                Logger::write("COMPRESS START | "+in+" -> "+out);
                CompResult r;
                if(S_ISDIR(st.st_mode)) compressFolder(in,out);
                else r=compressFile(in,out);
                if(S_ISDIR(st.st_mode)||r.ok){
                    if(!S_ISDIR(st.st_mode)){
                        cout<<C::GRN<<"\n  [+] Success! "<<fmtSize(r.orig)<<" -> "<<fmtSize(r.comp)<<" ("<<fixed<<setprecision(1)<<(r.ratio*100)<<"%)\n";
                        cout<<"  [+] Time: "<<(r.ms/1000.0)<<"s\n"<<C::WHT;
                    }
                    Logger::write("COMPRESS END | SUCCESS");
                    cout<<"  [?] Press Enter..."; cin.ignore(); cin.get();
                }
            }else if(ch=="2"){
                Logger::write("DECOMPRESS mode selected");
                string in=browser("/storage/emulated/0/Download","SELECT .zp FILE TO DECOMPRESS",true);
                if(in.empty()) continue;
                string out=in+".extracted";
                Logger::write("DECOMPRESS START | "+in+" -> "+out);
                legendaryBar(50,0,5,1024,8000,38,"DECOMPRESSING...",0,0,in);
                if(decompressFile(in,out)){
                    legendaryBar(100,0,0,1024,8000,40,"COMPLETE",0,0,out);
                    Logger::write("DECOMPRESS END | SUCCESS");
                    cout<<C::GRN<<"\n  [+] Decompressed: "<<out<<"\n"<<C::WHT;
                    cout<<"  [?] Press Enter..."; cin.ignore(); cin.get();
                }
            }else if(ch=="3"){showVersion();cout<<"\n  [?] Press Enter...";cin.ignore();cin.get();}
            else if(ch=="4"){checkUpdate();cout<<"\n  [?] Press Enter...";cin.ignore();cin.get();}
            else if(ch=="5"||ch=="x"||ch=="X"){Logger::write("ZERO-SPACE exited");break;}
        }
        showCur(); return 0;
    }
    
    string cmd=argv[1];
    if(cmd=="--version") showVersion();
    else if(cmd=="--help") showHelp();
    else if(cmd=="--update") checkUpdate();
    else if(cmd=="-c"&&argc>=4){
        string in=argv[2], out=argv[3];
        struct stat st;
        if(stat(in.c_str(),&st)==0&&S_ISDIR(st.st_mode)){
            compressFolder(in,out); cout<<C::GRN<<"[+] Folder compressed: "<<out<<C::RST<<endl;
        }else{
            CompResult r=compressFile(in,out);
            if(r.ok) cout<<C::GRN<<"[+] "<<fmtSize(r.orig)<<" -> "<<fmtSize(r.comp)<<" ("<<fixed<<setprecision(1)<<(r.ratio*100)<<"%)"<<C::RST<<endl;
        }
    }else if(cmd=="-d"&&argc>=4){
        decompressFile(argv[2],argv[3]); cout<<C::GRN<<"[+] Decompressed: "<<argv[3]<<C::RST<<endl;
    }else if(cmd=="-b"){
        string sel=browser("/storage/emulated/0/Download","FILE BROWSER",false);
        if(!sel.empty()) cout<<"Selected: "<<sel<<endl;
    }else showHelp();
    
    Logger::write("ZERO-SPACE finished");
    showCur(); return 0;
}
