#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>

int directories = 0;
int files = 0;

void print_tree(const char *path, const char *prefix);

void print_permissions(mode_t mode) {
    printf((S_ISDIR(mode)) ? "d" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

void print_owner(uid_t uid) {
    printf("%s", getpwuid(uid)->pw_name);
}

void print_group(gid_t gid) {
    printf("%s", getgrgid(gid)->gr_name);
}

void print_size(off_t size) {
    printf("%ld", (long)size);
}

int size_of_directory(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    struct dirent *entry;
    int num_entries = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        num_entries++;
    }
    closedir(dir);

    return num_entries;
}

void print_tree_recursive(const char *path, const char *prefix, const char *name, int is_last) {

    struct stat st;
    if (lstat(path, &st) != 0) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }


    S_ISDIR(st.st_mode) ? directories++ : files++;

    printf("%s", prefix);
    if (is_last) {
        printf("└── ");
    } else {
        printf("├── ");
    }

    printf("[");
    
    print_permissions(st.st_mode);
    printf(" ");
    print_owner(st.st_uid);
    printf(" ");
    print_group(st.st_gid);
    printf("        ");
    print_size(st.st_size);

    printf("]");

    printf("  %s\n", name);

    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (dir == NULL) {
            perror("opendir");
            exit(EXIT_FAILURE);
        }
        struct dirent *entry;
        int count = 0;
        int num_entries = size_of_directory(path);
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') {
                continue;
            }
            count++;

            char child_path[1024];
            snprintf(child_path, sizeof(child_path), "%s/%s", path, entry->d_name);

            char new_prefix[1024];
            snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, (is_last ? "    " : "│   "));

            int is_last_entry = count == num_entries;

            print_tree_recursive(child_path, new_prefix, entry->d_name, is_last_entry);
        }
        closedir(dir);
    }
}

void first_tree_recursive(const char *path) {
    struct stat st;
    if (lstat(path, &st) != 0) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }
    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (dir == NULL) {
            perror("opendir");
            exit(EXIT_FAILURE);
        }
        struct dirent *entry;
        int count = 0;
        int num_entries = size_of_directory(path);
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') {
                continue;
            }
            count++;

            char child_path[1024];
            snprintf(child_path, sizeof(child_path), "%s/%s", path, entry->d_name);

            int is_last_entry = count == num_entries;

            print_tree_recursive(child_path, "", entry->d_name, is_last_entry);
        }
        closedir(dir);
    }
}


int main(int argc, char *argv[]) {
    const char *path;
    if(argc == 2) {
        path = argv[1];
    } else {
        path = ".";
    }

    printf("%s\n", path);

    first_tree_recursive(path);

    printf("\n%d directories, %d files\n", directories, files);

    return 0;
}
