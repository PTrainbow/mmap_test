#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <strings.h>


int getFileSize(const char *name);

void appendData(char *data);

int offset = 0;
int mmapSize = 0;
int fd = 0;
char *mmapPointer = NULL;
int offsetSize = 4;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("need input data, found none");
        exit(0);
    }
    char *testStr = argv[1];
    const char *fileName = "../data.bin";
    fd = open(fileName, O_RDWR | O_CREAT, 0777);
    int fileSize = getFileSize(fileName);
    // 获取一页的内存大小
    int pageSize = getpagesize();
    // 不满一页补足
    if (fileSize == 0 || fileSize % pageSize != 0) {
        mmapSize = (fileSize / pageSize + 1) * pageSize;
        if (ftruncate(fd, mmapSize) != 0) {
            return 0;
        }
    } else {
        mmapSize = fileSize;
    }
    // mmap
    mmapPointer = (char *) mmap(NULL, mmapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmapPointer == MAP_FAILED) {
        printf("mmap失败");
        return 0;
    }
    // 读取前四个字节找到偏移量
    memcpy(&offset, mmapPointer, offsetSize);
    // 写数据
    appendData(testStr);
    // 写 offset
    memcpy(mmapPointer, &offset, offsetSize);
    munmap(mmapPointer, mmapSize);
    close(fd);
    return 0;
}

void appendData(char *data) {
    if (offset + strlen(data) + offsetSize > mmapSize) {
        int newSize = mmapSize * 2;
        while (offset + strlen(data) > newSize) {
            newSize *= 2;
        }
        if (ftruncate(fd, newSize) != 0) {
            printf("fail to truncate [%zu] to size %lld", mmapSize, newSize);
            return;
        }
        if (munmap(mmapPointer, mmapSize) != 0) {
            return;
        }
        mmapSize = newSize;
        mmapPointer = (char *) (mmap(NULL, mmapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (mmapPointer == MAP_FAILED) {
            printf("mmap失败");
            return;
        }
    }
    memcpy(mmapPointer + offset + offsetSize, data, strlen(data));
    offset = offset + strlen(data);
}


int getFileSize(const char *name) {
    struct stat statbuf;
    stat(name, &statbuf);
    return statbuf.st_size;
}
