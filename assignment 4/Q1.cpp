#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <dirent.h>

#define BUFF 256

using namespace std;

struct FileContent{
    char fileName[BUFF];
    char contents[BUFF];
};

void printFiles(vector<FileContent*> files){
    for(int i=0; i< files.size(); ++i){
        cout<<files[i]->fileName<<endl;
        cout<<files[i]->contents<<endl;
        cout<<endl;
    }
}

vector<FileContent *> readDir(const char *path){
    DIR *dir;
    vector<FileContent *> fileList;
    struct dirent *ent;
    if ((dir = opendir (path)) != NULL) {
        // iterate all the files and directories within directory
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_type != DT_REG)continue; // if not regular file ignore
            FileContent *fContent = new FileContent();
            snprintf(fContent->fileName, BUFF+1, "%s", ent->d_name);// copying the filename
            // getting the path of the file
            char filePath[BUFF];
            snprintf(filePath, BUFF+1, "%s/%s", path, fContent->fileName);
            // cout<<filePath<<endl;
            // reading from the file
            FILE *fp = fopen(filePath, "r"); // for reading only
            if(fp == NULL){
                cout<<"Error while opening file: "<<filePath<<endl;
                exit(EXIT_FAILURE);
            }
            char buff[BUFF];
            int i=0;
            if(fgets(buff, BUFF, fp) != NULL){
                snprintf(fContent->contents, BUFF+1, "%s", buff);
            }
            fclose(fp);
            fileList.push_back(fContent);
        }
        closedir (dir);
    }
    else {
        // could not open directory
        cout<<"Eroor while reading directory: "<<path<<endl;
        exit(EXIT_FAILURE);
    }
    // printFiles(fileList);
    return fileList;
}

void sendDir(int *fd, vector<FileContent*> files){
    int fileCount = files.size();
    write(fd[1], &fileCount, sizeof(int));
    for(int i=0; i< fileCount; ++i){
        // sending the file name
        int fileNameLen = strlen(files[i]->fileName);
        cout<<"Sending file: "<<files[i]->fileName<<endl;
        write(fd[1], &fileNameLen, sizeof(int));
        write(fd[1], files[i]->fileName, strlen(files[i]->fileName));
        // sending the file contents
        int contentSize = strlen(files[i]->contents);
        write(fd[1], &contentSize, sizeof(int));
        write(fd[1], files[i]->contents, strlen(files[i]->contents));
        
    }
}

vector<FileContent*> receiveDir(int *fd){
    vector<FileContent*> files; // storing the files along with contents
    int fileCount; // for storing # of files
    read(fd[0], &fileCount, sizeof(int));
    for(int i=0; i< fileCount; ++i){
        FileContent *fc = new FileContent(); // to hold file name and contents
        // reading the file name length
        int fileNameLen;
        read(fd[0], &fileNameLen, sizeof(int));
        // reading the file name
        char fileName[fileNameLen];
        read(fd[0], fileName, fileNameLen);
        strcpy(fc->fileName, fileName);
        int contentSize;
        read(fd[0],&contentSize, sizeof(int));
        // reading lines
        read(fd[0], fc->contents, contentSize);
        files.push_back(fc);
    }
    return files;
}

void createFiles(char *dir, vector<FileContent*> files){
    // creating the same file in the current directory
    int count = files.size();
    for(int i=0; i< count; ++i){
        char *fileName = files[i]->fileName;
        cout<<"Creating the file: "<<fileName<<" in: "<<dir<<endl;
        int pathLen = strlen(dir)+strlen(fileName)+1;
        char path[pathLen];
        snprintf(path, pathLen, "%s/%s", dir, fileName);
        FILE *fp = fopen(path, "w");
        if(fp == NULL){
            cout<<"Error while creating file: "<<fileName<<endl;
            exit(EXIT_FAILURE);
            return;
        }
        fputs(files[i]->contents, fp);
        
        fclose(fp);
    }
}

int main(int argc, char *argv[]){
    pid_t wpid;
    int status = 0;

    if(argc < 3){
        cout<<"2 directory name is needed\n";
        return 0;
    }

    char dir1[BUFF];
    char dir2[BUFF];

    snprintf(dir1, BUFF, "%s", argv[1]);
    snprintf(dir2, BUFF, "%s", argv[2]);

    int fd1[2], fd2[2];
    int ret1 = pipe(fd1), ret2 = pipe(fd2);
    pid_t n1, n2;
    
    if(ret1 == -1){
        cout<<"Creation of pipe1 failed.\n";
        exit(0);
    }
    if(ret2 == -1){
        cout<<"Creation of pipe2 failed.\n";
        exit(0);
    }
    
    n1 = fork();
    if(n1 == -1){
        cout<<"Error in creating child 1\n";
        exit(EXIT_FAILURE);
    }
    else if(n1 == 0){ // 1st child process
        // reading from the directory
        vector<FileContent *> files = readDir(dir1);
        // sending the list of files along with it's contents
        sendDir(fd1, files);
        // reading the list of files along with it's contents
        // as well as creating the files in the current directory
        vector<FileContent *> newFiles = receiveDir(fd2);
        // creating new file
        createFiles(dir1, newFiles);
    }
    else{ // parent process
        n2 = fork();
        if(n2 == -1){
            cout<<"Error creating child 2\n";
            exit(EXIT_FAILURE);
        }
        else if(n2 == 0){ // 2nd child process
            // reading from the directory
            vector<FileContent *> files = readDir(dir2);
            // sending the list of files along with it's contents
            sendDir(fd2, files);
            // reading the list of files along with it's contents
            // as well as creating the files in the cureent directory
            vector<FileContent *> newFiles = receiveDir(fd1);
            // creating new files
            createFiles(dir2, newFiles);
        }
    }
    while ((wpid = wait(&status)) > 0); // waiting for all child process to finish execution

    return 0;
}