#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

#define BUFSIZE     200

char currentPath[BUFSIZE]="";

int antivirus(char *file_name)
{
    //需要感染的目标文件，已经被病毒文件，和一个临时文件
    FILE *fp_obj,*fp_tmp;
    //对文件一行一行的的扫描，每行的最大容量
    char buf[BUFSIZE];
    //一个标识，对文件感染的判断
    int flag;
    //临时文件名
    char *tmp_buf;
    //得到目标文件的stat
    struct stat statBuf;
    //用来放置文件的访问时间和修改时间
    struct utimbuf timeBuf;

    //打开目标文件，用于读写
    if ((fp_obj=fopen(file_name,"r+"))==NULL)
    {
        printf("open object file %s failure !!!\n",file_name);
        return 1;
    }
    //得到目标文件的stat
    if (stat(file_name,&statBuf) < 0)
    {
        printf("get object file %s stat failure !!\n",file_name);
        return 1;
    }
    //存放目标文件的修改时间和访问时间，这个避免被发现文件已被感染
    timeBuf.actime = statBuf.st_atime;
    timeBuf.modtime = statBuf.st_mtime;
   
    //新建一个临时文件
    if ((tmp_buf=tmpnam(NULL))==NULL)
    {
        printf("create temp file failure !!\n");
        return 1;
    }
    //打开临时文件，每次都是在临时文件中添加内容
    if ((fp_tmp=fopen(tmp_buf,"a+"))==NULL)
    {
        printf("open temp file failure !!\n");
        return 1;
    }
    //使用完后删除临时文件
    unlink(tmp_buf);

    flag = 1;
    //一行一行遍历目标文件
    while (fgets(buf,BUFSIZE,fp_obj) != NULL)
    {
        //目标文件已被感染的标志
        if (!strcmp(buf,"/* linux virus start */\n"))
        {
            printf("object file %s has been recovered !!\n",file_name);
            flag = 0;
            break;
        }

        //删除do_virus();语句
        if (!strcmp(buf,"\tdo_virus();\n"))
        {
            continue;
        }

        fputs(buf,fp_tmp);
    }
    
    //目标文件没有main函数，即不会被感染，直接返回
    if (flag != 0)
    {
        printf("object don't been infected !!\n");
        return 1;
    }

    //重置文件指针
    rewind(fp_tmp);
    if ((fp_obj=fopen(file_name,"w+"))==NULL)
    {
        printf("open object file %s failure !!!\n",file_name);
        return 1;
    }
    //将临时文件的内容覆盖掉目标文件的内容
    while (fgets(buf,BUFSIZE,fp_tmp) != NULL)
    {
        fputs(buf,fp_obj);
    }
    //关闭临时文件和目标文件
    fclose(fp_tmp);
    fclose(fp_obj);

    //设置目标文件的访问时间和修改时间为原来的时间
    if (utime(file_name,&timeBuf) < 0)
    {
        printf("set access and modify time failure !!!\n");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    antivirus(argv[1]);
    return 0;
}
