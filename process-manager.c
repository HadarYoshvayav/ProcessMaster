#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <dirent.h>
#endif

void log_action(const char *action);

void print_colored(const char *text, const char *color) {
#ifdef _WIN32
    printf("%s\n", text);
#else
    printf("%s%s\033[0m", color, text);
#endif
}

void list_processes() {
#ifdef _WIN32
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_PROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);

    printf("\n");
    print_colored("PID\tCMD\n", "\033[1;34m"); 
    printf("--------------------------------------\n");

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            printf("%d\t%s\n", pe32.th32ProcessID, pe32.szExeFile);
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);
#elif defined(__linux__) || defined(__APPLE__)
    struct dirent *entry;
    DIR *proc = opendir("/proc");

    if (!proc) {
        perror("Cannot open /proc");
        exit(1);
    }

    printf("\n");
    print_colored("PID\tCMD\n", "\033[1;34m"); 
    printf("--------------------------------------\n");

    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            char path[256], cmdline[256];
            sprintf(path, "/proc/%s/cmdline", entry->d_name);

            FILE *cmd_file = fopen(path, "r");
            if (cmd_file) {
                fgets(cmdline, sizeof(cmdline), cmd_file);
                fclose(cmd_file);

                printf("%s%d\t%s\n", "\033[1;32m", atoi(entry->d_name), strlen(cmdline) > 0 ? cmdline : "[No command]");
            }
        }
    }

    closedir(proc);
#endif
}

void kill_process(int pid) {
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess) {
        if (TerminateProcess(hProcess, 0)) {
            print_colored("Process killed successfully.\n", "\033[1;32m"); // Green color
            log_action("Killed process");
        } else {
            perror("Failed to kill process");
        }
        CloseHandle(hProcess);
    } else {
        perror("Failed to open process");
    }
#else
    if (kill(pid, SIGKILL) == 0) {
        print_colored("Process killed successfully.\n", "\033[1;32m"); // Green color
        log_action("Killed process");
    } else {
        perror("Failed to kill process");
    }
#endif
}

void suspend_process(int pid) {
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (hProcess) {
        SuspendThread(hProcess);
        print_colored("Process suspended successfully.\n", "\033[1;32m"); // Green color
        log_action("Suspended process");
        CloseHandle(hProcess);
    } else {
        perror("Failed to open process");
    }
#else
    if (kill(pid, SIGSTOP) == 0) {
        print_colored("Process suspended successfully.\n", "\033[1;32m"); // Green color
        log_action("Suspended process");
    } else {
        perror("Failed to suspend process");
    }
#endif
}

void resume_process(int pid) {
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (hProcess) {
        ResumeThread(hProcess);
        print_colored("Process resumed successfully.\n", "\033[1;32m"); 
        log_action("Resumed process");
        CloseHandle(hProcess);
    } else {
        perror("Failed to open process");
    }
#else
    if (kill(pid, SIGCONT) == 0) {
        print_colored("Process resumed successfully.\n", "\033[1;32m"); 
        log_action("Resumed process");
    } else {
        perror("Failed to resume process");
    }
#endif
}

void show_process_details(int pid) {
#ifdef _WIN32
    printf("Details for PID %d not implemented on Windows.\n", pid);
#else
    char path[256], buffer[256];
    sprintf(path, "/proc/%d/status", pid);
    FILE *status_file = fopen(path, "r");
    
    if (status_file) {
        printf("Details for PID %d:\n", pid);
        while (fgets(buffer, sizeof(buffer), status_file)) {
            printf("%s", buffer);
        }
        fclose(status_file);
    } else {
        perror("Failed to open process status file");
    }
#endif
}

void start_process(const char *cmd) {
#ifdef _WIN32
    system(cmd); 
#else
    if (fork() == 0) {
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        perror("Failed to start process");
        exit(1);
    }
#endif
}

void log_action(const char *action) {
    FILE *log_file = fopen("process_manager.log", "a");
    if (log_file) {
        fprintf(log_file, "%s: %s\n", action, ctime(&(time_t){time(NULL)}));
        fclose(log_file);
    } else {
        perror("Failed to open log file");
    }
}

void export_processes_to_csv() {
    FILE *csv_file = fopen("processes.csv", "w");
    if (!csv_file) {
        perror("Failed to open CSV file");
        return;
    }

    fprintf(csv_file, "PID,CMD\n");
#ifdef _WIN32
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_PROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            fprintf(csv_file, "%d,%s\n", pe32.th32ProcessID, pe32.szExeFile);
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);
#else
    struct dirent *entry;
    DIR *proc = opendir("/proc");
    
    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            char path[256], cmdline[256];
            sprintf(path, "/proc/%s/cmdline", entry->d_name);

            FILE *cmd_file = fopen(path, "r");
            if (cmd_file) {
                fgets(cmdline, sizeof(cmdline), cmd_file);
                fclose(cmd_file);
                fprintf(csv_file, "%d,%s\n", atoi(entry->d_name), strlen(cmdline) > 0 ? cmdline : "[No command]");
            }
        }
    }

    closedir(proc);
#endif

    fclose(csv_file);
    print_colored("Processes exported to processes.csv\n", "\033[1;32m");
}

void search_process(int pid) {
#ifdef _WIN32
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_PROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    while (Process32Next(hProcessSnap, &pe32)) {
        if (pe32.th32ProcessID == pid) {
            printf("Found process: PID %d, CMD: %s\n", pid, pe32.szExeFile);
            break;
        }
    }
    CloseHandle(hProcessSnap);
#else
    struct dirent *entry;
    DIR *proc = opendir("/proc");
    
    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) == pid) {
            char path[256], cmdline[256];
            sprintf(path, "/proc/%s/cmdline", entry->d_name);

            FILE *cmd_file = fopen(path, "r");
            if (cmd_file) {
                fgets(cmdline, sizeof(cmdline), cmd_file);
                fclose(cmd_file);
                printf("Found process: PID %d, CMD: %s\n", pid, strlen(cmdline) > 0 ? cmdline : "[No command]");
            }
            break;
        }
    }

    closedir(proc);
#endif
}

int main() {
    int choice, pid;
    char cmd[256];

    while (1) {
        printf("\nProcess Manager\n");
        printf("1. List processes\n");
        printf("2. Kill a process\n");
        printf("3. Suspend a process\n");
        printf("4. Resume a process\n");
        printf("5. Show process details\n");
        printf("6. Start a new process\n");
        printf("7. Export processes to CSV\n");
        printf("8. Search for a process\n");
        printf("9. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                list_processes();
                break;
            case 2:
                printf("Enter PID to kill: ");
                scanf("%d", &pid);
                kill_process(pid);
                break;
            case 3:
                printf("Enter PID to suspend: ");
                scanf("%d", &pid);
                suspend_process(pid);
                break;
            case 4:
                printf("Enter PID to resume: ");
                scanf("%d", &pid);
                resume_process(pid);
                break;
            case 5:
                printf("Enter PID to show details: ");
                scanf("%d", &pid);
                show_process_details(pid);
                break;
            case 6:
                printf("Enter command to start a new process: ");
                scanf(" %[^\n]", cmd);
                start_process(cmd);
                break;
            case 7:
                export_processes_to_csv();
                break;
            case 8:
                printf("Enter PID to search: ");
                scanf("%d", &pid);
                search_process(pid);
                break;
            case 9:
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
