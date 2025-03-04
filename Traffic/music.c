#include "main.h"
#include <signal.h>
#include <fcntl.h>

#include <sys/wait.h>

// Variable globale pour stocker le PID du processus enfant
int control_pipe[2];
int status_pipe[2];  // To read mpg123 output
int audio_pid = -1;
bool audio_player_started = false;

void kill_audio_process() {
    if (audio_pid > 0 && audio_player_started) {
        // Send QUIT command to gracefully stop mpg123
        dprintf(control_pipe[1], "QUIT\n");
        close(control_pipe[1]); // Close write end of the pipe
        waitpid(audio_pid, NULL, 0);
        audio_pid = -1;
    }
}

// void init_mpg123(t_main *main) {
//     if (pipe(control_pipe) == -1) {
//         perror("pipe failed");
//         exit(1);
//     }

//     pid_t pid = fork();

//     if (pid < 0) {
//         perror("fork failed");
//         exit(1);
//     } else if (pid == 0) { // Child process
//         int devnull = open("/dev/null", O_WRONLY);
//         if (devnull != -1) {
//             dup2(devnull, STDOUT_FILENO);
//             dup2(devnull, STDERR_FILENO);
//             close(devnull);
//         }

//         setsid();

//         // Redirect control_pipe read end to child's stdin
//         dup2(control_pipe[0], STDIN_FILENO);
//         close(control_pipe[1]); // Close write end

//         execlp("mpg123", "mpg123", "-R", NULL);
//         perror("exec failed");
//         exit(1);
//     }

//     audio_pid = pid;
//     close(control_pipe[0]); // Close read end in parent
//     atexit(kill_audio_process);

//     set_volume(MUSIC_VOLUME);
// }

void init_mpg123(t_main *main) {
    if (pipe(control_pipe) == -1 || pipe(status_pipe) == -1) {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) { // Child process
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDERR_FILENO); // Silence stderr
            close(devnull);
        }

        setsid();

        // Redirect control_pipe read end to child's stdin
        dup2(control_pipe[0], STDIN_FILENO);
        close(control_pipe[1]); // Close write end in child
        close(control_pipe[0]);

        // Redirect child's stdout to status_pipe so we can read its output
        dup2(status_pipe[1], STDOUT_FILENO);
        close(status_pipe[0]);
        close(status_pipe[1]);

        execlp("mpg123", "mpg123", "-R", NULL);

        perror("exec failed");
        // exit(1);
    }

    // Parent process
    audio_pid = pid;
    close(control_pipe[0]); // Parent writes, so close read end
    close(status_pipe[1]);  // Parent reads, so close write end

    // Wait for mpg123's startup message
    char buffer[256];
    ssize_t bytes_read = read(status_pipe[0], buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        if (strstr(buffer, "MPG123") == NULL) {
            fprintf(stderr, "mpg123 did not start properly: %s\n", buffer);
            main->audio_player_started = false;
            return;
        } else {
            printf("mpg123 started successfully!\n");
            main->audio_player_started = true;
            audio_player_started = true;
        }
    } else {
        printf("Failed to read from mpg123\n");
        main->audio_player_started = false;
        return;
    }

    // Cleanup on exit
    atexit(kill_audio_process);
}

void play_mp3(){
    if (audio_pid > 0) {
        // Check if file exists
        if (access("Berlin_Retro.mp3", F_OK) == 0) {
            // Send LOAD command to start playback
            dprintf(control_pipe[1], "LOAD Berlin_Retro.mp3\n");
            dprintf(control_pipe[1], "LOOP\n");
        } else if (access("Traffic/Berlin_Retro.mp3", F_OK) == 0) {
            dprintf(control_pipe[1], "LOAD Traffic/Berlin_Retro.mp3\n");
            dprintf(control_pipe[1], "LOOP\n");
        }
    }
}

void stop_mp3() {
    if (audio_pid > 0) {
        dprintf(control_pipe[1], "STOP\n");
    }
}

void set_volume(int volume) {
    if (audio_pid > 0) {
        dprintf(control_pipe[1], "VOLUME %d\n", volume);
    }
}