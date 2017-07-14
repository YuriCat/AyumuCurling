#pragma once
#include <cstdio>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <strings.h>
#include <unistd.h>
#include <sys/param.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#ifdef _WIN32
//windows用

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

//extern SIMULATION_FUNC Simulation;
//extern CREATESHOT_FUNC CreateShot;
//extern CREATEHITSHOT_FUNC CreateHitShot;
//extern GETSCORE_FUNC GetScore;
//extern HMODULE hCSDLL;

//extern CRITICAL_SECTION cs;

typedef int socklen_t;

#else
//mac用

#include <sys/fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SOCKET_ERROR 1

typedef unsigned long DWORD;
typedef int SOCKET;
typedef pthread_t HANDLE;

#define LONG_PTR long

#define strcpy_s(a,b,c) strcpy(a,c) 
#define strncpy_s(a,b,c,d) strncpy(a,c,d) 
#define _stricmp(a,b) !strstr(a,b)

#define WritePrivateProfileString(a,b,c,d) 

static DWORD GetTickCount(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	DWORD t=tv.tv_sec*1000+tv.tv_usec/1000;
	return t;
}

#endif

using std::cout;
using std::endl;

#include "CurlingSimulator.h"
#include "Message.h"

//extern bool g_EndFlg;


#define PERMISSION_ID ""
#define PERMISSION_PASS ""
#define SERVER_ADMINISTRATOR ((struct UserList **)-1)

#define NAME_SIZE 32
#define ID_SIZE 32
#define PASS_SIZE 16

#define DEFAULT_LASTEND 10
#define DEFAULT_TIME 438000

/* エラーコード一覧 */
enum
{
	NOT_ERR,		// エラーなし
	ERR_DISCONNECT,
};

/* プレイヤーの状態 */
enum
{
	IN_WAIT			= 0x0001,		// 待機モード
	IN_PRE			= 0x0002,		// 対戦準備中
	IN_READY		= 0x0004,		// 対戦準備完了
	IN_GAME			= 0x0008,		// 対戦中
	IN_DISCONNECT	= 0x0010,		// 切断
	UNKNOWN			= 0x0020		// エラー
};


struct NetworkInfo
{
	SOCKET				socket;
	struct sockaddr_in	addr;
	socklen_t					addr_size;
};

typedef struct _GameStateEx{
	int		ShotNum;		// 現在のショット数
	// ShotNum が n の場合、次に行うショットが n+1 投目になる
	
	int		CurEnd;			// 現在のエンド数
	int		LastEnd;		// 最終エンド数
	int		Score[10];		// 第1エンドから第10エンドまでのスコア
	bool	WhiteToMove;	// 手番の情報
	// WhiteToMove が 0 の場合次のショットを行うのは先手、WhiteToMove が 1 の場合次のショットを行うのは後手となる
	
	float	body[16][2];	// body[n] は n 投目のストーンの位置座標を表す
	// body[n][0] は n 投目のストーンの x 座標、body[n][1] は n 投目のストーンの y 座標を表す
	
	
	// 拡張情報
	struct UserList	*First;
	struct UserList	*Second;
	float		Random;				// シミュレーション時に扱う乱数値
	char		LogFileName[1024];	// ログファイル名
	
} GAMESTATEEX, *PGAMESTATEEX;

struct UserList
{
	char			id[ID_SIZE];				// ID
	bool			permission;					// 管理者権限
	char			password[PASS_SIZE];		// パスワード
	char			user_name[NAME_SIZE];		// ユーザー名
	NetworkInfo	network_info;			// ネットワーク情報
	
	unsigned long	starting_time;				// goコマンドを送られた時間
	unsigned long	remaining_time;				// 持ち時間
	int				status;						// プレイヤー状態
	
	void SetUser(char *aid, char *apassword, char *auser_name )
	{
		//COUT<<"UserList::SetUser()"<<endl;
		
		strcpy_s(id, sizeof(id), aid);
		permission = false;
		
		strcpy_s(password, sizeof(password), apassword);
		strcpy_s(user_name, sizeof(user_name), auser_name);
		status = IN_WAIT;
	}
	
};

typedef struct _System
{
	float				rand;
	//struct _AutoMatch	AutoMatch;
} SYSTEM, *PSYSTEM;

extern struct _System System;


void ReleaseGame(GAMESTATEEX *game_state);