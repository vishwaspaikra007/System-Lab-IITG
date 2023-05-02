#include <bits/stdc++.h>
using namespace std;
/*
    creating, reading, updating, and deleting
*/
struct File{
    string name;
    vector<int> contents;
    File(string name_ = "", vector<int> contents_ = {}):name{name_}, contents{contents_}{}
};

class Contiguous{
private:
    int diskSize, blockSize, blockNo;
    vector<int> diskSpace; // represents the blocks in disk
    unordered_map<string, pair<int, int>> startBlock; // to get the start block of file & #of blocks
    unordered_set<string> files;
    /*
        To check if l contiguous blocks are available or not
    */
    int check(int l){
        int c = 0;
        for(int i = 0; i< blockNo; ++i){
            if(diskSpace[i] != -1)c = 0;
            else c++;
            if(c == l)return i-l+1;
        }
        return -1;
    }
public:
    Contiguous(int diskSize_, int blockSize_):diskSize{diskSize_}, blockSize{blockSize_}{
        blockNo = (diskSize - 1)/blockSize + 1;
        diskSpace.resize(blockNo, -1); // -1->empty, else->contains a file
    }
    /*
        Adding a file in contiguos allocation
        @parameters
            file: type string represents file name
            len : type int    represents file size
    */
    void Add(File f){
        string file = f.name;
        vector<int> contents = f.contents;
        int blocksNeeded = contents.size();
        if(files.find(file) != files.end()){
            cout<<file<<" same name file already exixts.\n";
            return;
        }
        // int blocksNeeded = (len-1)/blockSize + 1;
        int index = check(blocksNeeded);
        // cout<<file<<" need "<<blocksNeeded<<" blocks in contiguous allocation!\n";
        if(index == -1){
            cout<<"Can not store "<<file<<" as not enough contiguous space is left.\n";
            return;
        }
        startBlock[file] = {index, blocksNeeded};
        files.insert(file);
        int i = 0;
        while(blocksNeeded--)diskSpace[index++] = contents[i++];
        cout<<"The file "<<file<<" is stored in Contiguous allocation!\n";
    }
    /*
        Deleting a file
        @parameters
            file: type string represents file name
    */
    void Delete(File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" do not exists in contiguous allocation.\n";
            return;
        }
        files.erase(file);
        int i = startBlock[file].first;
        int blockNumber = startBlock[file].second;
        startBlock.erase(file);
        while(blockNumber--)diskSpace[i++] = -1;
        cout<<file<<" is deleted from contiguous allocation.\n";
    }
    /*
        Update a file
        @parameter
            file: type string represents file name
            len : type int    represents updated file length
    */
    void Update(File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" can not be updated as it do not exists in contiguous allocation.\n";
            return;
        }
        vector<int> contents = f.contents;
        int blocksNeeded = contents.size(), prevBlock = startBlock[file].second;
        if(blocksNeeded == 0){ // if the updated length 0 then delete the file
            this->Delete(file);
            return;
        }
        // int blocksNeeded = (len-1)/blockSize + 1, prevBlock = startBlock[file].second;
        // cout<<file<<" now need "<<blocksNeeded<<" blocks in contiguous allocation to update!\n";
        int i, index, j = 0;
        i = startBlock[file].first; // the start index of the file
        
        int extraBlocks = blocksNeeded - prevBlock;
        if(extraBlocks <= 0){ // file size reduced
            startBlock[file].second = blocksNeeded;
            while(blocksNeeded-- && prevBlock--)diskSpace[i++] = contents[j++]; // copying the contents
            while(prevBlock--)diskSpace[i++] = -1; // deleting excess blocks
        }
        else{ // file size increased
            index = i + prevBlock; // get the end index of the current file
            while(extraBlocks-- && diskSpace[index] == -1)index++;
            if(extraBlocks >= 0){ // not enough contiguous space to update the file
                cout<<"Can not update "<<file<<" due to insufficient contiguous blocks.\n";
                return;
            }
            index = i;
            startBlock[file].second = blocksNeeded; // update the current blocks size of file
            while(blocksNeeded--)diskSpace[i++] = contents[j++]; // update the disk space
        }
        cout<<file<<" is upadted!\n";
    }
    /*
        Read file
        @parameters
            file: type string represents file name
    */
    void ReadFile (File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" is not present in contiguous allocation.\n";
            return;
        }
        int i = startBlock[file].first; // find the start index of the file
        int blocks = startBlock[file].second;
        cout<<"Reading file "<<file<<": ";
        while(blocks--)cout<<diskSpace[i++]<<" ";
        cout<<endl;
    }
    /*
        To show the current allocation
    */
    void showDisk(){
        for(auto ind: diskSpace){
            cout<<ind<<" ";
        }
        cout<<endl;
    }
};

