#include "cache.h"

#include <time.h>

#define BLOCKING (1ul << 14)
#define ORIG_OFFSET (40)

/*
 * Leave space at the beginning to insert the tag
 * once we know how big things are.
 *
 * FIXME! Share the code with "write-tree.c"
 */
static void init_buffer(char **bufp, unsigned int *sizep)
{
	char *buf = malloc(BLOCKING);
	memset(buf, 0, ORIG_OFFSET);
	*sizep = ORIG_OFFSET;
	*bufp = buf;
}

static void add_buffer(char **bufp, unsigned int *sizep, const char *fmt, ...)
{
	char one_line[2048];
	va_list args;
	int len;
	unsigned long alloc, size, newsize;
	char *buf;

	va_start(args, fmt);
	len = vsnprintf(one_line, sizeof(one_line), fmt, args);
	va_end(args);
	size = *sizep;
	newsize = size + len;
	alloc = (size + 32767) & ~32767;
	buf = *bufp;
	if (newsize > alloc) {
		alloc = (newsize + 32767) & ~32767;
		buf = realloc(buf, alloc);
		*bufp = buf;
	}
	*sizep = newsize;
	memcpy(buf + size, one_line, len);
}

static int prepend_integer(char *buffer, unsigned val, int i)
{
	buffer[--i] = '\0';
	do {
		buffer[--i] = '0' + (val % 10);
		val /= 10;
	} while (val);
	return i;
}

static void finish_buffer(char *tag, char **bufp, unsigned int *sizep)
{
	int taglen;
	int offset;
	char *buf = *bufp;
	unsigned int size = *sizep;

	offset = prepend_integer(buf, size - ORIG_OFFSET, ORIG_OFFSET);
	taglen = strlen(tag);
	offset -= taglen;
	buf += offset;
	size -= offset;
	memcpy(buf, tag, taglen);

	*bufp = buf;
	*sizep = size;
}

static void remove_special(char *p)
{
	char c;
	char *dst = p;

	for (;;) {
		c = *p;
		p++;
		switch(c) {
		case '\n': case '<': case '>':
			continue;
		}
		*dst++ = c;
		if (!c)
			break;
	}
}

/*
 * Having more than two parents may be strange, but hey, there's
 * no conceptual reason why the file format couldn't accept multi-way
 * merges. It might be the "union" of several packages, for example.
 *
 * I don't really expect that to happen, but this is here to make
 * it clear that _conceptually_ it's ok..
 */
#define MAXPARENT (16)

// Struct containing bits of commit info that are fetched in a machine-specific way
struct commit_info
{
    char* gecos;
    char* email;
    char* date;
    char* realgecos;
    char realemail[1000];
    char* realdate;
    char comment[1000];
};
int fill_commit_info(struct commit_info* info); // per-platform routine

int main(int argc, char **argv)
{
	int i;
	int parents = 0;
	unsigned char tree_sha1[20];
	unsigned char parent_sha1[MAXPARENT][20];
    struct commit_info info;
	char *buffer;
	unsigned int size;

	if (argc < 2 || get_sha1_hex(argv[1], tree_sha1) < 0)
		usage("commit-tree <sha1> [-p <sha1>]* < changelog");

	for (i = 2; i < argc; i += 2) {
		char *a, *b;
		a = argv[i]; b = argv[i+1];
		if (!b || strcmp(a, "-p") || get_sha1_hex(b, parent_sha1[parents]))
			usage("commit-tree <sha1> [-p <sha1>]* < changelog");
		parents++;
	}
	if (!parents)
		fprintf(stderr, "Committing initial tree %s\n", argv[1]);

    if (!fill_commit_info(&info))
		usage("You don't exist. Go away!");

	init_buffer(&buffer, &size);
	add_buffer(&buffer, &size, "tree %s\n", sha1_to_hex(tree_sha1));

	/*
	 * NOTE! This ordering means that the same exact tree merged with a
	 * different order of parents will be a _different_ changeset even
	 * if everything else stays the same.
	 */
	for (i = 0; i < parents; i++)
		add_buffer(&buffer, &size, "parent %s\n", sha1_to_hex(parent_sha1[i]));

	/* Person/date information */
	add_buffer(&buffer, &size, "author %s <%s> %s\n", info.gecos, info.email, info.date);
	add_buffer(&buffer, &size, "committer %s <%s> %s\n\n", info.realgecos, info.realemail, info.realdate);

	/* And add the comment */
	while (fgets(info.comment, sizeof(info.comment), stdin) != NULL)
		add_buffer(&buffer, &size, "%s", info.comment);

	finish_buffer("commit ", &buffer, &size);

	write_sha1_file(buffer, size);
	return 0;
}

#ifdef __linux__
#include <pwd.h>
int fill_commit_info(struct commit_info* info)
{
    int len;
	struct passwd *pw;
	time_t now;

	pw = getpwuid(getuid());
	if (!pw)
		return 0;
	info->realgecos = pw->pw_gecos;
	len = strlen(pw->pw_name);
	memcpy(info->realemail, pw->pw_name, len);
	info->realemail[len] = '@';
	gethostname(info->realemail+len+1, sizeof(info->realemail)-len-1);
	time(&now);
	info->realdate = ctime(&now);

	info->gecos = getenv("COMMITTER_NAME"); if (!info->gecos) info->gecos = info->realgecos;
	info->email = getenv("COMMITTER_EMAIL"); if (!info->email) info->email = info->realemail;
	info->date = getenv("COMMITTER_DATE"); if (!info->date) info->date = info->realdate;

	remove_special(info->gecos); remove_special(info->realgecos);
	remove_special(info->email); remove_special(info->realemail);
	remove_special(info->date); remove_special(info->realdate);

    return 1;
}
#endif
