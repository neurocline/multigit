#include "cache.h"

const char *sha1_file_directory = NULL;
struct cache_entry **active_cache = NULL;
unsigned int active_nr = 0, active_alloc = 0;

void usage(const char *err)
{
	fprintf(stderr, "read-tree: %s\n", err);
	exit(1);
}

static unsigned hexval(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return ~0u;
}

int get_sha1_hex(char *hex, unsigned char *sha1)
{
	int i;
	for (i = 0; i < 20; i++) {
		unsigned int val = (hexval(hex[0]) << 4) | hexval(hex[1]);
		if (val & ~0xff)
			return -1;
		*sha1++ = (unsigned char) val;
		hex += 2;
	}
	return 0;
}

char * sha1_to_hex(unsigned char *sha1)
{
	static char buffer[50];
	static const char hex[] = "0123456789abcdef";
	char *buf = buffer;
	int i;

	for (i = 0; i < 20; i++) {
		unsigned int val = *sha1++;
		*buf++ = hex[val >> 4];
		*buf++ = hex[val & 0xf];
	}
	return buffer;
}

/*
 * NOTE! This returns a statically allocated buffer, so you have to be
 * careful about using it. Do a "strdup()" if you need to save the
 * filename.
 */
char *sha1_file_name(unsigned char *sha1)
{
	int i;
	static char *name, *base;

	if (!base) {
		char *sha1_dir = getenv(DB_ENVIRONMENT);
        if (sha1_dir == NULL)
            sha1_dir = DEFAULT_DB_ENVIRONMENT;
		int len = strlen(sha1_dir);
		base = malloc(len + 60);
		memcpy(base, sha1_dir, len);
		memset(base+len, 0, 60);
		base[len] = '/';
		base[len+3] = '/';
		name = base + len + 1;
	}
	for (i = 0; i < 20; i++) {
		static char hex[] = "0123456789abcdef";
		unsigned int val = sha1[i];
		char *pos = name + i*2 + (i > 0);
		*pos++ = hex[val >> 4];
		*pos = hex[val & 0xf];
	}
	return base;
}

void * read_sha1_file(unsigned char *sha1, char *type, unsigned long *size)
{
	z_stream stream;
	char buffer[8192];
	struct xplat_stat st;
	int fd, ret, bytes;
	void *map, *buf;
	char *filename = sha1_file_name(sha1);

	fd = xplat_open(filename, O_RDONLY);
	if (fd < 0) {
		perror(filename);
		return NULL;
	}
	if (xplat_fstat(fd, &st) < 0) {
		xplat_close(fd);
		return NULL;
	}
	map = xplat_mmap_READ_PRIVATE(NULL, (size_t) st.st_size, fd, 0);
	xplat_close(fd);
	if (-1 == (int)(long)map)
		return NULL;

	/* Get the data stream */
	memset(&stream, 0, sizeof(stream));
	stream.next_in = map;
	stream.avail_in = (uInt) st.st_size;
	stream.next_out = (Bytef*) buffer;
	stream.avail_out = sizeof(buffer);

	inflateInit(&stream);
	ret = inflate(&stream, 0);
	if (sscanf(buffer, "%10s %lu", type, size) != 2)
		return NULL;
	bytes = strlen(buffer) + 1;
	buf = malloc(*size);
	if (!buf)
		return NULL;

	memcpy(buf, buffer + bytes, stream.total_out - bytes);
	bytes = stream.total_out - bytes;
	if (bytes < (int)*size && ret == Z_OK) {
		stream.next_out = (Bytef*)buf + bytes;
		stream.avail_out = *size - bytes;
		while (inflate(&stream, Z_FINISH) == Z_OK)
			/* nothing */;
	}
	inflateEnd(&stream);
	return buf;
}

int write_sha1_file(char *buf, unsigned len)
{
	int size;
	char *compressed;
	z_stream stream;
	unsigned char sha1[20];
	SHA_CTX c;

	/* Set it up */
	memset(&stream, 0, sizeof(stream));
	deflateInit(&stream, Z_BEST_COMPRESSION);
	size = deflateBound(&stream, len);
	compressed = malloc(size);

	/* Compress it */
	stream.next_in = (Bytef*) buf;
	stream.avail_in = len;
	stream.next_out = (Bytef*)compressed;
	stream.avail_out = size;
	while (deflate(&stream, Z_FINISH) == Z_OK)
		/* nothing */;
	deflateEnd(&stream);
	size = stream.total_out;

	/* Sha1.. */
	SHA1_Init(&c);
	SHA1_Update(&c, compressed, size);
	SHA1_Final(sha1, &c);

	if (write_sha1_buffer(sha1, compressed, size) < 0)
		return -1;
	printf("%s\n", sha1_to_hex(sha1));
	return 0;
}

int write_sha1_buffer(unsigned char *sha1, void *buf, unsigned int size)
{
	char *filename = sha1_file_name(sha1);
	int fd;

	fd = xplat_open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (fd < 0)
		return (errno == EEXIST) ? 0 : -1;
	xplat_write(fd, buf, size);
	xplat_close(fd);
	return 0;
}

static int error(const char * string)
{
	fprintf(stderr, "error: %s\n", string);
	return -1;
}

static int verify_hdr(struct cache_header *hdr, unsigned long size)
{
	SHA_CTX c;
	unsigned char sha1[20];

	if (hdr->signature != CACHE_SIGNATURE)
		return error("bad signature");
	if (hdr->version != 1)
		return error("bad version");
	SHA1_Init(&c);
	SHA1_Update(&c, hdr, offsetof(struct cache_header, sha1));
	SHA1_Update(&c, hdr+1, size - sizeof(*hdr));
	SHA1_Final(sha1, &c);
	if (memcmp(sha1, hdr->sha1, 20))
		return error("bad header sha1");
	return 0;
}

int read_cache(void)
{
	int fd, i;
	struct xplat_stat st;
	unsigned long size, offset;
	void *map;
	struct cache_header *hdr;

	errno = EBUSY;
	if (active_cache)
		return error("more than one cachefile");
	errno = ENOENT;
	sha1_file_directory = getenv(DB_ENVIRONMENT);
	if (!sha1_file_directory)
		sha1_file_directory = DEFAULT_DB_ENVIRONMENT;
	if (!xplat_access_R_OK(sha1_file_directory))
		return error("no access to SHA1 file directory");
	fd = xplat_open(".dircache/index", O_RDONLY);
	if (fd < 0)
		return (errno == ENOENT) ? 0 : error("open failed");

	map = (void *)-1;
	size = 0;
	if (!xplat_fstat(fd, &st)) {
		map = NULL;
		size = (unsigned long) st.st_size;
		errno = EINVAL;
		if (size >= sizeof(struct cache_header))
			map = xplat_mmap_READ_PRIVATE(NULL, size, fd, 0);
	}
	xplat_close(fd);
	if (-1 == (int)(long)map)
		return error("mmap failed");

	hdr = map;
	if (verify_hdr(hdr, size) < 0)
		goto unmap;

	active_nr = hdr->entries;
	active_alloc = alloc_nr(active_nr);
	active_cache = calloc(active_alloc, sizeof(struct cache_entry *));

	offset = sizeof(*hdr);
	for (i = 0; i < (int) hdr->entries; i++) {
		struct cache_entry *ce = (struct cache_entry *)((char*) map + offset);
		offset = offset + ce_size(ce);
		active_cache[i] = ce;
	}
	return active_nr;

unmap:
	xplat_munmap(map, size);
	errno = EINVAL;
	return error("verify header failed");
}