class Linked{
private:
    int diskSize, blockSize, blockNo;
    vector<vector<int>> diskSpace; // represents the blocks in disk
    unordered_map<string, pair<int, int>> startBlock; // to get the start and number of blocks
    unordered_set<string> files;
    unordered_set<int> blankBlocks;
    /*
        To check if l contiguous blocks are available or not
    */
    bool check(int l){
        // cout<<"size "<<blankBlocks.size()<<endl;
        return blankBlocks.size() >= l;
    }
public:
    Linked(int diskSize_, int blockSize_):diskSize{diskSize_}, blockSize{blockSize_}{
        blockNo = (diskSize - 1)/blockSize + 1;
        diskSpace.resize(blockNo, vector<int>(2, -1)); // [i][1]-> -1 end of a file, else->contains a file
        for(int i = 0; i< blockNo; ++i)blankBlocks.insert(i); // initially all blocks are blank or empty
    }
    /*
        Adding a file in contiguos allocation
        @parameters
            file: type string represents file name
            len : type int    represents file size
    */
    void Add(File f){
        string file = f.name;
        if(files.find(file) != files.end()){
            cout<<file<<" same name file already exixts.\n";
            return;
        }
        vector<int> contents = f.contents;
        int blocksNeeded = contents.size();
        bool b = check(blocksNeeded);
        // cout<<file<<" need "<<blocksNeeded<<" blocks in contiguous allocation!\n";
        if(!b){
            cout<<"Can not store "<<file<<" as not enough blocks are left in linked allocation.\n";
            return;
        }
        // cout<<index<<endl;
        int j = 0;
        int index = *blankBlocks.begin();
        
        startBlock[file].first = index;
        startBlock[file].second = blocksNeeded; // storing number of blocks

        blankBlocks.erase(index);
        blocksNeeded--;
        diskSpace[index][0] = contents[j++];
        files.insert(file);
        
        while(blocksNeeded--){
            int next = *blankBlocks.begin();
            blankBlocks.erase(next);
            diskSpace[next][0] = contents[j++];
            diskSpace[index][1] = next;
            index = next;
        }
        diskSpace[index][1] = -1;
        cout<<"The file "<<file<<" is stored in linked allocation!\n";
    }
    /*
        Deleting a file
        @parameters
            file: type string represents file name
    */
    void Delete(File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" do not exists in linked allocation.\n";
            return;
        }
        files.erase(file);
        
        int i = startBlock[file].first;
        startBlock.erase(file);

