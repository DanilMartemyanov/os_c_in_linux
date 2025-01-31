#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>


typedef struct {
    char path[1024];
    char name[256];
    off_t size;
} FileInfo;

int compare_by_size(const void *a, const void *b) {
    FileInfo *fileA = (FileInfo *)a;
    FileInfo *fileB = (FileInfo *)b;
    return (fileA->size - fileB->size);
}

int compare_by_name(const void *a, const void *b) {
    FileInfo *fileA = (FileInfo *)a;
    FileInfo *fileB = (FileInfo *)b;
    return strcmp(fileA->name, fileB->name);
}

void sort_files_by_size(FileInfo *files, int count) {
    if (files == NULL) {
        printf("Ошибка: массив файлов пуст\n");
        return;
    }
    qsort(files, count, sizeof(FileInfo), compare_by_size);
}

void sort_files_by_name(FileInfo *files, int count) {
    if (files == NULL) {
        printf("массив файлов пуст\n");
        return;
    }
    qsort(files, count, sizeof(FileInfo), compare_by_name);
}

void list_files_recursive(const char *dir_path, FileInfo **files, int *count) {
    DIR *dir;
    struct dirent *el_dir;

    dir = opendir(dir_path);
    if (!dir) {
        printf("Не удалось открыть каталог '%s'\n", dir_path);
        return;
    }

    while ((el_dir = readdir(dir)) != NULL) {
        char full_path[1024];
        struct stat st;

        if (strcmp(el_dir->d_name, ".") == 0 || strcmp(el_dir->d_name, "..") == 0) {
            continue;
        }


        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, el_dir->d_name);


        if (stat(full_path, &st) == -1) {
            printf("Не удалось получить информацию о файле '%s'. Проверьте права доступа.\n", full_path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            list_files_recursive(full_path, files, count);
        }
        else if (S_ISREG(st.st_mode)) {
            *files = realloc(*files, (*count + 1) * sizeof(FileInfo));
            if (!*files) {
                printf("Не удалось выделить память\n");
                closedir(dir);
                exit(1);
            }

            strncpy((*files)[*count].path, full_path, sizeof((*files)[*count].path));
            strncpy((*files)[*count].name, el_dir->d_name, sizeof((*files)[*count].name));
            (*files)[*count].size = st.st_size;
            (*count)++;
        }
    }

    closedir(dir);
}

void copy_files(FileInfo *files, int count, const char *dest_dir) {
    for (int i = 0; i < count; i++) {
        char dest_path[1024];
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, files[i].name);

        FILE *src_file = fopen(files[i].path, "rb");
        if (!src_file) {
            printf("не удалось открыть файл '%s' для чтения. Проверьте права доступа.\n", files[i].path);
            continue;
        }

        FILE *dest_file = fopen(dest_path, "wb");
        if (!dest_file) {
            printf("не удалось открыть файл '%s' для записи. Проверьте права доступа.\n", dest_path);
            fclose(src_file);
            continue;
        }

        char buffer[1024];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
            fwrite(buffer, 1, bytes_read, dest_file);
        }

        fclose(src_file);
        fclose(dest_file);

        printf("Скопирован: %s, Имя: %s, Размер: %ld байт\n", files[i].path, files[i].name, files[i].size);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Введите корректные аргументы\n");
        return 1;
    }

    const char *source_dir = argv[1];
    int sort = atoi(argv[2]);
    const char *dest_dir = argv[3];

    if (sort != 1 && sort != 2) {
        printf("Выберите корректную сортировку\n");
        return 1;
    }

    FileInfo *files = NULL;
    int count = 0;

    list_files_recursive(source_dir, &files, &count);

    if (sort == 1) {
        sort_files_by_size(files, count);
    } else if (sort == 2) {
        sort_files_by_name(files, count);
    }

    if (mkdir(dest_dir, 0700) == -1) {
        printf("не удалось создать каталог '%s'.Проверьте права\n", dest_dir);
        free(files);
        return 1;
    }

    copy_files(files, count, dest_dir);

    free(files);

    return 0;
}