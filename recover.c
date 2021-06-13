#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "zfsimpl.h"
#include "stand.h"

struct zfsmount {
	const spa_t	*spa;
	objset_phys_t	objset;
	uint64_t	rootobj;
};

extern int vdev_probe(vdev_phys_read_t *, vdev_phys_write_t *, void *,
    spa_t **);
extern void zfs_init(void);
extern int zfs_lookup_dataset(const spa_t *, const char *, uint64_t *);
extern int zfs_mount(const spa_t *, uint64_t, struct zfsmount *);
extern int zfs_open(const char *, struct open_file *);
extern int zfs_stat(struct open_file *, struct stat *);
extern int zfs_read(struct open_file *, void *, size_t, size_t *);

int
my_phys_read(struct vdev *v, void *priv, off_t offset, void *buf, size_t psize)
{
	ssize_t n;
	int fd = (int)(uintptr_t)priv;

	n = pread(fd, buf, psize, offset);
	return (n);
}

int
my_phys_write(struct vdev *v, off_t offset, void *buf, size_t psize)
{
	return (0);
}

int
main(int argc, char **argv)
{
	const char *device = NULL;
	const char *dataset = NULL;
	const char *file = NULL;
	const char *out = NULL;
	FILE *outf = NULL;
	spa_t *spa = NULL;
	int devfd = -1;
	int ret;
	uint64_t objset_id;
	struct zfsmount mnt = { 0 };
	struct open_file f = { 0 };
	struct stat sb = { 0 };
	uint8_t buf[1024];
	size_t n;

	if (argc < 5) {
		fprintf(stderr, "Usage: %s dev dataset file outfile\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	device = argv[1];
	dataset = argv[2];
	file = argv[3];
	out = argv[4];

	if ((devfd = open(device, O_RDONLY)) == -1)
		err(EXIT_FAILURE, "%s", argv[1]);

	if ((outf = fopen(out, "w+")) == NULL)
		err(EXIT_FAILURE, "%s", argv[4]);

	zfs_init();
	ret = vdev_probe(my_phys_read, my_phys_write, (void *)(uintptr_t)devfd,
	    &spa);
	if (ret != 0)
		errx(EXIT_FAILURE, "vdev_probe failed");
	
	ret = zfs_lookup_dataset(spa, dataset, &objset_id);
	if (ret != 0)
		errx(EXIT_FAILURE, "zfs_lookup_dataset(%s) failed", dataset);

	printf("dataset %s id is %" PRIu64 "\n", dataset, objset_id);

	ret = zfs_mount(spa, objset_id, &mnt);
	if (ret != 0)
		errx(EXIT_FAILURE, "zfs_mount(%s) failed", dataset);

	f.f_devdata = &mnt;
	ret = zfs_open(file, &f);
	if (ret != 0)
		errx(EXIT_FAILURE, "zfs_open(%s) failed", file);

	ret = zfs_stat(&f, &sb);
	if (ret != 0)
		errx(EXIT_FAILURE, "zfs_stat failed");

	n = 0;
	while (n < sb.st_size) {
		size_t amt = sizeof (buf);

		if (n + amt > sb.st_size)
			amt = sb.st_size - n;

		ret = zfs_read(&f, buf, amt, NULL);
		if (ret != 0)
			errx(EXIT_FAILURE, "zfs_read failed");

		fwrite(buf, amt, 1, outf);
		if (ferror(outf))
			err(EXIT_FAILURE, "write failed");

		n += amt;
	}

	fclose(outf);
	close(devfd);
	return (0);
}