        while(i != -1){
            int next = diskSpace[i][1];
            diskSpace[i][1] = -1;
            diskSpace[i][0] = -1;
            blankBlocks.insert(i);
            i = next;
        }
        cout<<file<<" is deleted from linked allocation.\n";
    }
    /*
        Update a file
        @parameter
            file: type string represents file name
            len : type int    represents updated file length
    */
    void Update(File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" can not be updated as it do not exists in Linked allocation.\n";
            return;
        }
        vector<int> contents = f.contents;
        int blocksNeeded = contents.size();
        if(blocksNeeded == 0){ // if the updated length 0 then delete the file
            this->Delete(file);
            return;
        }
        // int blocksNeeded = (len-1)/blockSize + 1;

        int i = startBlock[file].first; // get the starting block
        int prevBlocks = startBlock[file].second;

        if(blocksNeeded <= prevBlocks){ // file size is reduced
            int j = 0;
            startBlock[file].second = blocksNeeded; // updating the block size
            while(--blocksNeeded){
                diskSpace[i][0] = contents[j++];
                i = diskSpace[i][1];
            }
            diskSpace[i][0] = contents[j++];
            int ii = diskSpace[i][1];
            diskSpace[i][1] = -1;
            i = ii;

            while(i != -1){
                diskSpace[i][0] = -1;
                int i_ = diskSpace[i][1];
                diskSpace[i][1] = -1;
                blankBlocks.insert(i);
                i = i_;
            }
        }
        else{ // file size is increasing
            bool index = check(blocksNeeded - prevBlocks);
            if(!index){
                cout<<"Can not update "<<file<<" as not enough blocks are left in Linked allocation.\n";
                return;
            }
            int i = startBlock[file].first, j = 0;
            startBlock[file].second = blocksNeeded;
            while(diskSpace[i][1] != -1){
                diskSpace[i][0] = contents[j++];
                i = diskSpace[i][1];
            }
            while(j < blocksNeeded-1){
                int next = *blankBlocks.begin();
                blankBlocks.erase(next);
                diskSpace[i][0] = contents[j++];
                diskSpace[i][1] = next;
                i = next;
            }
            diskSpace[i][0] = contents[j++];
            diskSpace[i][1] = -1;
        }
        cout<<"The file "<<file<<" is updated in linked allocation!\n";

    }
    /*
        Read file
        @parameters
            file: type string represents file name
    */
    void ReadFile (File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" is not present in Linked allocation.\n";
            return;
        }
        int i = startBlock[file].first; // find the start index of the file
        cout<<"Reading file "<<file<<" from block \n";
        while(diskSpace[i][1] != -1)cout<<diskSpace[i][0]<<" ", i = diskSpace[i][1];
        cout<<endl;
    }
    /*
        To show the current allocation
    */
    void showDisk(){
        for(auto ind: diskSpace){
            cout<<ind[0]<<"\t"<<ind[1]<<"\n";
        }
    }
};

