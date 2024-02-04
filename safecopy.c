#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <libgen.h> 

#include <sys/inotify.h>

#include <libnotify/notify.h>

#define EXT_SUCCESS 0
#define EXT_ERR_TOO_FEW_ARGS 1
#define EXT_ERR_INIT_INOTIFY 2
#define EXT_ERR_ADD_WATCH 3
#define EXT_ERR_BASE_PATH_NULL 4
#define EXT_ERR_READ_INOTIFY 5
#define EXT_ERR_INIT_LIBNOTIFY 6

int IeventQueue = -1;
int IeventStatus = -1;

char *ProgramTitle = "safecopy";

void signal_handler(int signal) {
    int closeStatus = -1;

    printf("Signal received, cleaning up... \n");
    closeStatus = inotify_rm_watch(IeventQueue, IeventStatus);
    if (closeStatus == -1) {
        fprintf(stderr, "Error removing from watch queue\n");
    }
    close(IeventQueue);
    exit(EXT_SUCCESS);
}

void create_backup(const char *source_path, const char *backup_path) {
    FILE *source_file = fopen(source_path, "r");
    FILE *backup_file = fopen(backup_path, "w");

    if (source_file == NULL || backup_file == NULL) {
        perror("Error opening files for backup");
        exit(EXIT_FAILURE);
    }

    int ch;
    while ((ch = fgetc(source_file)) != EOF) {
        fputc(ch, backup_file);
    }

    fclose(source_file);
    fclose(backup_file);

    printf("Backup created: %s\n", backup_path);
}

int main(int argc, char **argv) {
    bool libnotifyInitStatus = false;

    char *basePath = NULL;
    char *token = NULL;
    char *notificationMessage = NULL;

    NotifyNotification *notifyHandle;

    char buffer[4096];
    int readLength;

    const struct inotify_event *watchEvent;

    const uint32_t watchMask = IN_ACCESS | IN_CLOSE_WRITE | IN_MODIFY;

    if (argc < 2) {
        fprintf(stderr, "usage: safecopy PATH\n");
        exit(EXT_ERR_TOO_FEW_ARGS);
    }

    basePath = (char *)malloc(sizeof(char) * (strlen(argv[1]) + 1));
    strcpy(basePath, argv[1]);

    token = strtok(basePath, "/");
    while (token != NULL) {
        basePath = token;
        token = strtok(NULL, "/");
    }

    if (basePath == NULL) {
        fprintf(stderr, "Error getting base path \n");
        exit(EXT_ERR_BASE_PATH_NULL);
    }

    libnotifyInitStatus = notify_init(ProgramTitle);
    if (!libnotifyInitStatus) {
        fprintf(stderr, "Error initializing libnotify\n");
        exit(EXT_ERR_INIT_LIBNOTIFY);
    }

    IeventQueue = inotify_init();
    if (IeventQueue == -1) {
        fprintf(stderr, "Failed to initialize inotify instance \n");
        exit(EXT_ERR_INIT_INOTIFY);
    }

    IeventStatus = inotify_add_watch(IeventQueue, argv[1], watchMask);
    if (IeventStatus == -1) {
        fprintf(stderr, "Failed to start watch instance \n");
        exit(EXT_ERR_ADD_WATCH);
    }

    signal(SIGABRT, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    while (true) {
        printf("waiting for ievent...\n");

        readLength = read(IeventQueue, buffer, sizeof(buffer));
        if (readLength == -1) {
            fprintf(stderr, "Error reading from inotify event");
            exit(EXT_ERR_READ_INOTIFY);
        }

        for (char *buffPointer = buffer; buffPointer < buffer + readLength; buffPointer += sizeof(struct inotify_event) + watchEvent->len) {
            notificationMessage = NULL;
            watchEvent = (const struct inotify_event *)buffPointer;

            if (watchEvent->mask & IN_ACCESS) {
                notificationMessage = "File Accessed";
            }

            if (watchEvent->mask & IN_CLOSE_WRITE) {
                notificationMessage = "File Closed After Writing";
            }

            if (watchEvent->mask & IN_MODIFY) {
                notificationMessage = "File Modified";

                // Extract the directory part of the path
                char dir_path[4096];
                strcpy(dir_path, argv[1]);
                char *dir_name = dirname(dir_path);

                // Get the full path of the backup file
                char backup_path[4096];
                snprintf(backup_path, sizeof(backup_path), "%s/%s.bak", dir_name, basePath);

                // Add backup creation logic before the modification event
                create_backup(argv[1], backup_path);
            }

            if (notificationMessage == NULL) {
                continue;
            }

            notifyHandle = notify_notification_new(basePath, notificationMessage, "dialog-information");
            if (notifyHandle == NULL) {
                fprintf(stderr, "Notification handle was null!\n");
                continue;
            }

            notify_notification_show(notifyHandle, NULL);
        }
    }
}
