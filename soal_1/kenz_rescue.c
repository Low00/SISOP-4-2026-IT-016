#define FUSE_USE_VERSION 30

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

static char *source_dir;

static void fullpath(char fpath[PATH_MAX], const char *path) {
    strcpy(fpath, source_dir);
    strncat(fpath, path, PATH_MAX - strlen(source_dir) - 1);
}

/* BUILD tujuan.txt CONTENT */
void build_tujuan_content(char *content) {

    content[0] = '\0';

    char path[PATH_MAX];
    char line[1024];

    strcat(content, "Tujuan Mas Amba: ");

    for (int i = 1; i <= 7; i++) {

        snprintf(path, sizeof(path), "%s/%d.txt", source_dir, i);

        FILE *fp = fopen(path, "r");

        if (!fp)
            continue;

        while (fgets(line, sizeof(line), fp)) {

            if (strncmp(line, "KOORD:", 6) == 0) {

                char *p = line + 7;

                p[strcspn(p, "\n")] = 0;

                strcat(content, p);
            }
        }

        fclose(fp);
    }

    strcat(content, "\n");
}

/* GETATTR */
static int x_getattr(const char *path, struct stat *stbuf,
                     struct fuse_file_info *fi) {

    (void) fi;

    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    if (strcmp(path, "/tujuan.txt") == 0) {

        char content[8192];
        build_tujuan_content(content);

        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(content);

        return 0;
    }

    char fpath[PATH_MAX];
    fullpath(fpath, path);

    if (lstat(fpath, stbuf) == -1)
        return -errno;

    return 0;
}

/* READDIR */
static int x_readdir(const char *path, void *buf,
                     fuse_fill_dir_t filler,
                     off_t offset,
                     struct fuse_file_info *fi,
                     enum fuse_readdir_flags flags) {

    (void) offset;
    (void) fi;
    (void) flags;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    DIR *dp;
    struct dirent *de;

    dp = opendir(source_dir);

    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {

        if (de->d_name[0] == '.')
            continue;

        filler(buf, de->d_name, NULL, 0, 0);
    }

    closedir(dp);

    /* virtual file */
    filler(buf, "tujuan.txt", NULL, 0, 0);

    return 0;
}

/* OPEN */
static int x_open(const char *path, struct fuse_file_info *fi) {

    (void) fi;

    if (strcmp(path, "/tujuan.txt") == 0)
        return 0;

    char fpath[PATH_MAX];
    fullpath(fpath, path);

    int fd = open(fpath, O_RDONLY);

    if (fd == -1)
        return -errno;

    close(fd);

    return 0;
}

/* READ */
static int x_read(const char *path, char *buf,
                  size_t size, off_t offset,
                  struct fuse_file_info *fi) {

    (void) fi;

    /* virtual file */
    if (strcmp(path, "/tujuan.txt") == 0) {

        char content[8192];

        build_tujuan_content(content);

        size_t len = strlen(content);

        if (offset < len) {

            if (offset + size > len)
                size = len - offset;

            memcpy(buf, content + offset, size);

        } else {
            size = 0;
        }

        return size;
    }

    /* passthrough normal files */
    char fpath[PATH_MAX];

    fullpath(fpath, path);

    int fd = open(fpath, O_RDONLY);

    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);

    if (res == -1)
        res = -errno;

    close(fd);

    return res;
}

static struct fuse_operations operations = {
    .getattr = x_getattr,
    .readdir = x_readdir,
    .open    = x_open,
    .read    = x_read,
};

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: %s <source_dir> <mount_point>\n", argv[0]);
        return 1;
    }

    source_dir = realpath(argv[1], NULL);

    argv[1] = argv[2];
    argc--;

    return fuse_main(argc, argv, &operations, NULL);
}
