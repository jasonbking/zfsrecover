#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

const char ntfs[] = "NTFS    ";
const char serial[] = "\x19\xf7\xf4\x76\x15\xf5\x76\x56";

int lzjb_decompress(void *s_start, void *d_start, size_t s_len __unused,
    size_t d_len, int n __unused);
int is_match(const char *buf, size_t buflen);
void *zalloc(size_t len);

int
main(int argc, char **argv)
{
	FILE *dev = NULL;
	char *inbuf = NULL;
	char *debuf = NULL;
	off_t off = 0;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s dev\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	inbuf = zalloc(128 * 1024);
	debuf = zalloc(4 * 128 * 1024);

	dev = fopen(argv[1], "r");
	if (dev == NULL)
		err(EXIT_FAILURE, "%s", argv[1]);

	for (off = 0;;off += 128 * 1024) {
		ssize_t n;
		int ret;

		n = fread(inbuf, 128 * 1024, 1, dev);
		if (n < 0)
			break;

		if (is_match(inbuf, 128 * 1024)) {
			printf("Potential match at 0x%x\n", off);
			continue;
		}

		(void) memset(debuf, 0, 4 * 128 * 1024);
		if (lzjb_decompress(inbuf, debuf, 128 * 1024,
		    4 * 128 * 1024, 0) < 0) {
			continue;
		}

		if (is_match(debuf, 4 * 128 * 1024)) {
			printf("Potential match at 0x%x\n", off);
		}
	}

	fclose(dev);
	return (0);
}

int
is_match(const char *buf, size_t buflen)
{
	if (memcmp(buf + 0x7e00, ntfs, 8) != 0)
		return (0);
	if (memcmp(buf + 0x7e48, serial, 8) != 0)
		return (0);
	return (1);
}

void *
zalloc(size_t len)
{
	void *p = malloc(len);

	if (p == NULL)
		return (p);

	(void) memset(p, '\0', len);
	return (p);
}