class Indexed{
    private:
    int diskSize, blockSize, blockNo;
    vector<vector<int>> diskSpace; // represents the blocks in disk
    unordered_map<string, int> indexBlock; // to get the index block of file
    unordered_set<string> files;
    unordered_set<int> blankBlocks;
    /*
        To check if l contiguous blocks are available or not
    */
    int check(int l){
        return blankBlocks.size() >= l;
    }
public:
    Indexed(int diskSize_, int blockSize_):diskSize{diskSize_}, blockSize{blockSize_}{
        blockNo = (diskSize - 1)/blockSize + 1;
        diskSpace.resize(blockNo); // {}->empty, else->contains a file
        for(int i = 0; i< blockNo; ++i)blankBlocks.insert(i); // initially all blocks are blank or empty
    }
    /*
        Adding a file in contiguos allocation
        @parameters
            file: type string represents file name
            len : type int    represents file size
    */
    void Add(File f){
        string file = f.name;
        if(files.find(file) != files.end()){
            cout<<file<<" same name file already exixts.\n";
            return;
        }
        vector<int> contents = f.contents;
        int blocksNeeded = contents.size() + 1;
        bool b = check(blocksNeeded); // as a block is required for index
        // cout<<file<<" need "<<blocksNeeded<<" blocks in contiguous allocation!\n";
        if(!b){
            cout<<"Can not store "<<file<<" as not enough blocks are left in Indexed allocation.\n";
            return;
        }
        // cout<<index<<endl;
        int index = *blankBlocks.begin(), j = 0;
        indexBlock[file] = index;
        blankBlocks.erase(index);
        blocksNeeded--;
        files.insert(file); // inserting files in file list
        
        while(blocksNeeded--){
            int next = *blankBlocks.begin();
            blankBlocks.erase(next);
            diskSpace[next].push_back(contents[j++]); // contains a file
            diskSpace[index].push_back(next);
        }
        cout<<"The file "<<file<<" is stored in indexed allocation!\n";
    }
    /*
        Deleting a file
        @parameters
            file: type string represents file name
    */
    void Delete(File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" do not exists in Indexed allocation.\n";
            return;
        }
        files.erase(file); // erasing from file list
        int i = indexBlock[file];
        indexBlock.erase(file);
        for(auto ind: diskSpace[i]){
            blankBlocks.insert(ind);
            diskSpace[ind].clear();
        }
        diskSpace[i].clear();
        blankBlocks.insert(i);
        cout<<file<<" is deleted from Indexed allocation.\n";
    }
    /*
        Update a file
        @parameter
            file: type string represents file name
            len : type int    represents updated file length
    */
    void Update(File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" can not be updated as it do not exists in indexed allocation.\n";
            return;
        }
        vector<int> contents = f.contents;
        int blocksNeeded = contents.size();
        
        if(blocksNeeded == 0){ // if the updated length 0 then delete the file
            this->Delete(file);
            return;
        }
        int i = indexBlock[file]; // get the index block
        int extraBlocks = blocksNeeded - diskSpace[i].size();
        
        if(extraBlocks < 0){ // file size is reduced
            int k = 0;
            for(; k< blocksNeeded; ++k)diskSpace[diskSpace[i][k]][0] = contents[k];
            for(; k< diskSpace[i].size(); ++k){
                blankBlocks.insert(diskSpace[i][k]);
                diskSpace[diskSpace[i][k]].clear();
            }
            diskSpace[i].resize(blocksNeeded); // need modification if necessary
        }
        else if(extraBlocks > 0){ // file size is increasing
            if(extraBlocks > blankBlocks.size()){
                cout<<"Can not update "<<file<<" as not enough blocks are left in indexed allocation.\n";
                return;
            }
            int j = 0;
            for(auto b: diskSpace[i])diskSpace[b][0] = contents[j++];
            while(extraBlocks--){
                int ind = *blankBlocks.begin();
                blankBlocks.erase(ind);
                diskSpace[ind].push_back(contents[j++]);
                diskSpace[i].push_back(ind);
            }
        }
        cout<<"The file "<<file<<" is updated in indexed allocation!\n";
    }
    /*
        Read file
        @parameters
            file: type string represents file name
    */
    void ReadFile (File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" is not present in Indexed allocation.\n";
            return;
        }
        int i = indexBlock[file]; // find the start index of the file
        cout<<"Reading file "<<file<<" from block \n";
        for(auto block: diskSpace[i])cout<<diskSpace[block][0]<<" ";
        cout<<endl;
    }
    /*
        To show the current allocation
    */
    void showDisk(){
        for(auto ind: diskSpace){
            for(auto i: ind)
                cout<<i<<" ";
            cout<<"--"<<endl;
        }
        cout<<endl;
    }
};

class ModifiedConteguous{
private:
    int diskSize, blockSize, blockNo;
    vector<vector<int>> diskSpace; // represents the blocks in disk
    unordered_map<string, pair<int, int>> startBlock; // to get the start and number of blocks
    unordered_set<string> files;
    set<int> blankBlocks;
    /*
        To check if l contiguous blocks are available or not
    */
    bool check(int l){
        // cout<<"size "<<blankBlocks.size()<<endl;
        return blankBlocks.size() >= l;
    }
public:
    ModifiedConteguous(int diskSize_, int blockSize_):diskSize{diskSize_}, blockSize{blockSize_}{
        blockNo = (diskSize - 1)/blockSize + 1;
        diskSpace.resize(blockNo, vector<int>(2, -1)); // [i][1]-> -1 end of a file, else->contains a file
        for(int i = 0; i< blockNo; ++i)blankBlocks.insert(i); // initially all blocks are blank or empty
    }
    /*
        Adding a file in contiguos allocation
        @parameters
            file: type string represents file name
            len : type int    represents file size
    */
    void Add(File f){
        string file = f.name;
        if(files.find(file) != files.end()){
            cout<<file<<" same name file already exixts.\n";
            return;
        }
        vector<int> contents = f.contents;
        int blocksNeeded = contents.size();
        bool b = check(blocksNeeded);
        // cout<<file<<" need "<<blocksNeeded<<" blocks in contiguous allocation!\n";
        if(!b){
            cout<<"Can not store "<<file<<" as not enough blocks are left in ModifiedConteguous allocation.\n";
            return;
        }
        // cout<<index<<endl;
        int j = 0;
        int index = *blankBlocks.begin();
        
        startBlock[file].first = index;
        startBlock[file].second = blocksNeeded; // storing number of blocks

        blankBlocks.erase(index);
        blocksNeeded--;
        diskSpace[index][0] = contents[j++];
        files.insert(file);
        
        while(blocksNeeded--){
            int next = *blankBlocks.begin();
            blankBlocks.erase(next);
            diskSpace[next][0] = contents[j++];
            diskSpace[index][1] = next;
            index = next;
        }
        diskSpace[index][1] = -1;
        cout<<"The file "<<file<<" is stored in ModifiedConteguous allocation!\n";
    }
    /*
        Deleting a file
        @parameters
            file: type string represents file name
    */
    void Delete(File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" do not exists in ModifiedConteguous allocation.\n";
            return;
        }
        files.erase(file);
        
