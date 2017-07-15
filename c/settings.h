/*
 settings.h
 Katsuki Ohto
 */

#ifndef DCURLING_SETTINGS_H_
#define DCURLING_SETTINGS_H_

// デジタルカーリング設定

/**************************基本設定**************************/

// ルール設定

// 着手の外乱設定 標準ではUEC杯ルール
#define RULE_ERROR_GAT // GAT杯の乱数ルール

// ビルド設定

//#define MINIMUM // 本番用
//#define DEBUG // デバッグ出力

// メソッドアナライザー使用
// 本番用では自動的にオフ
#define USE_ANALYZER

// 浮動小数点演算の型。高速な方を使う
// メモリ節約のためのfloat使用は内部で固定されているので関係無し
//using fpn_t = float;
using fpn_t = double;

// 精度を上げたいときの浮動小数点型
//using pfpn_t = __float128;
using pfpn_t = long double;

// 評価値を浮動小数で出すか整数で出すか
//#define USE_EVAL_INT

#ifdef USE_EVAL_INT
using eval_t = int64_t;
#else
using eval_t = fpn_t;
#endif

// スレッド
#define N_THREADS (1)
using thread_t = std::thread;

// Box2D利用時の変数

// FPS
constexpr fpn_t BOX2D_TIMESTEP = 1.0 / 100.0; // 1秒分に相当するステップ数の逆数
constexpr fpn_t BOX2D_TIMESTEP_OFFICIAL = 1.0f / 1000.0f; // 1秒分に相当するステップ数の逆数(公式のシミュレータ)
// シミュレーション精度
constexpr int BOX2D_VELOCITY_ITERATIONS = 10;
constexpr int BOX2D_POSITION_ITERATIONS = 10;

// IDファイルの場所
const std::string DIRECTORY_ID = "./id/";

/**************************「歩」思考設定**************************/

// プロフィール
const std::string MY_NAME = "Ayumu";
const std::string MY_VERSION = "160822";
const std::string MY_COACH = "KatsukiOhto";

// 接続用プロフィール
const std::string MY_ID = "ayumu";
const std::string MY_PASSWORD = "walkpass";

#ifdef FOR_NETWORK
const std::string SERVER_IP = "130.153.151.129";
#else
const std::string SERVER_IP = "127.0.0.1";
#endif

// 重要な設定

#define MONITOR // 着手決定関連の表示
//#define BROADCAST // 試合進行実況

//#define TENPLAYOUTS_ONLY // プレイアウトを10回に制限(デバッグ用)
//#define POLICY_ONLY // 方策関数そのままでプレイ

// 相手手番中の思考を行うために別スレッドを立てるか
//#define CREATE_PONDER_THREAD
// 相手手番中の思考
//#define DO_PONDERING

// 投了設定
constexpr int CONCEDE_MAX_END = 4; // 投了する可能性のある最大エンド
constexpr int CONCEDE_MIN_END = 1; // 投了する可能性のある最小エンド

// 投了する勝率ライン
// 0未満に設定すると投了しない
// 0に設定すると、勝てない事が完全に証明出来たときのみ投了
constexpr double CONCEDE_WP = -100;

// ヌルムーブ(パスとして変な所に投げる)の使用設定
//#define USE_NULLMOVE

// 純粋モンテカルロかモンテカルロ木探索か
//#define USE_MCTS

// 末端の獲得スコアを試合展開によっていじるか
//#define MODIFY_END_SCORE

// ルート直下の補助評価点を加算するか
//#define USE_SUBEVAL

// 局面認識の際に各情報の認識精度差をつけるか
//#define USE_BASE_RANGE

// 静的評価点を使うか
//#define USE_STATIC_VALUE_FUNCTION

// 行動価値関数を使うか
//#define USE_POLICY_SCORE

// 細かい着手生成を行うか
//#define USE_HANDMADE_MOVES

// ルートで決定着手を行うか
//#define USE_ROOT_ONLY_MOVE

// 局面評価が自明な時にプレイアウトを途中で打ち切るか
//#define USE_PLAYOUT_LEAVING

// MATE判定のタイミングで加点するか
//#define USE_MATE_TIMING_REWARD

// Kernel Regression を行うか
//#define USE_KERNEL_REGRESSION

// ガウシアンにより行動のずれを最適化するか
//#define USE_GAUSSIAN_OPTIMIZATION

/**************************以下は直接変更しない**************************/

// スレッド数
#ifdef N_THREADS

#if N_THREADS <= 0
#undef N_THREADS
#define N_THREADS (1)
#endif

// スレッド数として2以上が指定された場合は、マルチスレッドフラグを立てる
#if N_THREADS >= (2)
#define MULTI_THREADING
#endif

#endif // N_THREADS


// 本番用のとき、プレーに無関係なものは全て削除
// ただしバグ落ちにdefineが絡む可能性があるので、強制終了時の出力は消さない
#ifdef MINIMUM

// デバッグ
#ifdef DEBUG
#undef DEBUG
#endif

// アナライザ
#ifdef USE_ANALYZER
#undef USE_ANALYZER
#endif

// 実況
#ifdef BROADCAST
#undef BROADCAST
#endif

#endif // MINIMUM

#endif // DCURLING_SETTINGS_H_
