
#pragma once
#include "System.h"

void PrintMessage(struct UserList *lpUser, char *msg);
bool SendGameState(struct _GameStateEx *game_state);
bool SendGoCommand(struct _GameStateEx *game_state);
unsigned long DoCommand(char *msg, struct UserList **dpUser, unsigned long *err, void *lpResult, struct NetworkInfo *pNetworkInfo);
