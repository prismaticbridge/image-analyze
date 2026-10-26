#include <filemanager.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

void expand_tilde(const char *input, char *output, size_t size) {
    if (input[0] == '~') {
        const char *home = getenv("HOME");
        if (!home) home = "";  // fallback

        snprintf(output, size, "%s%s", home, input + 1);
    } else {
        snprintf(output, size, "%s", input);
    }
}

static int strcasestr_simple(const char *haystack, const char *needle) {
    // Simple portable case-insensitive substring search
    size_t nlen = strlen(needle);
    size_t hlen = strlen(haystack);

    for (size_t i = 0; i + nlen <= hlen; i++) {
        if (strncasecmp(&haystack[i], needle, nlen) == 0)
            return 1;
    }
    return 0;
}

static int has_image_extension(const char *name) {
    const char *ext = strrchr(name, '.');
    if (!ext) return 0;

    return !strcasecmp(ext, ".png") ||
           !strcasecmp(ext, ".jpg") ||
           !strcasecmp(ext, ".jpeg") ||
           !strcasecmp(ext, ".heic") ||
           !strcasecmp(ext, ".heif");
}

int find_latest_screenshot(char *buffer, const char *directory) {
    buffer[0] = '\0';  // empty output first

    DIR *dir = opendir(directory);
    if (!dir) return -1;

    struct dirent *entry;
    struct stat st;
    char path[PATH_MAX];

    time_t latest_mtime = 0;
    char latest_path[PATH_MAX] = {0};

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        if (!strcasestr_simple(entry->d_name, "screenshot"))
            continue;

        if (!has_image_extension(entry->d_name))
            continue;

        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        if (stat(path, &st) == -1)
            continue;

        if (S_ISREG(st.st_mode) && st.st_mtime > latest_mtime) {
            latest_mtime = st.st_mtime;
            strncpy(latest_path, path, sizeof(latest_path) - 1);
        }
    }

    closedir(dir);

    if (latest_mtime == 0)
        return 1;   // No screenshot files found

    strcpy(buffer, latest_path);
    return 0;       // Success
}

static int compare_paths(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

char **list_image_files(const char *filepath, int *count) {
    *count = 0;

    char directory[PATH_MAX];
    snprintf(directory, sizeof(directory), "%s", filepath);

    char *slash = strrchr(directory, '/');
    if (slash == NULL) {
        strcpy(directory, ".");
    } else if (slash == directory) {
        slash[1] = '\0';  // keep root "/"
    } else {
        *slash = '\0';
    }

    DIR *dir = opendir(directory);
    if (dir == NULL)
        return NULL;

    char **paths = NULL;
    int capacity = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        if (!has_image_extension(entry->d_name))
            continue;

        char path[PATH_MAX];
        if (!strcmp(directory, "."))
            snprintf(path, sizeof(path), "%s", entry->d_name);
        else
            snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        struct stat st;
        if (stat(path, &st) == -1 || !S_ISREG(st.st_mode))
            continue;

        if (*count == capacity) {
            capacity = (capacity == 0) ? 16 : capacity * 2;
            paths = realloc(paths, (size_t)capacity * sizeof(char *));
        }
        paths[*count] = strdup(path);
        (*count)++;
    }

    closedir(dir);

    if (*count > 1)
        qsort(paths, (size_t)*count, sizeof(char *), compare_paths);

    return paths;
}

void free_image_files(char **paths, int count) {
    for (int i = 0; i < count; i++)
        free(paths[i]);
    free(paths);
}

