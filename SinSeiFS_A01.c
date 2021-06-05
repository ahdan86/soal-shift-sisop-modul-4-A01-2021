#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#define SEGMENT 1024

static  const  char *dirpath = "/home/ahdan/Downloads";

void decryptV(char *str)
{
    char key[] = "SISOP";
    int msgLen = strlen(str), keyLen = strlen(key), i ,j;
    char newKey[msgLen];
    for(i = 0, j = 0; i < msgLen; ++i, ++j){
        if(j == keyLen)
            j = 0;
 
        newKey[i] = key[j];
    }
    newKey[i] = '\0';

    for(i = 0; i < msgLen; ++i)
        str[i] = (((str[i] - newKey[i]) + 26) % 26) + 'A';
}

void encryptV(char *str)
{
    char key[] = "SISOP";
    int msgLen = strlen(str), keyLen = strlen(key), i ,j;
    char newKey[msgLen];
    for(i = 0, j = 0; i < msgLen; ++i, ++j){
        if(j == keyLen)
            j = 0;
 
        newKey[i] = key[j];
    }
    newKey[i] = '\0';

    for(i = 0; i < msgLen; ++i)
        str[i] = ((str[i] + newKey[i]) % 26) + 'A';
}

void rot13(char *start){
    int i, len = strlen(start);
    for (i=0; i<len; i++)
    {
        if( (*(start+i)>='a' && *(start+i)<'n') || (*(start+i)>='A' && *(start+i)<'N') )
            *(start + i) += 13;
        else if ( (*(start+i)>'m' && *(start+i)<'z') || (*(start+i)>'M' && *(start+i)<'Z') )
            *(start +i) -= 13;
    }
}

char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

void getStr(char str[], char newStr[], char delim){
	int len = strlen(str) - 1;
	int index = 0;
    char tempStr[1024];
    sprintf(tempStr, "%s", str);

	while(tempStr[len] != delim){
        if(len == 0)
            break;
		newStr[index] = tempStr[len];
		tempStr[len] = '\0';
		index++;
		len--;
	}
	newStr[index] = tempStr[len];
	index++;
	newStr[index] = '\0';
	tempStr[len] = '\0';

    if(!strcmp(tempStr, "")) 
        newStr[0] = '\0';
    else
        sprintf(str, "%s", tempStr);
}


void atBash(char str[]){
	int i;

	for(i =0;i<strlen(str);i++){
		if(str[i] >= 'A' && str[i] <= 'Z'){
			str[i] = 'Z' - str[i] + 'A';
		}
		if(str[i] >= 'a' && str[i] <= 'z'){
			str[i] = 'z' - str[i] + 'a';
		}
	}
}

void renameRecursive(char *basePath,int tag) {
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            char newPath[1000], oldPath[1000];
            char newName[256];
            char ext[10];
            sprintf(newName, "%s", dp->d_name);
            sprintf(oldPath, "%s/%s", basePath, dp->d_name);
            printf("NEWPATH %s\n", oldPath);

            if(dp->d_type == DT_DIR){
                //Folder AtoZ_ di dalem AtoZ_. Isi folder dalem AtoZ_ yang dalem 
                // gausah di encrypt
                char tempFold[256];
                sprintf(tempFold, "%s", dp->d_name);
                atBash(tempFold);
                if(tag==2)
                {
                    encryptV(tempFold);
                }
                if(!strstr(dp->d_name, "AtoZ_") && !strstr(tempFold, "AtoZ_"))  
                    renameRecursive(oldPath,1);
                else if(!strstr(dp->d_name, "RX_") && !strstr(tempFold, "RX_"))
                    renameRecursive(oldPath,2);
            }
            getStr(newName, ext, '.');
            strrev(ext);
            atBash(newName);
            if(tag==2){
                encryptV(newName);
            }
            strcat(newName, ext);
            sprintf(newPath, "%s/%s", basePath, newName);
            rename(oldPath, newPath);
        }
    }

    closedir(dir);
}

