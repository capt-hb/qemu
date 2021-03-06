/*
 * 9p utilities
 *
 * Copyright IBM, Corp. 2017
 *
 * Authors:
 *  Greg Kurz <groug@kaod.org>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#ifndef QEMU_9P_UTIL_H
#define QEMU_9P_UTIL_H

#ifdef O_PATH
#define O_PATH_9P_UTIL O_PATH
#else
#define O_PATH_9P_UTIL 0
#endif

#ifdef CONFIG_DARWIN
#define qemu_fgetxattr(...) fgetxattr(__VA_ARGS__, 0, 0)
#define qemu_lgetxattr(...) getxattr(__VA_ARGS__, 0, XATTR_NOFOLLOW)
#define qemu_llistxattr(...) listxattr(__VA_ARGS__, XATTR_NOFOLLOW)
#define qemu_lremovexattr(...) removexattr(__VA_ARGS__, XATTR_NOFOLLOW)
static inline int qemu_lsetxattr(const char *path, const char *name,
                                 const void *value, size_t size, int flags) {
    return setxattr(path, name, value, size, 0, flags | XATTR_NOFOLLOW);
}
#else
#define qemu_fgetxattr fgetxattr
#define qemu_lgetxattr lgetxattr
#define qemu_llistxattr llistxattr
#define qemu_lremovexattr lremovexattr
#define qemu_lsetxattr lsetxattr
#endif

/* Compatibility with old SDK Versions for Darwin */
#if defined(CONFIG_DARWIN) && !defined(UTIME_NOW)
#define UTIME_NOW -1
#define UTIME_OMIT -2
#endif

static inline void close_preserve_errno(int fd)
{
    int serrno = errno;
    close(fd);
    errno = serrno;
}

static inline int openat_dir(int dirfd, const char *name)
{
    return openat(dirfd, name,
                  O_DIRECTORY | O_RDONLY | O_NOFOLLOW | O_PATH_9P_UTIL);
}

static inline int openat_file(int dirfd, const char *name, int flags,
                              mode_t mode)
{
    int fd, serrno, ret;

    fd = openat(dirfd, name, flags | O_NOFOLLOW | O_NOCTTY | O_NONBLOCK,
                mode);
    if (fd == -1) {
        return -1;
    }

    serrno = errno;
    /* O_NONBLOCK was only needed to open the file. Let's drop it. We don't
     * do that with O_PATH since fcntl(F_SETFL) isn't supported, and openat()
     * ignored it anyway.
     */
    if (!(flags & O_PATH_9P_UTIL)) {
        ret = fcntl(fd, F_SETFL, flags);
        assert(!ret);
    }
    errno = serrno;
    return fd;
}

ssize_t fgetxattrat_nofollow(int dirfd, const char *path, const char *name,
                             void *value, size_t size);
int fsetxattrat_nofollow(int dirfd, const char *path, const char *name,
                         void *value, size_t size, int flags);
ssize_t flistxattrat_nofollow(int dirfd, const char *filename,
                              char *list, size_t size);
ssize_t fremovexattrat_nofollow(int dirfd, const char *filename,
                                const char *name);
int utimensat_nofollow(int dirfd, const char *filename,
                       const struct timespec times[2]);

int qemu_mknodat(int dirfd, const char *filename, mode_t mode, dev_t dev);

#endif
