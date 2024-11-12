#include <pthread.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH_LEN 1024
#define MAX_FILES 1000


typedef struct {
    char path[MAX_PATH_LEN];
} FilePath;


FilePath *file_list;
int file_count = 0;


pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;


void explore_directory(const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, MAX_PATH_LEN, "%s/%s", dir_path, entry->d_name);

      
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

      
        if (entry->d_type == DT_REG) {
            pthread_mutex_lock(&list_mutex);
            if (file_count < MAX_FILES) {
                strcpy(file_list[file_count].path, full_path);
                file_count++;
            }
            pthread_mutex_unlock(&list_mutex);
        }

       
        if (entry->d_type == DT_DIR) {
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, (void*)explore_directory, (void*)full_path);
            pthread_detach(thread_id); 
        }
    }

    closedir(dir);
}


void display_results() {
    pthread_mutex_lock(&list_mutex);
    printf("Fichiers trouvés :\n");
    for (int i = 0; i < file_count; i++) {
        printf("%s\n", file_list[i].path);
    }
    pthread_mutex_unlock(&list_mutex);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <répertoire>\n", argv[0]);
        return 1;
    }


    file_list = (FilePath *)malloc(sizeof(FilePath) * MAX_FILES);

    
    explore_directory(argv[1]);

    
    sleep(5);
    display_results();

   
    free(file_list);
    return 0;
}
