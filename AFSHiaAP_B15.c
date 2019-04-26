#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <libgen.h>

#define resetString(x) memset(x, 0, sizeof(x));

//dir tiap ganti laptop harus ganti
static const char *dirpath = "/home/wildangbudhi/Documents/coba";
// static const char *dirpath = "/home/hp/shift4";

static const char set1[]="qE1~ YMUR2\"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";
static const char set2[]="zi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0qE1~ YMUR2\"`hNIdP";
static unsigned char encryptTable[256], decryptTable[256];
int i;

void encrypt(char* name)
{
    for(i=0; i<strlen(name); i++) name[i] = encryptTable[(int) name[i]];
}

void decrypt(char* name)
{
    for(i=0; i<strlen(name); i++) name[i] = decryptTable[(int) name[i]];
}

void Caesar()
{
    for(i=0; i<256; i++) encryptTable[i] = decryptTable[i] = i;

    for(i=0; i<strlen(set1); i++)
        if(set2[i] != set1[i]) {
            encryptTable[(int) set1[i]] = set2[i];
            decryptTable[(int) set2[i]] = set1[i];
        }  
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000], fname[1000];
    int res = 0;
    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

    if(strcmp(path,"/") == 0)
    {
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else {
        strcpy(fname, path);
        encrypt(fname);
        sprintf(fpath, "%s%s",dirpath,fname);
    }

    dp = opendir(fpath);
    if (dp == NULL) return -errno;

    while ((de = readdir(dp)) != NULL) {
            struct stat st;
            memset(&st, 0, sizeof(st));
            st.st_ino = de->d_ino;
            st.st_mode = de->d_type << 12;
            res = (filler(buf, de->d_name, &st, 0));
                    if(res!=0) break;
    }

    closedir(dp);
    return 0;
}


static int xmp_getattr(const char *path, struct stat *stbuf)
{
        printf("%s\n", path);
        int res;
        char fpath[1000];
        sprintf(fpath,"%s%s",dirpath,path);
        res = lstat(fpath, stbuf);

        if (res == -1)
                return -errno;

        return 0;
}

static int xmp_mkdir(const char *path,mode_t mode)
{
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    res = mkdir (fpath,mode);
    if(res == -1)
        return -errno;

    
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static struct fuse_operations xmp_oper = {
        .getattr        = xmp_getattr,
        .mkdir          = xmp_mkdir,
        .readdir        = xmp_readdir,
        .read           = xmp_read,
};

int main(int argc, char *argv[])
{
        umask(0);
        return fuse_main(argc, argv, &xmp_oper, NULL);
}
