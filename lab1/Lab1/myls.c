/**
 * ECE 254 Lab 1
 * @author Akshay Budhkar <abudhkar@uwaterloo.ca>
 * @author Akshay Joshi <a24joshi@uwaterloo.ca>
 * @date 2014-09-09
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <grp.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>

int dir_option = 0;

typedef struct myls_struct {
    char *perms;
    char *owner;
    char *group;
    off_t size;
    char *datetime;
    char *filename;
} myls_struct;

char* perms(struct stat *lstat_r) {
    char *result = malloc(10 * sizeof(char));
    strcpy(result, "----------");

    switch (lstat_r->st_mode & S_IFMT) {
        case S_IFREG:
            break;
        case S_IFDIR:
            result[0] = 'd';
            break;
        case S_IFLNK:
            result[0] = 'l';
            break;
        default:
            fputs("Could not determine file type", stderr);
            exit(1);
    }

    result[1] = (lstat_r->st_mode & S_IRUSR) ? 'r' : '-';
    result[2] = (lstat_r->st_mode & S_IWUSR) ? 'w' : '-';
    result[3] = (lstat_r->st_mode & S_IXUSR) ? 'x' : '-'; 
    result[4] = (lstat_r->st_mode & S_IRGRP) ? 'r' : '-';
    result[5] = (lstat_r->st_mode & S_IWGRP) ? 'w' : '-';
    result[6] = (lstat_r->st_mode & S_IXGRP) ? 'x' : '-';
    result[7] = (lstat_r->st_mode & S_IROTH) ? 'r' : '-';
    result[8] = (lstat_r->st_mode & S_IWOTH) ? 'w' : '-';
    result[9] = (lstat_r->st_mode & S_IXOTH) ? 'x' : '-';

    return result;
}

char* owner(struct stat *lstat_r) {
    struct passwd *usr = getpwuid(lstat_r->st_uid);
    return usr->pw_name;
}

char* group(struct stat *lstat_r) {
    struct group *grp = getgrgid(lstat_r->st_gid);
    return grp->gr_name;
}

off_t size(struct stat *lstat_r) {
    return lstat_r->st_size;
}

char* datetime(struct stat *lstat_r) {
    char *value = malloc(13 * sizeof(char));

    time_t file_time;
    time_t real_time = time(NULL);
    
    struct tm broken_real_time;
    struct tm broken_file_time;

    if (dir_option == 1) {
        file_time = lstat_r->st_atime;
    } else if (dir_option == 2) {
        file_time = lstat_r->st_ctime;
    } else if (dir_option == 4) {
        file_time = lstat_r->st_mtime;
    } else {
        fputs("DIE A HORRIBLE DEATH.\n", stderr);
        exit(1);
    }

    localtime_r(&file_time, &broken_file_time);
    int file_year = broken_file_time.tm_year;
    
    localtime_r(&real_time, &broken_real_time);
    int real_year = broken_real_time.tm_year;

    if (real_year == file_year) {
        strftime(value, 13, "%b %e %H:%M", &broken_file_time);
    } else {
        strftime(value, 13, "%b %e  %Y", &broken_file_time);
    }

    return value;
}

char* filename(struct stat *lstat_r, char *path) {
    int val = PATH_MAX * sizeof(char);
    char *value = malloc(2 * val);
    char real_path[val];

    strcat(value, path);

    if (S_ISLNK(lstat_r->st_mode)) {
        strcat(value, " -> ");
        readlink(path, real_path, val);
        strcat(value, real_path);
    }

    return value;
}

myls_struct* getstruct(char *path) {
    struct stat lstat_r;
    if (lstat(path, &lstat_r) != 0) {
        fputs("Could not stat file ", stderr);
        fputs(path, stderr);
    }

    struct myls_struct *str= malloc(sizeof(myls_struct));
    str->perms = perms(&lstat_r);
    str->owner = owner(&lstat_r);
    str->group = group(&lstat_r);
    str->size = size(&lstat_r);
    str->datetime = datetime(&lstat_r);
    str->filename = filename(&lstat_r, path);

    return str;
}


void fmt(struct dirent *p_dirent) {
    struct myls_struct *cols = getstruct(p_dirent->d_name);

    printf("%s\t%s\t%s\t%d\t%s\t%s\n", cols->perms, cols->owner, cols->group, (int)cols->size, cols->datetime, cols->filename);
    free(cols);
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
        dir_option = 1;
    } else if (strcmp(option, "-c") == 0) {
        dir_option = 2;
    } else if (strcmp(option, "-l") == 0) {
        dir_option = 4;
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

    if (chdir(directory) != 0) {
        fputs("failed to change directory '", stderr);
        fputs(directory, stderr);
        fputs("'\n", stderr);
        return 1;
    }

    struct dirent *directory_entry;
    while ((directory_entry = readdir(directory_h)) != NULL) {
        fmt(directory_entry);
    }

    return 0;
}

