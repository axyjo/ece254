/**
 * ECE 254 Lab 1
 * @author Akshay Joshi <a24joshi@uwaterloo.ca>
 * @date 2014-09-09
 */

#include <stdio.h>

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

    printf(argv[0]);
    return 0;
}
