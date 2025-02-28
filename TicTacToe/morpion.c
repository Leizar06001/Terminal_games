#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>
#include <pthread.h>
#include <time.h>

#define DEFAULT_PORT 9468
#define BUFFER_SIZE 1024

#define MAP_SIDE    3
#define yTOP        3
#define xLEFT       1

static int socket_port = DEFAULT_PORT;

const char vsteps = 2;
const char hsteps = 3;

static char map[MAP_SIDE * MAP_SIDE];
static int xCur = 2;
static int yCur = 2;

char current_player = 1;
char self_player = 1;
char opponent_player = 2;
char winner = 0;

int exit_flags = 0;
int new_socket;
int server_fd;

pthread_t thread1;
pthread_mutex_t mut_exit_flags;
pthread_mutex_t mut_new_socket;

static char server_mode = 0;
static int end_game = 0;
static int game_steps = 0;
static char game_draw = 0;

static int score_self = 0;
static int score_opponent = 0;

char msg[32];


int client_init(char *server_ip);
int server_init();
void enableRawMode();
void disableRawMode();
void moveCursor(char direction, char steps);
void resetCursor();
void mvCursor(int x, int y);
void prt_debug();
int read_exit_flags();
void write_exit_flags(int flags);
int send_message(char *message);
void update_whos_turn();
int check_win(char player);
void init_new_game();
void choose_and_send_first_player(int player);
int check_draw();
int check_args(int argc, char **argv);
int init_network(char **argv);
int init_vars();
void exit_game();
void *thread_function(void *arg);


int main(int argc, char **argv) {
    if (check_args(argc, argv) != 0){
        return EXIT_FAILURE;
    }
    if (init_network(argv) != 0){
        return EXIT_FAILURE;
    }
    if (init_vars() != 0){
        return EXIT_FAILURE;
    }

    char c;
    int id1 = 1;

    if (pthread_create(&thread1, NULL, thread_function, &id1) != 0) {
        fprintf(stderr, "Error creating thread 1\n");
        return EXIT_FAILURE;
    }

    update_whos_turn();
    while (read_exit_flags() == 0) {

        c = getchar();
        if (c == '\033') { // If the first value is esc
            getchar(); // Skip the [
            char cc = getchar(); // The real value
            switch(cc) { // The real value
                case 'A': moveCursor('A', vsteps); break;
                case 'B': moveCursor('B', vsteps); break;
                case 'C': moveCursor('C', hsteps); break;
                case 'D': moveCursor('D', hsteps); break;
            }
        } else if (c == ' ') {
            if (self_player != current_player){
                continue;
            }
            int pos = (yCur - 1) * MAP_SIDE + xCur - 1;
            if (map[pos] == 0) {
                map[pos] = self_player;
                printf("\033[D%c", 'X');

                msg[0] = '0' + pos;
                msg[1] = 0;
                send_message(msg);
                current_player = opponent_player;

                if (check_win(self_player) == 1){
                    continue;
                } else {
                    check_draw();
                }

                update_whos_turn();
                fflush(stdout);
            }
        } else if (c == 'r' && server_mode == 1){
            init_new_game();
            choose_and_send_first_player(winner);
        } else if (c == 'q') {
            write_exit_flags(1);
            break;
        }
    }

    exit_game();

    return 0;
}

void *thread_function(void *arg) {
    (void)arg;
    char buffer[BUFFER_SIZE] = {0};

    while (read_exit_flags() == 0) {

        if (read(new_socket, buffer, BUFFER_SIZE) <= 0){
            printf("Client disconnected");
            pthread_mutex_lock(&mut_exit_flags);
            exit_flags = 1;
            pthread_mutex_unlock(&mut_exit_flags);
            break;
        }

        if (buffer[0] == 'S'){
            current_player = buffer[1] - '0';
            init_new_game();
            update_whos_turn();
            fflush(stdout);
            continue;
        } else {
            buffer[1] = 0;
        }

        int pos = atoi(buffer);

        char yAdv = pos / MAP_SIDE + 1;
        char xAdv = pos % MAP_SIDE + 1;
        map[(yAdv - 1) * MAP_SIDE + xAdv - 1] = opponent_player;
        printf("\033[%d;%dH", vsteps * yAdv + yTOP - 1, hsteps * xAdv + xLEFT - 2);
        printf("O");

        current_player = self_player;

        if (check_win(opponent_player) == 1){
            continue;
        } else {
            check_draw();
        }

        update_whos_turn();
        fflush(stdout);

    }
    return NULL;
}

void exit_game(){
    disableRawMode();
    pthread_join(thread1, NULL);
    close(new_socket);
    if (server_mode){
        close(server_fd);
    }
    pthread_mutex_destroy(&mut_exit_flags);
    pthread_mutex_destroy(&mut_new_socket);
}

int init_vars(){
    pthread_mutex_init(&mut_exit_flags, NULL);
    pthread_mutex_init(&mut_new_socket, NULL);
    pthread_mutex_unlock(&mut_exit_flags);
    pthread_mutex_unlock(&mut_new_socket);
    return 0;
}

int init_network(char **argv){
    int status = 0;
    if (server_mode){
        self_player = 1;
        opponent_player = 2;
        status = server_init();
        srand(time(NULL));
        choose_and_send_first_player(0);
        init_new_game();
    } else {
        self_player = 2;
        opponent_player = 1;
        status = client_init(argv[1]);
    }
    if (status != 0){
        return -1;
    }
    return 0;
}

