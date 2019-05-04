#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <sys/xattr.h>
#include <sys/wait.h>
#include <pthread.h>

#define resetString(x) memset(x, 0, sizeof(x));

static const char *dirpath = "/home/wildangbudhi/Documents/coba";
// static const char *dirpath = "/home/hp/shift4";

static const char worldlist[] = "qE1~ YMUR2\"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";
static const int KEYchiper = 17;
static const char filemiris[] = "filemiris.txt";

void Decrypt(char *s){
    int idx;
    char *ptr;

    for (int i = 0; i < strlen(s); i++){
        if (s[i] == '/') continue;

        ptr = strchr(worldlist, s[i]);
        if(ptr){
            idx = ptr - worldlist - KEYchiper;
            if (idx < 0) idx = idx + strlen(worldlist);
            s[i] = worldlist[idx];
        }
    }
}

void Encrypt(char *s){
    int idx;
    char *ptr;

    for (int i = 0; i < strlen(s); i++){
        if (s[i] == '/') continue;

        ptr = strchr(worldlist, s[i]);
        if(ptr){
            idx = ptr - worldlist;
            s[i] = worldlist[(idx + KEYchiper) % strlen(worldlist)];
        }
    }
}

void cekBahaya(const char *path, const char *name){
    struct stat stat;
	lstat(path, &stat);
	struct passwd *pwd = getpwuid(stat.st_uid);
	struct group *grp = getgrgid(stat.st_gid);
	int ans = 0;
	if (!strcmp(pwd->pw_name, "ic-controller") || !strcmp(pwd->pw_name, "chipset"))
		if (strcmp(grp->gr_name, "rusak") == 0 && access(path, R_OK) != 0) ans = 1;
    
    if(ans){
        FILE *fp;
		struct tm *tm;
		char buf[200],  filemirispath[1000];

		time_t t = stat.st_atime;
		tm = localtime(&t);
		strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", tm);

		sprintf(filemirispath, "%s/%s", dirpath, filemiris);
		fp = fopen(filemirispath, "a");

		fprintf(fp, "%s %s %s %s\n", name, grp->gr_name, pwd->pw_name, buf);
        
		fclose(fp);
		remove(path);
    }
}

static int xmp_readdir(const char *path, 
                       void *buf, 
                       fuse_fill_dir_t filler, 
                       off_t offset, 
                       struct fuse_file_info *fi)
{
    char fpath[1000], fname[1000], c[1000];
    int res = 0;
    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }else {
        strcpy(fname, path);
        Encrypt(fname);
        sprintf(fpath, "%s%s",dirpath,fname);
    }

    dp = opendir(fpath);
    if (dp == NULL) return -errno;

    while ((de = readdir(dp)) != NULL) {
        if(strcmp(de->d_name,".")==0||strcmp(de->d_name,"..")==0) continue;
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        strcpy(c, de->d_name);
        Decrypt(c);
        res = (filler(buf, c, &st, 0));
        if(res!=0) break;
    }

    closedir(dp);
    return 0;
}


static int xmp_getattr(const char *path, struct stat *stbuf)
{
        int res;
        char fpath[1000];

        strcpy(fpath, path);
        Encrypt(fpath);
        if(strcmp(path,"/") == 0) sprintf(fpath,"%s",fpath);
        else sprintf(fpath,"%s%s",dirpath,fpath);

        res = lstat(fpath, stbuf);

        if (res == -1)return -errno;
        return 0;
}

static int xmp_mkdir(const char *path,mode_t mode)
{
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    res = mkdir (fpath,mode);
    if(res == -1) return -errno;
    return 0;
}

static int xmp_read(const char *path, 
                    char *buf, 
                    size_t size, 
                    off_t offset, 
                    struct fuse_file_info *fi)
{
	int fd, res;
    char fpath[1000], fname[1000];

	(void) fi;

    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else {
        strcpy(fname, path);
        Encrypt(fname);
        sprintf(fpath, "%s%s",dirpath,fname);
    }

	fd = open(fpath, O_RDONLY);
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