        int i = startBlock[file].first;
        startBlock.erase(file);

        while(i != -1){
            int next = diskSpace[i][1];
            diskSpace[i][1] = -1;
            diskSpace[i][0] = -1;
            blankBlocks.insert(i);
            i = next;
        }
        cout<<file<<" is deleted from ModifiedConteguous allocation.\n";
    }
    /*
        Update a file
        @parameter
            file: type string represents file name
            len : type int    represents updated file length
    */
    void Update(File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" can not be updated as it do not exists in ModifiedConteguous allocation.\n";
            return;
        }
        vector<int> contents = f.contents;
        int blocksNeeded = contents.size();
        if(blocksNeeded == 0){ // if the updated length 0 then delete the file
            this->Delete(file);
            return;
        }
        // int blocksNeeded = (len-1)/blockSize + 1;

        int i = startBlock[file].first; // get the starting block
        int prevBlocks = startBlock[file].second;

        if(blocksNeeded <= prevBlocks){ // file size is reduced
            int j = 0;
            startBlock[file].second = blocksNeeded; // updating the block size
            while(--blocksNeeded){
                diskSpace[i][0] = contents[j++];
                i = diskSpace[i][1];
            }
            diskSpace[i][0] = contents[j++];
            int ii = diskSpace[i][1];
            diskSpace[i][1] = -1;
            i = ii;

            while(i != -1){
                diskSpace[i][0] = -1;
                int i_ = diskSpace[i][1];
                diskSpace[i][1] = -1;
                blankBlocks.insert(i);
                i = i_;
            }
        }
        else{ // file size is increasing
            bool index = check(blocksNeeded - prevBlocks);
            if(!index){
                cout<<"Can not update "<<file<<" as not enough blocks are left in ModifiedConteguous allocation.\n";
                return;
            }
            int i = startBlock[file].first, j = 0;
            startBlock[file].second = blocksNeeded;
            while(diskSpace[i][1] != -1){
                diskSpace[i][0] = contents[j++];
                i = diskSpace[i][1];
            }
            while(j < blocksNeeded-1){
                int next = *blankBlocks.begin();
                blankBlocks.erase(next);
                diskSpace[i][0] = contents[j++];
                diskSpace[i][1] = next;
                i = next;
            }
            diskSpace[i][0] = contents[j++];
            diskSpace[i][1] = -1;
        }
        cout<<"The file "<<file<<" is updated in ModifiedConteguous allocation!\n";

    }
    /*
        Read file
        @parameters
            file: type string represents file name
    */
    void ReadFile (File f){
        string file = f.name;
        if(files.find(file) == files.end()){
            cout<<file<<" is not present in ModifiedConteguous allocation.\n";
            return;
        }
        int i = startBlock[file].first; // find the start index of the file
        cout<<"Reading file "<<file<<" from block \n";
        while(diskSpace[i][1] != -1)cout<<diskSpace[i][0]<<" ", i = diskSpace[i][1];
        cout<<endl;
    }
    /*
        To show the current allocation
    */
    void showDisk(){
        for(auto ind: diskSpace){
            cout<<ind[0]<<"\n";
        }
    }
};