/*FUNGSI FUSE*/
static  int  xmp_getattr(const char *path, struct stat *stbuf)
{
    printf("Masuk GetAttr\n");
    int res;
    char fpath[1000];

    sprintf(fpath,"%s%s",dirpath,path);

    res = lstat(fpath, stbuf);

    if (res == -1) return -errno;

    return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
	char fpath[1000], fileName[100];

	sprintf(fpath, "%s%s", dirpath, path);

    if(strstr(fpath, "AtoZ_")){
        getStr(fpath, fileName, '/');
        strrev(fileName);
        atBash(fileName);
        printf("MASUK SINI AJG\n");

        FILE *fp;
        fp = fopen("/home/ahdan/Downloads/encode.log", "a");
        fprintf(fp, "%s\n", fpath);
        fclose(fp);
    }
    else if(strstr(fpath,"RX_"))
    {
        getStr(fpath, fileName, '/');
        strrev(fileName);
        rot13(fileName);
        printf("hasil rot13: %s\n",fileName);
        atBash(fileName);
        printf("hasil atBash: %s\n",fileName);

        FILE *fp;
        fp = fopen("/home/ahdan/Downloads/encode.log", "a");
        fprintf(fp, "%s%s method: mkdir\n", fpath, fileName);
        fclose(fp);
    }

    strcat(fpath, fileName);
	printf("FILE MKDIR %s\n", fpath);
	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    printf("Masuk Readdir\n");
    char fpath[1000];

    if(strcmp(path,"/") == 0)
    {
        path=dirpath;
        sprintf(fpath,"%s",path);
    } else sprintf(fpath, "%s%s",dirpath,path);

    int res = 0;

    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

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

static int xmp_rename(const char *from, const char *to)
{
	int res;
    char fromPath[1000], toPath[1000];
    sprintf(fromPath, "%s%s", dirpath, from);
    sprintf(toPath, "%s%s", dirpath, to);
    printf("FROM PATH %s\n", fromPath);

    if(strstr(toPath, "AtoZ_") && !strstr(fromPath, "AtoZ_")){
        renameRecursive(fromPath,1);
            
        FILE *fp;
        fp = fopen("/home/ahdan/Downloads/encode.log", "a");
        fprintf(fp, "%s -> %s\n", fromPath, toPath);
        fclose(fp);
    }

    else if(strstr(fromPath, "AtoZ_") && !strstr(toPath, "AtoZ_")){
        renameRecursive(fromPath,1);
    }
    
    else if(strstr(toPath, "RX_") && !strstr(fromPath, "RX_"))
    {
        renameRecursive(fromPath,2);
            
        FILE *fp;
        fp = fopen("/home/ahdan/Downloads/encode.log", "a");
        fprintf(fp, "%s -> %s method:Rename\n", fromPath, toPath);
        fclose(fp);
    }

    else if(strstr(fromPath, "RX_") && !strstr(toPath, "RX_")){
        renameRecursive(fromPath,2);
    }

	res = rename(fromPath, toPath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("Masuk Read\n");
    char fpath[1000];
    if(strcmp(path,"/") == 0)
    {
        path=dirpath;

        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);

    int res = 0;
    int fd = 0 ;

    (void) fi;

    fd = open(fpath, O_RDONLY);

    if (fd == -1) return -errno;

    res = pread(fd, buf, size, offset);

    if (res == -1) res = -errno;

    close(fd);

    return res;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    char fpath[1000];
    char fileName[100], ext[10];

    sprintf(fpath, "%s%s", dirpath, path);
    
    (void) fi;
    if(strstr(fpath, "AtoZ_")){
        getStr(fpath, fileName, '/');
        strrev(fileName);
        getStr(fileName, ext, '.');
        strrev(ext);
        atBash(fileName);
        strcat(fileName, ext);
        printf("MASUK SINI AJG\n");

        FILE *fp;
        fp = fopen("/home/ahdan/Downloads/encode.log", "a");
        fprintf(fp, "%s\n", fpath);
        fclose(fp);
    }
    // strcat(fpath, fileName);

    printf("CREATE %s\n", fpath);
    
    int res;
    res = creat(fpath, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
	.mkdir	= xmp_mkdir,
    .readdir = xmp_readdir,
    .rename	= xmp_rename,
    .read = xmp_read,
    .create = xmp_create,
};



int  main(int  argc, char *argv[])
{
    umask(0);
    if( access( "/home/ahdan/Downloads/encode.log", F_OK ) != 0 ) {
        FILE *fp;
        fp = fopen("/home/ahdan/Downloads/encode.log", "w");
        fclose(fp);
    } 
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
