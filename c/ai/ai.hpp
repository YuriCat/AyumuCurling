// Digital Curling
// base class of AI

#include "dc.hpp"

#include "lib.hpp"

#include "simulation/simulator.hpp"
#include "simulation/fastsimulator.hpp"
#include "simulation/rand.hpp"
#include "tools/field.hpp"

using namespace DigitalCurling;

namespace DigitalCurling{
    
    class AI{
    private:
        ClientField field_;
        NetworkInfo info_;
        
    public:
        
        //ai function
        virtual int initAll();
        virtual int initGame();
        virtual int initEnd();
        virtual int closeAll();
        virtual int closeGame();
        virtual int closeEnd();
        
        virtual int play(fMoveXY *const);
        virtual int ponder();
        virtual int prepareAfterPlay();
        
        //procedure
        int connect(){
            
            //open socket
            NetworkInfo.dstSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (NetworkInfo.dstSocket<0){
                cerr << "failed to open socket." << endl;
            }
            
            //sockaddr_in 構造体のセット
            memset(&NetworkInfo.dstAddr, 0, sizeof(NetworkInfo.dstAddr));
            NetworkInfo.dstAddr.sin_port = htons(NetworkInfo.PORT);
            NetworkInfo.dstAddr.sin_family = AF_INET;
            NetworkInfo.dstAddr.sin_addr.s_addr = inet_addr(NetworkInfo.IP);
            
            //接続
            if (SOCKET_ERROR == connect(NetworkInfo.dstSocket, (struct sockaddr *) &NetworkInfo.dstAddr, sizeof(NetworkInfo.dstAddr))){
                cerr << "failed to connect to server." << endl;
                return -1;//ソケット接続失敗
            }
            
            //接続結果受け取り
            int q = recv(NetworkInfo.dstSocket, cmd_recv, sizeof(cmd_recv), 0);
            CERR << q << " " << cmd_recv << endl;
            
            if (!std::strstr(cmd_recv, "CONNECTED")){
                cerr << "failed to make logging-in connection" << endl;
                return -1;
            }
            
            //接続完了
            NetworkInfo.IsConnected = TRUE;
            CERR << "connected to server." << endl;
            return 0;
        }
        
        int commandLoop(){
            
        }
    };
}


const unsigned short PORT_NUM = 9876;

