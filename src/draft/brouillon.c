/*définir communications :
    - présentation des joueurs (nom, mode de jeu)
    - échange de coups
    - vérification de règles identiques

arbitre qui stocke la liste des coups 
et qui la donne aux participants à la fin

Mode auto :
1. connexion à l'arbitre
2. pool de gens en attente
3. matcmaking selon critères (elo, mode de jeux, ...), 
    au début un seul mode de jeu et pas de elo
4. début de partie -> jusqu'à fin ou interruption
5. retour dans le pool.

mode selection :
connextion avec un utilisateur humain qui choisi un adversaire puis idem.

définir comportement fin de partie

choix des règles : 
- limitation de profondeur
- horloge globale(combien de temps)
- horloge coup par coup
- bo combien ? définir qui a les blancs à chaque partie d'un bo

pour la suite
- autoriser des choix de couleur
- autoriser de rollback pour un humain
- hint...

REGAERDE A PARTIR DE ICI

Serveur Design :*/

#define MAX_NAME_LENGTH
#define MAX_PLAYERS
#define MAX_LISTEN

typedef struct player{
    int socket, index;
    char username[MAX_NAME_LENGTH];
    match_t* m;
} player_t;

typdef player_t* players;

typdef struct match{
    player_t* players[2];
    clock_t timers[2];
    char current_player;
    game_p game;
    clock_t last_write;
    int index;
} match_t;

typdef match_t* match_p;

typedef struct serv{
    player_p players[MAX_PLAYERS];
    match_p matchs[MAX_PLAYERS/2 + 1];
    int socket, nb_players, nb_matchs, max_socket;
} serv_t;

typedef serv_t* serv_p;

serv_p serv_new(void);

int player_connect(serv_p server);

int player_disconnect(int* index, int* socket, serv_p serveur);

int match_start(serv_p server, player_p p1, player_p p2);

/*    Pop the match,
    put the two player at the right position in the queue (players), 
    send the recap to players*/
int match_end()

queue : 
    in arrays
    push at the end then swap with first in match
    when pop, free the cell then put last inside

server pseudo code :
/* on any failed read or write disconned the player*/

server = serv_new()
for(;;)
    si deux personnes dans la queue -> début match
    select()
        if stdin "exit" -> break
        if nouveau 
            add
        else 
            for each player
            if talking
                check move
                if valid
                    ENDCLOCK
                    if mat or draw
                        send result to both
                        send move list (en uchar)
			end match
                    else        
                        do move
                        send back to oppo
                        STARTCLOCK
                        change current player
                else 
                    send illegal
                break ?

client pseudo code
	char*   char* int 	bool   char* char*
client adresse port game_length human name (password future) 
  connect
  send is_human[1 byte] timerchoice[1 int (4bytes ou sizeofint)] name[inférieur à max_length]
  while(1)
    select
    if stdin "exit" -> break
    if message début partie
      if white -> play
        CALCUL
      while(1)
        select (pour pouvoir utiliser stdin)
        if stdin ff, concede, surrend, exit -> break
	      if sock 
          read 
          if CODE(1byte)
            if ILLEGAL ??
            if game ended -> break;
          else (move 2 bytes)
            update
	    CALCUL
            write
      
/*
database 

player
  id
  username
  hash_password
  salt
  elo
  is_human

match
  id
  player1
  player2
  result (white/black win, draw, or white/black disconnected)
  clock_1
  clock_2
  move_list
  rules(
  init_time
  remise)
  */