int main(){
    int size, blockSize;
    FILE *fp = fopen("input.txt", "r");
    fscanf(fp, "%d %d", &size, &blockSize);

    Contiguous disk1(size, blockSize);
    Linked disk2(size, blockSize);
    Indexed disk3(size, blockSize);
    ModifiedConteguous disk4(size, blockSize);

    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    int s, t;
    string name;
    char n[100];
    vector<int> contents;
    unordered_map<string, File> files;

    while(!feof(fp)){
        int type;
        fscanf(fp, "%d", &type);
        switch(type){
            case 1: // creating the file
                fscanf(fp, "%s", n);
                fscanf(fp, "%d", &s);
                contents.clear();
                for(int i = 0; i< s; ++i){
                    fscanf(fp, "%d", &t);
                    contents.push_back(t);
                }
                files[n] = File(n, contents);
            break;
            case 2: // insert the file into disks
                fscanf(fp, "%s", n);
                if(files.find(n) == files.end()){
                    cout<<"File does not exixts.\n"<<endl;
                }
                else{
                    start = std::chrono::high_resolution_clock::now();
                    disk1.Add(files[n]);
                    stop = std::chrono::high_resolution_clock::now();
                    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                    cout << "Time taken by Insert: " << duration.count() << " microseconds" << std::endl;
                    cout<<"----------------------------------------------\n";
                    start = std::chrono::high_resolution_clock::now();
                    disk2.Add(files[n]);
                    stop = std::chrono::high_resolution_clock::now();
                    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                    cout << "Time taken by Insert: " << duration.count() << " microseconds" << std::endl;
                    cout<<"----------------------------------------------\n";
                    start = std::chrono::high_resolution_clock::now();
                    disk3.Add(files[n]);
                    stop = std::chrono::high_resolution_clock::now();
                    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                    cout << "Time taken by Insert: " << duration.count() << " microseconds" << std::endl;
                    cout<<"----------------------------------------------\n";
                    start = std::chrono::high_resolution_clock::now();
                    disk4.Add(files[n]);
                    stop = std::chrono::high_resolution_clock::now();
                    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                    cout << "Time taken by Insert: " << duration.count() << " microseconds" << std::endl;
                    cout<<"----------------------------------------------\n";
                }
            break;
            case 3: // update file
                fscanf(fp, "%s", n);
                fscanf(fp, "%d", &s);
                contents.clear();
                for(int i = 0; i< s; ++i){
                    fscanf(fp, "%d", &t);
                    contents.push_back(t);
                }
                files[n] = File(n, contents);
                start = std::chrono::high_resolution_clock::now();
                disk1.Update(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by Update: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
                start = std::chrono::high_resolution_clock::now();
                disk2.Update(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by Update: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
                start = std::chrono::high_resolution_clock::now();
                disk3.Update(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by Update: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
                start = std::chrono::high_resolution_clock::now();
                disk4.Update(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by Update: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
            break;
            case 4: // delete a file
                fscanf(fp, "%s", n);
                start = std::chrono::high_resolution_clock::now();
                disk1.Delete(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by delete: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
                start = std::chrono::high_resolution_clock::now();
                disk2.Delete(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by delete: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
                start = std::chrono::high_resolution_clock::now();
                disk3.Delete(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by delete: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
                start = std::chrono::high_resolution_clock::now();
                disk4.Delete(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by delete: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
            break;
            case 5: // show disk
                disk1.showDisk();
                cout<<"----------------------------------------------\n";
                disk2.showDisk();
                cout<<"----------------------------------------------\n";
                disk3.showDisk();
                cout<<"----------------------------------------------\n";
                disk4.showDisk();
                cout<<"----------------------------------------------\n";

            break;
            case 6: // show file
                fscanf(fp, "%s", n);
                start = std::chrono::high_resolution_clock::now();
                disk1.ReadFile(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by ReadFile: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
                start = std::chrono::high_resolution_clock::now();
                disk2.ReadFile(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by ReadFile: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
                start = std::chrono::high_resolution_clock::now();
                disk3.ReadFile(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by ReadFile: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
                start = std::chrono::high_resolution_clock::now();
                disk4.ReadFile(files[n]);
                stop = std::chrono::high_resolution_clock::now();
                duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                cout << "Time taken by ReadFile: " << duration.count() << " microseconds" << std::endl;
                cout<<"----------------------------------------------\n";
            break;
            default:
                exit(0);
            break;
        }
    }
    return 0;
}