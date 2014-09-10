/**
 * ECE 254 Lab 1
 * @author Akshay Joshi <a24joshi@uwaterloo.ca>
 * @date 2014-09-09
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

typedef struct myls_struct {
    char perms[10];
} myls_struct;

char* perms(char *path) {
    char *result = malloc(10 * sizeof(char));
    strcpy(result, "----------");
    struct stat lstat_r;
    lstat(path, &lstat_r);

    switch (lstat_r.st_mode & S_IFMT) {
        case S_IFREG:
            break;
        case S_IFDIR:
            result[0] = 'd';
            break;
        case S_IFLNK:
            result[0] = 'l';
            break;
        default:
            fputs("Could not determine file type for ", stderr);
            fputs(path, stderr);
            printf("%d", lstat_r.st_mode);
            fputs("\n", stderr);
            exit(1);
    }

    return result;
}

myls_struct* getstruct(char *full_path) {
    struct myls_struct *str= malloc(sizeof(myls_struct));
    strncpy(str->perms, perms(full_path), 10);

    return str;
}

void fmt(char *directory, struct dirent *p_dirent) {
     printf("Directory: %s\n", directory);
    //printf("%s %s\n", directory, p_dirent->d_name);
    char *full_path = malloc(PATH_MAX * sizeof(char));
    strcat(full_path, directory);
    strcat(full_path, "/");
    strcat(full_path, p_dirent->d_name);


    printf("%s\n", full_path);

    free(full_path);
    
//    struct myls_struct *cols = getstruct(full_path);
//    printf("%s %s\n", cols->perms, p_dirent->d_name);
//    free(cols);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fputs("ERROR: incorrect number of arguments\n", stderr);
        fputs("usage: ./myls OPTION DIRECTORY\n", stderr);
        return 1;
    }

    char *option = argv[1];
    char *directory = argv[2];

    if (strcmp(option, "-u") == 0) {
    } else if (strcmp(option, "-c") == 0) {
    } else if (strcmp(option, "-l") == 0) {
    } else {
        fputs("ERROR: invalid argument for OPTION\n", stderr);
        fputs("Valid options: -u, -c, -l\n", stderr);
        return 1;
    }

    DIR *directory_h;
    if ((directory_h = opendir(directory)) == NULL) {
        fputs("failed to open directory '", stderr);
        fputs(directory, stderr);
        fputs("'\n", stderr);
        return 1;
    }

    struct dirent *directory_entry;
    while ((directory_entry = readdir(directory_h)) != NULL) {
        fmt(directory, directory_entry);
    }

    return 0;
}

