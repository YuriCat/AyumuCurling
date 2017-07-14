#include "System.h"
#define MAX_THREAD_NUM 128
#define BUFFER_SIZE 1024

UserList user[2];
GAMESTATEEX game_state;

#include "../log/dclMaker.hpp" // .dcl形式の棋譜作成用

DigitalCurling::GameBoard bd;
std::string dclName;

struct _System System;

int Send(SOCKET sock,const char* buffer, int size, int flag ){
	CERR<<"To Sock"<<sock<<" >> "<<buffer<<endl;
	int status=send(sock, buffer, size , flag);
	if( status == -1 ){
		cerr<<"send failed..."<<endl;
	}
	return status;
}

int Recv(SOCKET sock,char* buffer, int size, int flag ){
	int status=recv(sock, buffer, size , flag);
	CERR<<"From Sock"<<sock<<" << "<<buffer<<endl;
	//cerr<<"send status : "<<status<<endl;
	if( status == -1 ){
		cerr<<"recv failed..."<<endl;
	}
	return status;
}

int Talk(SOCKET sock,const char* buffer, char *dst ){
	CERR<<"To Sock"<<sock<<" >> "<<buffer<<endl;
	int status=send(sock, buffer, strlen(buffer)+1 , 0);
	//cerr<<"send status : "<<status<<endl;
	if( status == -1 ){
		cerr<<"send failed..."<<endl;return -1;
	}
	//そのまま受け取り
	/*if( dst!=nullptr ){
		status=recv(sock, dst, sizeof(dst) , 0);
		CERR<<"From Sock"<<sock<<" << "<<dst<<endl;
	}else{
		char c[8];
		status=recv(sock, c, sizeof(c) , 0);
	}
	if( status == -1 ){
		cerr<<"recv failed..."<<endl;return -1;
	}*/
	return status;
}

// マッチメイク
bool MatchMake(GAMESTATEEX *game_state, int sente, int gote, int last_end,
               unsigned long time1, unsigned long time2)
{
	
	//CERR<<"MatchMake()"<<endl;
	
	bool bRet = false;
	UserList *p, *user1 = &user[sente], *user2 = &user[gote];
	
	// 先手情報取得
	
	if (user1 != nullptr && user2 != nullptr
		&& user1->status == IN_WAIT && user2->status == IN_WAIT)
	{
		
		char msg[BUFFER_SIZE] = {0};
		char snd[BUFFER_SIZE] = {0};
		
		// ゲームの初期化
		game_state->CurEnd = 0;
		game_state->ShotNum = 0;
		game_state->WhiteToMove = 0;
		game_state->LastEnd = last_end;
		game_state->Random = System.rand;
		memset(game_state->Score, 0x00, sizeof(int) * 10);
		memset(game_state->body, 0x00, sizeof(float) * 32);
        
		// プレイヤー状態の変更
		user1->status = user2->status = IN_PRE;
		
		// プレイヤーの残り時間の設定
		user1->remaining_time = time1;
		user2->remaining_time = time2;
		
		// 対戦相手情報の設定
		game_state->First = user1;
		game_state->Second = user2;
        
        // ルールと人の記録
        bd.initGame();
        //bd.setName(game_state->First->id, game_state->Second->id);
        bd.setName(game_state->First->user_name, game_state->Second->user_name);
        bd.setRule(last_end, 16, game_state->Random);
        bd.setLimit(time1, time2);
		
		// 対戦開始の合図
		Send(game_state->First->network_info.socket, "ISREADY", strlen("ISREADY") + 1, 0);
		Recv(game_state->First->network_info.socket, msg, sizeof(msg), 0);
		
		if(!strstr(msg, "READYOK")){return false;}
		
		Send(game_state->Second->network_info.socket, "ISREADY", strlen("ISREADY") + 1, 0);
		Recv(game_state->Second->network_info.socket, msg, sizeof(msg), 0);
		
		if(!strstr(msg, "READYOK")){return false;}
		
		//Talk( game_state->First->network_info.socket, "ISREADY", msg );
		//Talk( game_state->Second->network_info.socket, "ISREADY", msg );
		
		bRet = true;
	}
	
	return bRet;
}

