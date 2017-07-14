/*
 client.cc
 Katsuki Ohto
 */

// デジタルカーリングメイン

#include "dc.hpp"
#include "ayumu/ayumu_dc.hpp"

#include "structure/field.hpp"

using namespace DigitalCurling;

constexpr unsigned short DEFAULT_PORT_NUM = 9876;

#ifdef _WIN32

#else

#define Sleep(a) sleep(a)
#define SOCKET_ERROR 1

typedef int SOCKET;
typedef unsigned long DWORD;
typedef pthread_t HANDLE;

#define TRUE true
#define FALSE false

#endif

// 引数による要求
namespace Requirement{
    constexpr int TEST_GAME = 30;
    constexpr int PREPROCCESS = 31;
}

class AdminInfo{
private:
    void close(){
        // 管理用ファイルを消す
        if (id_num >= 0){ // id_numが設定されている
            char str_file[256];
            snprintf(str_file, 256, std::string(DIRECTORY_ID + "%d.txt").c_str(), id_num);
            if (remove(str_file) >= 0){
                CERR << "succeeded to remove Admin file." << endl;
            }
            else{
                CERR << "failed to remove Admin file." << endl;
            }
        }
    }
public:
    
    int id_num;
    
    AdminInfo() :
    id_num(-1){}
    
    ~AdminInfo(){
        close();
    }
};

AdminInfo gadm;
ClientField myField;

#include "ayumu/ayumu.hpp"

int decideID(){
    // 自分が複数ログインするときのために、
    // 自分の識別番号を決める
    // TODO: 排他制御がなされていない
    FILE *pf;
    char str_file[256];
    for (int i = 0; i < 999; ++i){
        snprintf(str_file, 256, std::string(DIRECTORY_ID + "%d.txt").c_str(), i);
        pf = fopen(str_file, "r");
        if (pf == NULL) {
            gadm.id_num = i;
            break;
        }
        int iret = fclose(pf);
        if (iret < 0) { return -1; }
    }
    // 最後に新しい番号のファイルを作る
    snprintf(str_file, 256, std::string(DIRECTORY_ID + "%d.txt").c_str(), gadm.id_num);
    pf = fopen(str_file, "w");
    if (pf == NULL) { return -1; }
    
    return 0;
}

// グローバル変数
AyumuAI client;
#if !defined(ENGINE)
SOCKET sock;
#endif
int gNLimitGames; // 試合数限度

int sendMessage(const std::string& msg){
    const char *const buf = msg.c_str();
    if (strlen(buf) > 0){
        CERR << "Client << " << buf << endl;
    }
#if defined(ENGINE) && defined(_WIN32)
    // 公式クライアントのエンジンの場合
    DWORD bytes;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, strlen(buf), &bytes, NULL);
#elif defined(ENGINE)
    // 標準入出力型のエンジンの場合
    std::cout << msg << endl;
#else
    // 自力でサーバーと通信する場合
    send(sock, buf, strlen(buf) + 1, 0);
#endif
    return 0;
}

std::string toUpper(const std::string& str){ // 文字列を大文字に
    std::string ret = "";
    for (int i = 0; i < str.size(); ++i){
        ret.push_back(toupper(str[i]));
    }
    return ret;
}

void searchToken(const char* str, const int size, int *const ret){
    for (int c = *ret; c < size - 1; ++c){
        if (str[c] == '\0'){
            if (str[c + 1] == '\0'){
                *ret = -1; return;
            }
            else{
                *ret = c + 1; return;
            }
        }
    }
    *ret = -1;
    return;
}

