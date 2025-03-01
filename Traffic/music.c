#include "main.h"
#include <signal.h>
#include <fcntl.h>

#include <sys/wait.h>

// Variable globale pour stocker le PID du processus enfant
pid_t audio_pid = -1;
int control_pipe[2];

void kill_audio_process() {
    if (audio_pid > 0) {
        // Send QUIT command to gracefully stop mpg123
        dprintf(control_pipe[1], "QUIT\n");
        close(control_pipe[1]); // Close write end of the pipe
        waitpid(audio_pid, NULL, 0);
        audio_pid = -1;
    }
}

void init_mpg123() {
    if (pipe(control_pipe) == -1) {
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
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        setsid();

        // Redirect control_pipe read end to child's stdin
        dup2(control_pipe[0], STDIN_FILENO);
        close(control_pipe[1]); // Close write end

        execlp("mpg123", "mpg123", "-R", NULL);
        perror("exec failed");
        exit(1);
    }

    audio_pid = pid;
    close(control_pipe[0]); // Close read end in parent
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