// 改行文字の削除
void DeleteNL(char *msg)
{
	char *p;
	p = msg;
	
	while (*p != 0x00){
		if (*p == '\n'){
			*p = 0x00;
			break;
		}
		p++;
	}
	return;
}

/*
void PrintMessage(UserList *lpUser, char *msg)
{
	// 実機サーバーコンソール及びリモートにメッセージを表示する
	if (lpUser != NULL && msg != NULL){
		Send(lpUser->network_info->socket, msg, strlen(msg) + 1, 0);
	}
	printf("%s\n", msg);
}*/

bool GetArgument(char *lpResult, size_t numberOfElements, char *msg, int n)
{
	bool bRet = false;
	char *p, *q;
	
	if (msg != nullptr){
		p = msg;
		while (*p == ' '){
			p++;
		}
		// ポインタを取得したい引数の先頭に合わせる
		for (int i = 0; i<n; i++){
			while (*p != ' '){
				if (*p == 0x00){
					return false;
				}
				p++;
			}
			while (*p == ' '){
				p++;
			}
		}
		// 取得したい引数をlpResultにコピーする
		q = strstr(p, " ");
		if (q == nullptr){
			strcpy_s(lpResult, numberOfElements, p);
			bRet = true;
		}
		else{
			strncpy_s(lpResult, numberOfElements, p, q - p);
			if ((unsigned int)(q - p) < numberOfElements){
				lpResult[q - p] = 0x00;
			}
			bRet = true;
		}
	}
	return bRet;
}

// ゲーム情報の送信
bool SendGameState(GAMESTATEEX *game_state)
{
	bool bRet = true;
	char buffer[BUFFER_SIZE] = {0};
	
	// POSITIONコマンド
	snprintf(buffer, sizeof(buffer), 
			 "POSITION %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
			 game_state->body[0][0], game_state->body[0][1],
			 game_state->body[1][0], game_state->body[1][1],
			 game_state->body[2][0], game_state->body[2][1],
			 game_state->body[3][0], game_state->body[3][1],
			 game_state->body[4][0], game_state->body[4][1],
			 game_state->body[5][0], game_state->body[5][1],
			 game_state->body[6][0], game_state->body[6][1],
			 game_state->body[7][0], game_state->body[7][1],
			 game_state->body[8][0], game_state->body[8][1],
			 game_state->body[9][0], game_state->body[9][1],
			 game_state->body[10][0], game_state->body[10][1],
			 game_state->body[11][0], game_state->body[11][1],
			 game_state->body[12][0], game_state->body[12][1],
			 game_state->body[13][0], game_state->body[13][1],
			 game_state->body[14][0], game_state->body[14][1],
			 game_state->body[15][0], game_state->body[15][1]
			 );
	//Send(game_state->First->network_info.socket, buffer, strlen(buffer)+1, 0);
	//Send(game_state->Second->network_info.socket, buffer, strlen(buffer)+1, 0);
	
	Talk(game_state->First->network_info.socket, buffer, NULL);
	Talk(game_state->Second->network_info.socket, buffer, NULL);
	
	// SETSTATEコマンド
	snprintf(buffer, sizeof(buffer), 
			 "SETSTATE %d %d %d %d", game_state->ShotNum, game_state->CurEnd, game_state->LastEnd, game_state->WhiteToMove);
	//Send(game_state->First->network_info.socket, buffer, strlen(buffer)+1, 0);
	//Send(game_state->Second->network_info.socket, buffer, strlen(buffer)+1, 0);
	
	
	Talk(game_state->First->network_info.socket, buffer, NULL);
	Talk(game_state->Second->network_info.socket, buffer, NULL);
	
	return bRet;
}

