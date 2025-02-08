#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>

#define MAX_PATH 1024


typedef struct {
  FILE *file;
  const char *file_name;
} FileInfo;

void compareFiles(FileInfo *file1_info, FileInfo *file2_info) {
  FILE *file1 = file1_info->file;
  FILE *file2 = file2_info->file;
  const char *file_name1 = file1_info->file_name;
  const char *file_name2 = file2_info->file_name;

  long ch1, ch2, count_bytes = 0;
  int result = 0;

  while ((ch1 = fgetc(file1)) != EOF && (ch2 = fgetc(file2)) != EOF) {
    count_bytes++;
    if (ch1 != ch2) {
      result = 1;
      break;
    }
  }

  printf("PID=%d,файл1:%s и файл2%s, ", getpid(), file_name1, file_name2);
  printf("байт: %ld ", count_bytes);
  printf("результат: %d ", result);


  fclose(file1);
  fclose(file2);
}

void processFilePair(const char *file1, const char *file2) {
  FileInfo file1_info = { fopen(file1, "r"), file1 };
  FileInfo file2_info = { fopen(file2, "r"), file2 };

  if (file1_info.file == NULL || file2_info.file == NULL) {
    perror("Ошибка открытия файла");
    return;
  }

  compareFiles(&file1_info, &file2_info);

}

void traverseAndCompare(const char *dir1, const char *dir2, int max_processes) {
  DIR *dp1 = opendir(dir1);
  DIR *dp2 = opendir(dir2);
  struct dirent *entry1, *entry2;
  char path1[MAX_PATH], path2[MAX_PATH];
  pid_t pid;
  int active_processes = 0;

  if (dp1 == NULL || dp2 == NULL) {
    perror("Ошибка открытия каталога");
    return;
  }

  while ((entry1 = readdir(dp1)) != NULL) {
    if (entry1->d_type == DT_REG) {
      snprintf(path1, MAX_PATH, "%s/%s", dir1, entry1->d_name);

      rewinddir(dp2);
      while ((entry2 = readdir(dp2)) != NULL) {
        if (entry2->d_type == DT_REG) {
          snprintf(path2, MAX_PATH, "%s/%s", dir2, entry2->d_name);

          if (active_processes >= max_processes) {
            wait(NULL);
            active_processes--;
          }

          pid = fork();
          if (pid == 0) {
            processFilePair(path1, path2);
            return;
          } else if (pid > 0) {
            active_processes++;
            printf("Количество процессов:%d\n", active_processes);
          } else {
            perror("Ошибка при создании процесса");
            return;
          }
        }
      }
    }
  }

  while (active_processes > 0) {
    wait(NULL);
    printf("Завершение процесса.");
    active_processes--;
    printf("Количество процессов:%d\n", active_processes);
  }

  closedir(dp1);
  closedir(dp2);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    return 1;
  }

  int n = atoi(argv[1]);
  const char *dir1 = argv[2];
  const char *dir2 = argv[3];

  traverseAndCompare(dir1, dir2, n);

  return 0;
}