int recvMessage(){
    
    CERR << "recvMessage() : Opened Receive Thread." << endl;
    
    //受信スレッド
    gState = 0;
    auto& myField = client.field;
    
    int games = 0;
    int inverseGame = -1;
    
    // 通信
    int L = 8192;
    char *rbuffer = new char[L];
    memset(rbuffer, 0, L);
    
    while (1){
        /*if(strlen(rbuffer) <= 0){
#if defined(_WIN32) && defined(ENGINE)
            // 公式クライアントのエンジンの場合
            DWORD bytes;
            ReadFile(GetStdHandle(STD_INPUT_HANDLE), rbuffer, L, &bytes, NULL);
#else
            // TCP/IPでサーバーと通信する場合
            int bytes = recv(sock, rbuffer, L, 0);
#endif
            CERR << bytes << endl;
            if(bytes == 0){
                CERR << "disconnected..." << endl;
                goto END_ABNORMAL;
            }else{
                while(bytes == L){ // 満杯
                    L = 2 * L;
                    char *nrbuffer = new char[L];
                    memset(nrbuffer, 0, L);
                    memcpy(nrbuffer, rbuffer, bytes);
                    delete[] rbuffer;
                    rbuffer = nrbuffer;
#if defined(_WIN32) && defined(ENGINE)
                    // 公式クライアントのエンジンの場合
                    DWORD tbytes;
                    ReadFile(GetStdHandle(STD_INPUT_HANDLE), rbuffer, L - bytes, &tbytes, NULL);
                    bytes += tbytes;
#else
                    // TCP/IPでサーバーと通信する場合
                    bytes += recv(sock, rbuffer + bytes, L - bytes, 0);
#endif
                }
            }
        }*/
        
        char *tok;
        int c;
        
        memset(rbuffer, 0, L);
        
#if defined(_WIN32) && defined(ENGINE)
        // 公式クライアントのエンジンの場合
        DWORD NBytesRead;
        ReadFile(GetStdHandle(STD_INPUT_HANDLE), rbuffer, L, &NBytesRead, NULL);
#elif defined(ENGINE)
        // 標準入出力型のエンジンの場合
        std::string tmp;
        std::getline(std::cin, tmp);
        int NBytesRead = snprintf(rbuffer, L, tmp.c_str());
#else
        // 自力でサーバーと通信する場合
        int NBytesRead = recv(sock, rbuffer, L, 0);
        if (NBytesRead < 0){ return -1; }
#endif
        if(NBytesRead == 0){
            CERR << "disconnected..." << endl;
            goto END_ABNORMAL;
        }
        
        c = 0;
        
        /*std::string str = std::string(rbuffer);
        int len = strlen(rbuffer);
        DERR << len << endl;
        memmove(rbuffer, rbuffer + len + 1, L - len - 1);
        memset(rbuffer + L - len - 1, 0, len + 1);

        std::vector<std::string> msgs;
        msgs.push_back(str);
        
        for(const auto& msg : msgs){*/
        
        while (c >= 0){
            
            tok = &rbuffer[c];
            
            std::string msg = std::string(tok);
            
            CERR << "Server >> " << msg << endl;
            
            std::vector<std::string> args = split(msg, ' ');
            
            std::string command = toUpper(args[0]); // 大文字にする
            
            if (command == "CONNECTED"){
            }
            else if (command == "LOGIN"){
                if(args.size() >= 2 && toUpper(args[1]) == "NG"){
                    goto END_ABNORMAL;
                }
            }
            else if (command == "ISREADY"){
                client.ready();
                sendMessage("READYOK");
            }
            else if (command == "NEWGAME"){
                //　プレーヤー名が送られてくるので記録
                std::string name[2];
                if(args.size() >= 2){
                    name[0] = args[1];
                }else{
                    name[0] = "First";
                }
                if(args.size() >= 3){
                    name[1] = args[2];
                }else{
                    name[1] = "Second";
                }
                client.setName(name[0], name[1]);
                inverseGame = -1;
            }
            else if (command == "POSITION"){
                // ストーンの位置については毎回初期化
                for (int i = 0; i < 16; ++i){
                    float x = (float)atof(args[1 + 2 * i].c_str());
                    float y = (float)atof(args[2 + 2 * i].c_str());
                    myField.setStonePos(i, x, y);
                }
            }
            else if (command == "SETSTATE"){
                
#if defined(CREATE_PONDER_THREAD)
                if (gState.isThinking()){
                    gState.requestToStop(); // 思考終了命令を出す
                    while (gState.isThinking()){}
                }
#endif
                // 状態を設定
                myField.setTurn(atoi(args[1].c_str()));
                myField.setEnd(atoi(args[2].c_str()));
                myField.setToEnd(atoi(args[3].c_str()));
                int tc = atoi(args[4].c_str());
                
                // 試合の第1投で第4引数が0か1かを見ることで、得点表示が逆になるか調べる
                if(inverseGame < 0){
                    inverseGame = (tc == 0) ? 0 : 1;
                }
                
                if (myField.my_first_color >= 0 && !myField.isMyTurn()){
                    // 自分のターンでない場合は、ここからRUNSHOTが来るまで相手手番中の思考に入る
                    // まだ自分の先後がわからない場合は何もしない
#if defined(CREATE_PONDER_THREAD)
                    gState.requestToPonder();
#ifdef DO_PONDERING
                    thread think(thinkThread);
                    think.detach();
#else
                    thinkThread();
#endif
#endif
                }
            }
            else if (command == "GO"){
                // 手を決めて送る
                
                if (myField.my_first_color < 0){
                    // ここで自分の先後がわかる
                    if (myField.getTurnColor() == 0){
                        myField.setMyFirstColor(0);
                    }
                    else{
                        myField.setMyFirstColor(1);
                    }
                }
                assert(myField.isMyTurn());
                
                // まず制限時間の確認
                for (int c = 0; c < 2; ++c){
                    uint64_t tl = (uint64_t)atoi(args[1 + c].c_str());
                    if (tl == 0ULL){ // これが無制限の意味
                        myField.setTimeLimit(c, TIME_LIMIT_UNLIMITED);
                    }
                    else{
                        myField.setTimeLimit(c, tl);
                    }
                }
                
#if defined(CREATE_PONDER_THREAD)
                if (gState.isThinking()){
                    // 先読み中の場合は、思考終了命令を出す
                    gState.requestToStop();
                    while (gState.isThinking()){}
                }
                
                gState.requestToDecideMove();
                
#ifdef DO_PONDERING
                thread think(thinkThread);
                think.detach();
#else
                thinkThread();
#endif
                
#else
                gState = 0;
                
                CERR << myField.toBroadcastString();
                fMoveXY<> fmv;
                int ret = client.play(&fmv);
                
                if(ret == -1){//投了
                    sendMessage("CONCEDE");
                }else{
                    std::ostringstream oss;
                    oss << "BESTSHOT " << (float)fmv.vx() << " " << (float)fmv.vy() << " " << (int)fmv.getSpin();
                    sendMessage(oss.str());
                }
#endif
                client.prepare();
            }
            else if (command == "TIMEOUT"){
                // タイムアウト
            }
            else if (command == "RUNSHOT"){
                // ショット結果
            }
            else if (command == "TOTALSCORE"){
                // 総合結果
            }
            else if (command == "SCORE"){
                // エンド結果
                int sc = atoi(args[1].c_str());
                // 得点表示が逆になる試合の場合には点を逆にする
                if(inverseGame == 1){ sc = -sc; }
                
                myField.setEndScore(sc);
                client.closeEnd();
                client.initEnd(); // 次のエンドの初期化までやっておく
            }
            else if (command == "GAMEOVER"){
                int r = 0;
                if (toUpper(args[1]) == "WIN"){
                    r = 1;
                }
                else if (toUpper(args[1]) == "LOSE"){
                    r = -1;
                }
                client.closeGame(r);
                ++games;
                
#if !defined(ENGINE)
                if (games >= gNLimitGames){
                    // クライアントの限界試合数に達した場合は勝手に終了する(エンジンモードでないとき)
                    goto END_NORMAL;
                }
#endif // !ENGINE
            }
            else if (command == "DISCONNECT"){
                goto END_ABNORMAL;
            }
            // 以下、歩で拡張したコマンド
            else if (command == "CONCEDE"){
                // 投了
#if defined(CREATE_PONDER_THREAD)
                if (gState.isThinking()){
                    gState.requestToStop(); // 思考終了命令を出す
                }
#endif
            }else if(command == "SEED"){
                // 乱数系列の初期化
                uint64_t seedNum = (args.size() > 1) ? atoi(args[1].c_str()) : 0;
                client.setRandomSeed(seedNum);
            }
            else{ // 知らないコマンド
                Sleep(1);
            }
            
            searchToken(rbuffer, NBytesRead, &c);
        }
        
    }
END_NORMAL:
#if defined(CREATE_PONDER_THREAD)
    gState.requestToExit();
#endif
    client.closeAll();
    return 0;
END_ABNORMAL:
#if defined(CREATE_PONDER_THREAD)
    gState.requestToExit();
#endif
    client.closeAll();
    delete[] rbuffer;
    return -1;
}