// 実際に演算されたショットの送信
bool SendRunShot(GAMESTATEEX *game_state, SHOTVEC ShotVec)
{
	bool bRet = true;
	char buffer[BUFFER_SIZE] = {0};
	char rcv[8];
	
	snprintf(buffer, sizeof(buffer), "RUNSHOT %f %f %d", ShotVec.x, ShotVec.y, ShotVec.angle);
	//Send(game_state->First->network_info.socket, buffer, strlen(buffer)+1, 0);
	
	//Send(game_state->Second->network_info.socket, buffer, strlen(buffer)+1, 0);
	
	Talk(game_state->First->network_info.socket, buffer, NULL);
	Talk(game_state->Second->network_info.socket, buffer, NULL);
	
	return bRet;
}

// スコアの送信
bool SendScore(GAMESTATEEX *game_state, int score)
{
	bool bRet = true;
	char buffer[BUFFER_SIZE] = {0};
	
	snprintf(buffer, sizeof(buffer), "SCORE %d", score);
	
	//Send(game_state->First->network_info.socket, buffer, strlen(buffer)+1, 0);
	//Send(game_state->Second->network_info.socket, buffer, strlen(buffer)+1, 0);
	
	Talk(game_state->First->network_info.socket, buffer, NULL);
	Talk(game_state->Second->network_info.socket, buffer, NULL);
    
	return bRet;
}

// GOコマンドの送信
bool SendGoCommand(GAMESTATEEX *game_state)
{
	//CERR<<"send go command"<<endl;
	
	char buffer[BUFFER_SIZE] = {0};
	
	snprintf(buffer, sizeof(buffer), "GO %d %d", int(game_state->First->remaining_time), int(game_state->Second->remaining_time));
	
	if (game_state->WhiteToMove == 0){
		
		Send(game_state->First->network_info.socket, buffer, strlen(buffer) + 1, 0);
		//game_state->First->starting_time = GetTickCount();
	}else{
		Send(game_state->Second->network_info.socket, buffer, strlen(buffer) + 1, 0);
		//game_state->Second->starting_time = GetTickCount();
	}
	
	return true;
}

// 勝敗の送信
int SendResult(GAMESTATEEX *game_state)
{
	bool bRet = true;
	char buffer[BUFFER_SIZE] = {0};
	
	int fScore = 0, sScore = 0;
	for (int i = 0; i < game_state->LastEnd; i++){
		if (game_state->Score[i] > 0){
			fScore += game_state->Score[i];
		}
		else{
			sScore += game_state->Score[i] * -1;
		}
	}
	
	// 総得点の送信
	cerr << game_state->First->id << " " << fScore << " - " << sScore << " ";
    cerr << game_state->Second->id << endl;
	
	snprintf(buffer, sizeof(buffer), "TOTALSCORE %d %d", fScore, sScore); 
	
	//Send(game_state->First->network_info.socket, buffer, strlen(buffer)+1, 0);
	//Send(game_state->Second->network_info.socket, buffer, strlen(buffer)+1, 0);
	
	Talk(game_state->First->network_info.socket, buffer, NULL);
	Talk(game_state->Second->network_info.socket, buffer, NULL);
	
	// 勝敗の送信
	if (fScore > sScore){
		//Send(game_state->First->network_info.socket, "GAMEOVER WIN", strlen("GAMEOVER WIN") + 1, 0);
		//Send(game_state->Second->network_info.socket, "GAMEOVER LOSE", strlen("GAMEOVER LOSE") + 1, 0);
		
		Talk(game_state->First->network_info.socket, "GAMEOVER WIN", NULL);
		Talk(game_state->Second->network_info.socket, "GAMEOVER LOSE", NULL);
		
		return 0;
	}else if (fScore == sScore){
		//Send(game_state->First->network_info.socket, "GAMEOVER DRAW", strlen("GAMEOVER DRAW") + 1, 0);
		//Send(game_state->Second->network_info.socket, "GAMEOVER DRAW", strlen("GAMEOVER DRAW") + 1, 0);
		
		Talk(game_state->First->network_info.socket, "GAMEOVER DRAW", NULL);
		Talk(game_state->Second->network_info.socket, "GAMEOVER DRAW", NULL);
		
		return 2;
	}else{
		//Send(game_state->First->network_info.socket, "GAMEOVER LOSE", strlen("GAMEOVER LOSE") + 1, 0);
		//Send(game_state->Second->network_info.socket, "GAMEOVER WIN", strlen("GAMEOVER WIN") + 1, 0);
		
		Talk(game_state->First->network_info.socket, "GAMEOVER LOSE", NULL);
		Talk(game_state->Second->network_info.socket, "GAMEOVER WIN", NULL);
		
		return 1;
	}

}