class AdminInfo{
private:
    void close(){
        //管理用ファイルを消す
        if (id_num >= 0){//id_numが設定されている
            char str_file[256];
            snprintf(str_file, 256, string(DIRECTORY_ID+"%d.txt").c_str(), id_num);
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

enum{
    FLAG_PONDER=1,
    FLAG_PLAY=2,
    FLAG_STOP=4,
};

#if defined(CREATE_PONDER_THREAD)


struct ClientState{
    std::atomic<uint32_t> st;
    
    uint32_t isPondering()const{ return st & 2U; }
    uint32_t isDecidingMove()const{ return st & 1U; }
    uint32_t requestedToStop()const{ return st & 4U; }
    uint32_t requestedToExit()const{ return st & 32U; }
    uint32_t requestedToPonder()const{ return st & 16U; }
    uint32_t requestedToDecideMove()const{ return st & 8U; }
    
    uint32_t isThinking()const{ return st & 3U; }
    
    void setPondering(){
        st |= 2U;
    }
    void setDecidingMove(){
        st |= 1U;
    }
    void requestToStop(){
        st |= 4U;
    }
    void requestToPonder(){
        st = (st & (~(4U | 32U))) | 16U;
    }
    void requestToDecideMove(){
        st = (st & (~(4U | 32U))) | 8U;
    }
    void requestToExit(){
        st |= 4U|32U;
    }
    
    void resetRequestToPonder(){
        st &= (~16U);
    }
    void resetRequestToDecideMove(){
        st &= (~8U);
    }
    
    void resetPlay(){
        st &= (4U | 32U);
    }
    
    void init(){
        st = 0;
    }
};

ClientState gState;
#else

std::atomic<uint32_t> gState;

#endif

std::bitset<32> gRequirements;//クライアントに対しての要求

#include "ayumu/ayumu.hpp"

int decideID(){
    //自分が複数ログインするときのために、
    //自分の識別番号を決める
    
    int i;
    FILE *pf;
    char str_file[256];
    //ログの番号を決める
    for (i = 0; i < 999; i++)
    {
        snprintf(str_file, 256, string(DIRECTORY_ID+"%d.txt").c_str(), i);
        pf = fopen(str_file, "r");
        if (pf == NULL) { break; }
        int iret = fclose(pf);
        if (iret < 0) { return -1; }
    }
    gadm.id_num = i;
    //最後に新しい番号のファイルを作る
    snprintf(str_file, 256, string(DIRECTORY_ID+"%d.txt").c_str(), i);
    pf = fopen(str_file, "w");
    if (pf == NULL) { return -1; }
    
    return 0;
}

//グローバル変数
Client client;
NETWORKINFO NetworkInfo;

std::string gmy_id;
int gNLimitGames;//試合数限度


void searchToken(const char* str, const int size, int *ret){
    for (int c = *ret; c<size - 1; ++c){
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

int sendCommand(const char* cmd_send){
    if (strlen(cmd_send)>0){
        CERR << "Client << " << cmd_send << endl;
    }
#if defined(_WIN32) && defined(FOR_ENGINE)
    DWORD NBytesWritten;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), cmd_send, strlen(cmd_send), &NBytesWritten, NULL);
#else
    send(NetworkInfo.dstSocket, cmd_send, strlen(cmd_send) + 1, 0);
#endif
    return 0;
}

#if defined(CREATE_PONDER_THREAD)
int thinkThread(){
    //要求された思考が終了したら抜ける
    uint32_t tmp=gState;
    while (1){
        if (tmp & FLAG_REQUEST_EXIT){
            gState.init(); return 0;
        }
        else if (tmp & FLAG_REQUEST_STOP){
            gState.init(); return 0;
        }
        else if (tmp.requestedToDecideMove()){
            CERR << "decidingMove" << endl;
            gState.setDecidingMove();
            gState.resetRequestToDecideMove();
            myField.broadcast();
            fMoveXY<> mv;
            int ret = client.play(myField, &mv);
            
            char cmd_send[BUFFER_SIZE] = { 0 };
            if (ret == -1){//投了
                snprintf(cmd_send, sizeof(cmd_send), "CONCEDE");
            }
            else{
                snprintf(cmd_send, sizeof(cmd_send), "BESTSHOT %f %f %d", (float)mv.vx(), (float)mv.vy(), (int)mv.getSpin());
            }
            sendCommand(cmd_send);
            gState.resetPlay();
        }
        else if (tmp.requestedToPonder()){
            CERR << "pondering" << endl;
            gState.setPondering();
            gState.resetRequestToPonder();
            client.prepare(myField);
#ifdef DO_PONDERING
            client.ponder();
#endif
            gState.resetPlay();
        }
        
        return 0;
    }
}
#endif

int recvCommand(){
    
    DEBUGCERR("Opened Receive Thread.");
    
    //受信スレッド
    gState=0;
    
    int games = 0;
    
    char cmd_recv[BUFFER_SIZE] = { 0 };
    char cmd_send[BUFFER_SIZE] = { 0 };
    
    while (1){
        
        char *tok;
        int c;
        
        memset(cmd_recv, 0, sizeof(char)*BUFFER_SIZE);
        memset(cmd_send, 0, sizeof(char)*BUFFER_SIZE);
        
#if defined(_WIN32) && defined(FOR_ENGINE)
        DWORD NBytesRead;
        ReadFile(GetStdHandle(STD_INPUT_HANDLE), cmd_recv, sizeof(cmd_recv), &NBytesRead, NULL);
#else
        int NBytesRead = recv(NetworkInfo.dstSocket, cmd_recv, sizeof(cmd_recv), 0);
        if (NBytesRead<0){ return -1; }
#endif
        //convert to o-moji
        for (int i = 0; i<NBytesRead; ++i){
            cmd_recv[i] = toupper(cmd_recv[i]);
        }
        
        c = 0;
        
        while (c >= 0){
            
            tok = &cmd_recv[c];
            
            CERR << "Server >> " << tok << endl;
            if (strstr(tok, "CONNECTED")){
            }
            else if (strstr(tok, "LOGIN OK")){
            }
            else if (strstr(tok, "LOGIN NG")){
#if defined(CREATE_PONDER_THREAD)
                gState.requestToExit();
#endif
                return -1;
            }
            else if (strstr(tok, "ISREADY")){
                //CERR << "init0";
                mvTT.init();
                //CERR << "init0.5";
                bdTT.init();
                
                //CERR << "init1";
                
                sendCommand("READYOK");
            }
            else if (strstr(tok, "NEWGAME")){
                
                myField.initGame();
                //プレーヤー名が送られてくるので記録
                {
                    char tmp[32];
                    if( GetArgument(tmp,sizeof(tmp),tok,1) ){
                        myField.setName(0,std::string(tmp));
                    }else{
                        myField.setName(0,"First");
                    }
                    if( GetArgument(tmp,sizeof(tmp),tok,2) ){
                        myField.setName(1,std::string(tmp));
                    }else{
                        myField.setName(1,"Second");
                    }
                }
                client.init1G(myField);
            }
            else if (strstr(tok, "POSITION")){
                //ストーンの位置については毎回初期化
                char tmp[32];
                for (int i = 0; i<16; i++){
                    GetArgument(tmp, sizeof(tmp), tok, 2 * i + 1);
                    float x = (float)atof(tmp);
                    GetArgument(tmp, sizeof(tmp), tok, 2 * i + 2);
                    float y = (float)atof(tmp);
                    
                    myField.setStonePos(i, x, y);
                }
            }
            else if (strstr(tok, "SETSTATE")){
                
#if defined(CREATE_PONDER_THREAD)
                if (gState.isThinking()){
                    gState.requestToStop();//思考終了命令を出す
                    while (gState.isThinking()){}
                }
#endif
                
                //状態を設定
                char tmp[32];
                GetArgument(tmp, sizeof(tmp), tok, 1);
                myField.setTurn(atoi(tmp));
                GetArgument(tmp, sizeof(tmp), tok, 2);
                myField.setEnd(atoi(tmp));
                GetArgument(tmp, sizeof(tmp), tok, 3);
                myField.setToEnd(atoi(tmp));
                
                if (myField.my_first_color >= 0 && !myField.isMyTurn()){
                    //自分のターンでない場合は、ここからRUNSHOTが来るまで相手手番中の思考に入る
                    //まだ自分の先後がわからない場合は何もしない
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
            else if (strstr(tok, "GO")){
                char tmp[32];
                //手を決めて送る
                
                if (myField.my_first_color<0){
                    //ここで自分の先後がわかる
                    if (myField.getTurnColor() == 0){
                        myField.setMyFirstColor(0);
                    }
                    else{
                        myField.setMyFirstColor(1);
                    }
                }
                assert(myField.isMyTurn());
                
                //まず制限時間の確認
                for (int c = 0; c<2; ++c){
                    GetArgument(tmp, sizeof(tmp), tok, c + 1);
                    uint64_t tl = (uint64_t)atoi(tmp);
                    if (tl == 0ULL){//これが無制限の意味
                        myField.setTimeLimit(c, TIME_LIMIT_UNLIMITED);
                    }
                    else{
                        myField.setTimeLimit(c, tl);
                    }
                }
                
#if defined(CREATE_PONDER_THREAD)
                if (gState.isThinking()){
                    gState.requestToStop();//思考終了命令を出す
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
                gState=0;
                
                myField.broadcast();
                fMoveXY<> fmv;
                int ret=client.play(myField,&fmv);
                
                if( ret==-1 ){//投了
                    snprintf( cmd_send, sizeof(cmd_send), "CONCEDE" );
                }else{
                    snprintf( cmd_send, sizeof(cmd_send), "BESTSHOT %f %f %d", (float)fmv.vx(), (float)fmv.vy(), (int)fmv.getSpin() );
                }
                sendCommand( cmd_send );
                
#endif
                
                //置換表掃除
                if( mvTT.npages>0 ){
                    mvTT.init();
                }
                if( bdTT.npages>16 ){
                    bdTT.init();
                }
            }
            else if (strstr(tok, "TIMEOUT")){
                //タイムアウト
            }
            else if (strstr(tok, "CONCEDE")){
                //投了
#if defined(CREATE_PONDER_THREAD)
                if (gState.isThinking()){
                    gState.requestToStop();//思考終了命令を出す
                }
#endif
            }
            else if (strstr(tok, "RUNSHOT")){
                //ショット結果
            }
            else if (strstr(tok, "TOTALSCORE")){
                //総合結果
            }
            else if (strstr(tok, "SCORE")){
                //SCOREを含むので、TOTALSCOREより後の必要
                char tmp[32];
                GetArgument(tmp, sizeof(tmp), tok, 1);
                myField.setEndScore(atoi(tmp));
                myField.closeEnd();
                myField.initEnd();//次のエンドの初期化までやっておく
                client.init1G(myField);
            }
            else if (strstr(tok, "GAMEOVER")){
                char tmp[32];
                GetArgument(tmp, sizeof(tmp), tok, 1);
                if (strstr(tmp, "WIN")){
                    myField.closeGame(1);
                }
                else if (strstr(tmp, "LOSE")){
                    myField.closeGame(-1);
                }
                else{
                    myField.closeGame(0);
                }
                
                ++games;
                
#if !defined(ENGINE)
                if (games >= gNLimitGames){
                    //クライアントの限界試合数に達した場合は勝手に終了する(エンジンモードでないとき)
#if defined(CREATE_PONDER_THREAD)
                    gState.requestToExit();
#endif
                    return 1;
                }
#endif
                
                //ここで次の試合の初期化
                myField.initGame();
            }
            else if (strstr(tok, "DISCONNECT")){
#if defined(CREATE_PONDER_THREAD)
                gState.requestToExit();
#endif
                return 1;
            }
            else{
                Sleep(1);
            }
            searchToken(cmd_recv, NBytesRead, &c);
            //CERR<<c<<" , "<<cmd_recv[c]<<endl;
        }
        
    }
#if defined(CREATE_PONDER_THREAD)
    gState.requestToExit();
#endif
    return 0;
    
}

int main(int argc, char* argv[]){
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    //std::cerr << std::setprecision(8);
    
    //initialize global valiables
    gNLimitGames = 100;
    gRequirements.reset();
    
    //DEBUGCERR("Opended Client.");
    std::cerr << "Opended Client." << endl;
    
    //read arguments
    for (int i = 1; i<argc; ++i){
        
        if (strstr(argv[i], "-pre")){//Preprocessing
            gRequirements.set(Requirement::PREPROCCESS);
        }else if(strstr(argv[i], "-game")){//Game Test
            gRequirements.set(Requirement::TEST_GAME);
        }
        else if(strstr(argv[i],"-tb")){//subjective time limit( byoyomi )
            uint64_t tl=atoi(argv[i+1]);
        }
        else if(strstr(argv[i],"-t")){//subjective time limit
            uint64_t tl=atoi(argv[i+1]);
        }
        else if (strstr(argv[i], "-g")){//the number of games
            gNLimitGames = atoi(argv[i + 1]);
        }
        
    }
    
    char cmd_send[BUFFER_SIZE] = { 0 };
    char cmd_recv[BUFFER_SIZE] = { 0 };
    
    //ID設定
    //VERSIONモードでは、バージョンをIDに入れる
#ifdef BUILD_VERSION
    const std::string my_name = MY_NAME + MY_VERSION;
    gmy_id = MY_ID + MY_VERSION;
#else
    if (decideID() != 0){
        CERR << "failed to get my ID number." << endl;
        exit(1);
    }
    const std::string my_name = MY_NAME + boost::lexical_cast<std::string>(gadm.id_num);
    gmy_id = MY_ID + boost::lexical_cast<std::string>(gadm.id_num);
#endif
    
    //const std::string my_name=MY_NAME+MY_VERSION;
    //gmy_id=MY_ID+MY_VERSION;
    
    CERR << "I'm " << my_name << endl;
    
    
    //試合のための初期化処理
    //接続の前にやらないと、サーバーを立ち上げないと何も出来ない
    myField.initAll();
    if (client.initAll() < 0){//ここで終了の時
        return 0;
    }
    
#ifndef FOR_ENGINE
    //設定
    strcpy(NetworkInfo.ID, std::string(gmy_id).c_str());
    strcpy(NetworkInfo.PASS, std::string(MY_PASSWORD).c_str());
    strcpy(NetworkInfo.NAME, std::string(my_name).c_str());
    
    //通信設定
    strcpy(NetworkInfo.IP, std::string(SERVER_IP).c_str());
    NetworkInfo.PORT = PORT_NUM;
    NetworkInfo.IsConnected = FALSE;
    
#ifdef _WIN32
    //Windows 独自の設定
    WSADATA data;
    if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 0), &data)){
        cerr << "failed to initialize WSA-data." << endl;
        exit(1);
    }
#endif //_WIN32
    
    
    
    //ログイン
    snprintf(cmd_send, sizeof(cmd_send), "LOGIN %s %s %s", NetworkInfo.ID, NetworkInfo.PASS, NetworkInfo.NAME);
    CERR << cmd_send << endl;
    send(NetworkInfo.dstSocket, cmd_send, strlen(cmd_send) + 1, 0);
    
#endif //FOR_ENGINE (NOT)
    
    
    
    //通信スレッド
    recvCommand();
    
#ifndef FOR_ENGINE
    //終了
    sendCommand("LOGOUT");
    
    CERR << "logeed out." << endl;
    
    // ソケットを閉じる
#ifdef _WIN32
    closesocket(NetworkInfo.dstSocket);
    WSACleanup();
#else
    close(NetworkInfo.dstSocket);
#endif
    
    NetworkInfo.IsConnected = FALSE;
    CERR << "closed socket." << std::endl;
#endif //FOR_ENGINE
    
    return 0;
}