int main(int argc, char* argv[]){
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    //std::cerr << std::setprecision(8);
    
    //initialize global valiables
    gNLimitGames = 65536;
    gRequirements.reset();
    
    std::cerr << "Opened Client." << endl;
    
    unsigned short port_num = DEFAULT_PORT_NUM;
    
    bool seedChange = false;
    uint64_t seedNum = 0;
    std::string logInID = "", name = "";
    int settledPlayouts = 200000;//-1;
    
    //read arguments
    for (int i = 1; i < argc; ++i){
        
        if(!strcmp(argv[i], "-g")){ // the number of games
            gNLimitGames = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-pre")){ // preprocessing
            gRequirements.set(Requirement::PREPROCCESS);
        }else if(!strcmp(argv[i], "-p")){ // port number
            port_num = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-s")){ // random seed
            seedChange = true;
            seedNum = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-tb")){ // subjective time limit(byoyomi)
            uint64_t tl = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-t")){ // subjective time limit
            uint64_t tl = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-id")){ // log in ID (forced)
            logInID = name = std::string(argv[i + 1]);
        }else if(!strcmp(argv[i], "-npo")){ // the number of playouts
            settledPlayouts = atoi(argv[i + 1]);
        }
    }
    
    // ID設定
    // VERSIONモードでは、バージョンをIDに入れる
#ifdef VERSION
    if(!logInID.size()){
        name = MY_NAME + MY_VERSION;
        logInID = MY_ID + MY_VERSION;
    }
#else
    if(!logInID.size()){ // 名前を指定されてい無い時
        // 複数ログイン時にIDを分ける
        if (decideID() != 0){
            cerr << "failed to get my ID number." << endl;
            exit(1);
        }
        std::ostringstream oss;
        oss << MY_NAME << gadm.id_num;
        
        name = oss.str();
        
        std::ostringstream oss1;
        oss1 << MY_ID << gadm.id_num;
        
        logInID = oss1.str();
    }
    
#endif // VERSION
    
    const std::string password = MY_PASSWORD;
    
    const std::string ip = SERVER_IP;
    const int port = port_num;
    
    CERR << "I'm " << name << endl;
    
    // 試合のための初期化処理
    // 接続の前にやらないと、サーバーを立ち上げないと何も出来ない
    if (client.initAll() < 0){ // ここで終了の時
        return 0;
    }
    
    // 乱数シード固定の時
    if(seedChange){
        client.setRandomSeed(seedNum);
    }
    if(settledPlayouts >= 0){ // プレイアウト回数固定の時
        client.settlePlayouts(settledPlayouts);
    }
    
#if !defined(ENGINE)
    
#ifdef _WIN32
    // Windows 独自の設定
    WSADATA data;
    if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 0), &data)){
        cerr << "failed to initialize WSA-data." << endl;
        exit(1);
    }
#endif // _WIN32
    
    //open socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        cerr << "failed to open socket." << endl;
    }
    
    // sockaddr_in 構造体のセット
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    // 接続
    if (SOCKET_ERROR == connect(sock, (struct sockaddr*)&addr, sizeof(addr))){
        cerr << "failed to connect to server." << endl;
        exit(1);//ソケット接続失敗
    }
    
    // 接続結果受け取り
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    int q = recv(sock, buf, sizeof(buf), 0);
    CERR << q << " " << buf << endl;
    
    if (!std::strstr(buf, "CONNECTED")){
        cerr << "failed to make logging-in connection" << endl;
        exit(1);
    }
    // 接続完了
    CERR << "connected to server." << endl;
    
    // ログイン
    sendMessage("LOGIN " + logInID + " " + password + " " + name);
    
#endif // !ENGINE
    
    // 通信
    recvMessage();
    
#ifndef ENGINE
    // 終了
    sendMessage("LOGOUT");
    CERR << "logeed out." << endl;
    // Close socket.
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif // _WIN32
    CERR << "closed socket." << std::endl;
#endif // !ENGINE
    
    return 0;
}