// 対戦準備
DWORD ReadyProc(GAMESTATEEX *game_state, UserList *lpUser)
{
	DWORD dwRet = IN_PRE;
	char buffer[BUFFER_SIZE]={0};
	
	// プレイヤー状態の変更
	lpUser[0].status = IN_READY;
	lpUser[1].status = IN_READY;
	
	if (game_state != NULL && (game_state->First->status & IN_READY) && (game_state->Second->status & IN_READY)){
		// NEWGAMEコマンドの送信
		
		snprintf(buffer, sizeof(buffer), "NEWGAME %s %s",
                 game_state->First->id, game_state->Second->id);
		
		//Send(game_state->First->network_info.socket, buffer, sizeof(buffer), 0);
		//Send(game_state->Second->network_info.socket, buffer, sizeof(buffer), 0);
		
		Talk(game_state->First->network_info.socket, buffer, NULL);
		Talk(game_state->Second->network_info.socket, buffer, NULL);
		
		// プレイヤーの状態の変更
		game_state->First->status = IN_GAME;
		game_state->Second->status = IN_GAME;
		
		// ゲーム情報の送信
		SendGameState(game_state);
		
		// GOコマンドの送信
		SendGoCommand(game_state);
		
		// 現在のゲーム状態を返す
		dwRet = IN_GAME;
	}
	
	return dwRet;
}

// ゲーム進行
int SimulateProc(float x, float y, bool turn, GAMESTATEEX *game_state)
{
	int iRet;
	char buffer[BUFFER_SIZE];
	
	//CERR<<"start to proc game."<<endl;
    
    // 結果記録用
    int e = game_state->CurEnd;
    int t = game_state->ShotNum;
    
    // 現局面記録
    bd.setStones(e, t, game_state->body);
	
	SHOTVEC ShotVec, ResShot;
	ShotVec.x = x; ShotVec.y = y; ShotVec.angle = turn;
	
	iRet = Simulation((GAMESTATE *)game_state, ShotVec, game_state->Random, &ResShot, -1);
	
	// RUNSHOT
	SendRunShot(game_state, ResShot);
    
    // 結果記録
    bd.setBestShot(e, t, x, y, static_cast<DigitalCurling::Spin>(turn));
    bd.setRunShot(e, t, ResShot.x, ResShot.y, static_cast<DigitalCurling::Spin>(ResShot.angle));
    // 局面記録
    assert(t < 16);
    bd.setStones(e, t + 1, game_state->body);
	
	// ストーン座標と局面情報の送信
	SendGameState(game_state);
	
	// 16投投げ終わった場合の処理
	//-------------------------------------
	if (game_state->ShotNum == 0){
		// スコアの送信
		SendScore(game_state, game_state->Score[game_state->CurEnd - 1]);
        
        // 得点の記録
        int e = game_state->CurEnd - 1;
        bd.setScore(e, game_state->Score[game_state->CurEnd - 1]);
        
        // 盤面の初期化
        for(int i = 0; i < 16; ++i){
            for(int j = 0; j < 2; ++j){
                game_state->body[i][j] = 0;
            }
        }
		
		// 最終エンドの場合の処理
		//-------------------------------------
		if (game_state->CurEnd >= game_state->LastEnd){
			// 勝敗の送信
			iRet=SendResult(game_state);
			return iRet;
		}
	}
	
	// GOコマンド
	snprintf(buffer, sizeof(buffer), "GO %d %d",
             (int)game_state->First->remaining_time,
             (int)game_state->Second->remaining_time);
	
	if (game_state->WhiteToMove == 0){
		Send(game_state->First->network_info.socket, buffer, strlen(buffer) + 1, 0);
		//game_state->First->starting_time = GetTickCount();
	}else{
		Send(game_state->Second->network_info.socket, buffer, strlen(buffer) + 1, 0);
		//game_state->Second->starting_time = GetTickCount();
	}
	
	
	return iRet;
}