int check_args(int argc, char **argv){
    if (argc < 2){
        printf("Usage: Server : 0 <port>. Client : <server_ip> <port>.\n       If no port is specified, the default port is 9468.\n");
        return -1;
    }
    if (strcmp(argv[1], "0") == 0){
        server_mode = 1;
    }
    if (argc > 2){
        socket_port = atoi(argv[2]);
        if (socket_port <= 0 || socket_port > 65535){
            printf("Error: Wrong port number\n");
            return -1;
        }
    }
    return 0;
}

int check_win(char player){
    int ret = 0;
    for (int i = 0; i < MAP_SIDE; i++){
        if (map[i * MAP_SIDE] == player && map[i * MAP_SIDE + 1] == player && map[i * MAP_SIDE + 2] == player){
            ret = 1;
            break;
        }
        if (map[i] == player && map[i + MAP_SIDE] == player && map[i + 2 * MAP_SIDE] == player){
            ret = 1;
            break;
        }
    }
    if (map[0] == player && map[4] == player && map[8] == player){
        ret = 1;
    }
    if (map[2] == player && map[4] == player && map[6] == player){
        ret = 1;
    }
    if (ret == 1){
        end_game = 1;
        winner = player;
        printf("\033[15;1H");
        if (player == self_player){
            printf("You win!\n");
            score_self++;
        } else {
            printf("You loose!");
            score_opponent++;
        }
        printf("\033[17;1H");
        printf("Score: %d - %d", score_self, score_opponent);
        if (server_mode == 1){
            printf(" Press 'r' to restart");
        }
        resetCursor();
        fflush(stdout);
    }
    return ret;
}

void init_new_game(){
    memset(map, 0, sizeof(map));
    game_steps = 0;
    game_draw = 0;

    system("clear");
    enableRawMode();

    printf("<Arrow keys> move | <Space> write | <q> quit.\n");
    printf("\033[%d;1H", yTOP);
    printf("----------\n");
    printf("|  |  |  |\n");
    printf("----------\n");
    printf("|  |  |  |\n");
    printf("----------\n");
    printf("|  |  |  |\n");
    printf("----------\n");

    xCur = 2;
    yCur = 2;

    resetCursor();
}

void choose_and_send_first_player(int player){
    if (player == 0){
        current_player = rand() % 2 + 1;
    } else {
        current_player = player;
    }
    sprintf(msg, "S%d", current_player);
    send_message(msg);
}

int check_draw(){
    if (game_steps >= MAP_SIDE * MAP_SIDE){
        printf("\033[15;1H");
        printf("The match ends in a draw");
        game_draw = 1;
        if (server_mode == 1){
            printf(" Press 'r' to restart");
        }
        return 1;
    }
    return 0;
}


void resetCursor() {
    printf("\033[%d;%dH", vsteps * yCur + yTOP - 1, hsteps * xCur + xLEFT - 1);
}

void mvCursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

void prt_debug(){
    for(int i = 0; i < MAP_SIDE; i++){
        mvCursor(15, 5 + 2 * i);
        for(int j = 0; j < MAP_SIDE; j++){
            printf("%d ", map[i * MAP_SIDE + j]);
        }
    }
    mvCursor(35,1);
    printf("Curx: %d, Cury: %d", xCur, yCur);
    resetCursor();
}

void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag |= (ICANON | ECHO); // Enable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void moveCursor(char direction, char steps) {
    switch (direction) {
        case 'A': if(yCur <= 1) return;         yCur--; break; // Up
        case 'B': if(yCur >= MAP_SIDE) return;  yCur++; break; // Down
        case 'C': if(xCur >= MAP_SIDE) return;  xCur++; break; // Right
        case 'D': if(xCur <= 1) return;         xCur--; break; // Left
    }
    for(int i = 0; i < steps; ++i){
        switch (direction) {
            case 'A': printf("\033[A"); break; // Up
            case 'B': printf("\033[B"); break; // Down
            case 'C': printf("\033[C"); break; // Right
            case 'D': printf("\033[D"); break; // Left
        }
    }
}

int read_exit_flags() {
    int flags;
    pthread_mutex_lock(&mut_exit_flags);
    flags = exit_flags;
    pthread_mutex_unlock(&mut_exit_flags);
    return flags;
}

void write_exit_flags(int flags) {
    pthread_mutex_lock(&mut_exit_flags);
    exit_flags = flags;
    pthread_mutex_unlock(&mut_exit_flags);
}

int send_message(char *message) {
    return send(new_socket, message, strlen(msg), 0);
}

void update_whos_turn(){
    mvCursor(1, 11);
    if (self_player == current_player){
        printf("Your turn       ");
    } else {
        printf("Opponent's turn");
    }
    resetCursor();
}


int server_init(){
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Créer le socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attacher le socket au port 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(socket_port);

    // Lier le socket à l'adresse et au port spécifiés
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur en attente de connexions...\n");

    // Accepter une nouvelle connexion
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int client_init(char *server_ip){
    struct sockaddr_in serv_addr;

    // Créer le socket
    if ((new_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Erreur de création du socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(socket_port);

    // Convertir l'adresse IPv4 de texte en binaire
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nAdresse invalide / Adresse non supportée \n");
        return -1;
    }

    // Connecter au serveur
    if (connect(new_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nLa connexion a échoué \n");
        return -1;
    }
    return 0;
}