int main(int argc, char* argv[])
{
	
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	
	// 全試合情報
	int wins[2] = {0, 0};
	int draws = 0;
	int ngames = 1;
	unsigned int tl = 438000;
	int nends = 10;
    unsigned short port = 9876;
    std::string dclDirectory = "";
    int mode = 1;
    int seed = -1;
	
	// 引数から諸々を指定
	for(int i = 1; i < argc; ++i){
        if(!strcmp(argv[i], "-e")){
            nends = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-g")){
			ngames = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-l")){
            dclDirectory = std::string(argv[i + 1]);
        }else if(!strcmp(argv[i], "-p")){
            port = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-t")){
            tl = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-sm")){
            mode = 0; // stable mode
        }else if(!strcmp(argv[i], "-cm")){
            mode = 1; // change by 1 game
        }else if(!strcmp(argv[i], "-rm")){
            mode = 2; // random game mode
        }else if(!strcmp(argv[i], "-s")){
            seed = atoi(argv[i + 1]);
        }
	}
	
	char msg[BUFFER_SIZE] = {0};
	char buffer[BUFFER_SIZE] = {0};
	
	//サーバー初期化
	System.rand = 0.145f;
    if(seed == -1){
        std::random_device seed_gen;
        dice.seed(seed_gen());
    }else{
        dice.seed(seed);
    }
	
	//試合受け入れ待ち
	// ポート番号、ソケット

	
	SOCKET srcSocket;
	for (int i = 0; i < 2; i++){
		memset(&user[i].network_info, 0x00, sizeof(NetworkInfo));
		user[i].network_info.addr_size = sizeof(struct sockaddr_in);
	}

	struct timeval timeout;
	
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	// sockaddr_in 構造体
	struct sockaddr_in srcAddr;
	fd_set	fds, readfds;
	
#ifdef _WIN32
	// Windows 独自の設定 
	WSADATA data;
	WSAStartup(MAKEWORD(2, 0), &data);
#endif
	
	// ソケットの生成
	srcSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	// sockaddr_in 構造体のセット
	memset(&srcAddr, 0, sizeof(srcAddr));
	srcAddr.sin_port = htons(port);
	srcAddr.sin_family = AF_INET;
	srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int i, j;
	i = 1; j = sizeof(i);
	setsockopt(srcSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&i, j);
	
	// ソケットのバインド
	i = bind(srcSocket, (struct sockaddr *) &srcAddr, sizeof(srcAddr));
	
	// 接続の許可
	i = listen(srcSocket, 1);
	
	// パケット受信
	for (int p = 0; p < 2; p++){
		
		user[p].network_info.socket = accept(srcSocket, (struct sockaddr *) &user[p].network_info.addr, &user[p].network_info.addr_size);
		
		//cerr << "socket " << p << " : " << user[p].network_info.socket << endl;
		
		// fd_setの初期化
		FD_ZERO(&readfds);
		
		// selectで待つ読み込みソケットとしてsrcScoketを登録
		FD_SET(user[p].network_info.socket, &readfds);
		
		// 読み込み用fd_setの初期化
		memcpy(&fds, &readfds, sizeof(fd_set));
		
		// fdsに設定されたソケットが読み込み可能になるまで待つ
		select(50, &fds, nullptr, nullptr, &timeout);
		
		// 接続の受付け
		//if (FD_ISSET(srcSocket, &fds)){
		
		Send(user[p].network_info.socket, "CONNECTED", strlen("CONNECTED") + 1, 0);
		
		//試合開始状態まで進める
		int recvSize = Recv(user[p].network_info.socket, msg, sizeof(msg), 0);
		
		//CERR << recvSize << endl;
		
		//ログイン
		double dwRet = true;
		char user_name[NAME_SIZE], id[ID_SIZE], password[PASS_SIZE];
		if (GetArgument(id, sizeof(id), msg, 1) == false){
			dwRet = false;
		}
		if (GetArgument(password, sizeof(password), msg, 2) == false){
			dwRet = false;
		}
		if (GetArgument(user_name, sizeof(user_name), msg, 3) == false){
			strcpy(user_name, id);
		}
		Send(user[p].network_info.socket, "LOGIN OK", strlen("LOGIN OK") + 1, 0);
		
		user[p].SetUser(id, password, user_name);
		//}
	}
	
	//試合開始準備
	//CERR<<user[0].network_info.socket<<" , "<<user[1].network_info.socket<<endl;
	
	std::srand((unsigned int)time(NULL));
	
	//試合
	for(int g = 0; g < ngames; ++g){
		
		user[0].status = IN_WAIT;
		user[1].status = IN_WAIT;
		
        // 試合の組み方を決める
        int firstPlayer = 0;
        
        if(mode == 0){
            firstPlayer = 0; // 常に0番が先攻
        }else if(mode == 1){
            firstPlayer = g % 2; // 1試合ごとに先後交代
        }else if(mode == 2){
            firstPlayer = rand() % 2; // ランダムに先後決定
        }
		if(!MatchMake(&game_state, firstPlayer, 1 - firstPlayer, nends, tl, tl)){
			exit(1);
		}
			
		ReadyProc(&game_state, user);
		
		UserList *lpUser = &user[firstPlayer];
		
		while(1){
			unsigned long t_start = GetTickCount();
			
			// プレー待ち
			Recv(lpUser->network_info.socket, msg, sizeof(msg), 0);
			
			//printf("\r%s>%s\n\nServer>", cUserList->id, msg);
			
			// プレー
			if(strstr(msg, "CONCEDE")){
				snprintf(buffer, sizeof(buffer), "CONCEDE %s", lpUser->id);
				
				Talk(game_state.First->network_info.socket, buffer, NULL);
				Talk(game_state.Second->network_info.socket, buffer, NULL);
				
				if (game_state.First == lpUser){
					Talk(game_state.First->network_info.socket, "GAMEOVER LOSE", NULL);
					Talk(game_state.Second->network_info.socket, "GAMEOVER WIN", NULL);
					
					wins[1 - firstPlayer]++;
				}else{
					Talk(game_state.First->network_info.socket, "GAMEOVER WIN", NULL);
					Talk(game_state.Second->network_info.socket, "GAMEOVER LOSE", NULL);
                
					wins[firstPlayer]++;
				}
				break;
			}
			
			// 持ち時間計算
			unsigned long used_time = GetTickCount() - t_start;
			
			if(used_time > lpUser->remaining_time){ // 時間オーバー
				
				snprintf(buffer, sizeof(buffer), "TIMEOUT %s", lpUser->id);
				
				Talk(game_state.First->network_info.socket, buffer, NULL);
				Talk(game_state.Second->network_info.socket, buffer, NULL);
				
				// 勝敗の送信
				if (game_state.First == lpUser){
					//snprintf(buffer, sizeof(buffer), "GAMEOVER LOSE");
					//Send(game_state.First->network_info.socket, buffer, strlen(buffer)+1, 0);
					//snprintf(buffer, sizeof(buffer), "GAMEOVER WIN");
					//Send(game_state.Second->network_info.socket, buffer, strlen(buffer)+1, 0);
					
					Talk(game_state.First->network_info.socket, "GAMEOVER LOSE", NULL);
					Talk(game_state.Second->network_info.socket, "GAMEOVER WIN", NULL);
					
					wins[1 - firstPlayer]++;
				}else{
					//snprintf(buffer, sizeof(buffer), "GAMEOVER LOSE");
					//Send(game_state.Second->network_info.socket, buffer, strlen(buffer)+1, 0);
					//snprintf(buffer, sizeof(buffer), "GAMEOVER WIN");
					//Send(game_state.First->network_info.socket, buffer, strlen(buffer)+1, 0);
					
					Talk(game_state.First->network_info.socket, "GAMEOVER WIN", NULL);
					Talk(game_state.Second->network_info.socket, "GAMEOVER LOSE", NULL);
					
					wins[firstPlayer]++;
				}
				break;
			}
			
			lpUser->remaining_time -= used_time;
				
			float x, y;
			int turn;
			
			// ショットの強さ(横方向)
			if (GetArgument(buffer, sizeof(buffer), msg, 1) == false){
				return false;
			}
			x = (float)atof(buffer);
			
			// ショットの強さ(縦方向)
			if (GetArgument(buffer, sizeof(buffer), msg, 2) == false){
				return false;
			}
			y = (float)atof(buffer);
			
			// 回転方向(0:右回転、1:左回転)
			if (GetArgument(buffer, sizeof(buffer), msg, 3) == false){
				return false;
			}
			turn = atoi(buffer);

			
			// シミュレート
			int ret = SimulateProc(x, y, (bool)turn, &game_state );
			
			lpUser = (game_state.WhiteToMove == 0) ? game_state.First : game_state.Second;
			
			if (game_state.ShotNum == 0){
				if (game_state.CurEnd >= game_state.LastEnd){
					//試合終了のため抜ける
					if(ret == 2){
						draws++;
					}else{
						wins[firstPlayer ^ ret]++;
					}
					break;
				}
			}
		}
		cerr << user[0].id << " : " << wins[0] << "  ";
        cerr << user[1].id << " : " << wins[1] << "  Draws : " << draws << endl;
        
        // dcl形式のログをdump
        if(dclDirectory != ""){
            std::string dclPath = dclDirectory + "/" + DigitalCurling::DCL::toDclName(bd) + ".dcl";
            cerr << "dcl file name = " << dclPath << endl;
            std::ofstream ofs;
            ofs.open(dclPath, std::ios::out);
            if(ofs){
                ofs << DigitalCurling::DCL::toDclString(bd);
                ofs.close();
            }else{
                cerr << "failed to open dcl file." << endl;
            }
        }
	}
	
	Send(user[0].network_info.socket, "DISCONNECT", strlen("DISCONNECT") + 1, 0);
	Send(user[1].network_info.socket, "DISCONNECT", strlen("DISCONNECT") + 1, 0);
	
	//Recv( user[0].network_info.socket, msg, sizeof( msg ), 0 );
	//Recv( user[1].network_info.socket, msg, sizeof( msg ), 0 );
	
	//Talk( game_state.First->network_info.socket, "DISCONNECT", NULL );
	//Talk( game_state.Second->network_info.socket, "DISCONNECT", NULL );
	
	sleep(1);
	
#ifdef _WIN32
	closesocket(user[0].network_info.socket);	
	closesocket(user[1].network_info.socket);
	WSACleanup();
#else
	close(user[0].network_info.socket);	
	close(user[1].network_info.socket);
#endif
	
	
	
	return 0;
}