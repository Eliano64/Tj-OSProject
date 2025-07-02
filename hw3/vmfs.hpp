#ifndef VMFS_HPP
#define VMFS_HPP
#include <bits/stdc++.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;

#define KB 1024UL
#define BLOCK (4 * KB)
#define FAT_END 0xFFFFFFFF
#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define END "\033[0m"
#define RED "\033[31m"
#define STRONG "\033[1m"

uint64_t bitmap = 0;
size_t fat_table[64] = {0xFFFFFFFF};
int fd = -1;

struct DirectoryEntry
{
    char name[32];        // 文件/目录名
    size_t first_block; // FAT中起始块号(计算物理地址)
    size_t size;        // 文件大小（字节）, 目录可设置为0
    uint8_t is_directory; // 是否为目录
};

size_t write_block(int block_num, const void *buffer, size_t size)
{
    assert(lseek(fd, (block_num)*BLOCK, SEEK_SET) != -1);
    size_t bytes_written = write(fd, buffer, size);
    assert(bytes_written == size);
    return bytes_written;
}

int init()
{
    fd = open("disk", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    assert(fd > -1);
    // Create a 64*4KB file
    const int size = BLOCK;
    char buffer[size];
    memset(buffer, 0, size);
    for (int i = 0; i < 64; ++i)
    {
        int bytes_written = write(fd, buffer, size);
        assert(bytes_written == size);
    }

    bitmap |= 1;
    bitmap |= (1 << 1);
    bitmap |= (1 << 2);
    assert(write_block(1, &bitmap, sizeof(bitmap)) == sizeof(bitmap));

    fat_table[0] = FAT_END;
    assert(write_block(2, fat_table, sizeof(fat_table)) == sizeof(fat_table));

    DirectoryEntry ROOT = {".", 0, 0, 1}; // root directory
    DirectoryEntry PARENT = {"..", 0, 0, 1}; // root has no parent directory, but for convenience, we define it is itself
    DirectoryEntry *root_dir_block_buffer = (DirectoryEntry *)malloc(BLOCK);
    memset(root_dir_block_buffer, 0, BLOCK);
    memcpy(root_dir_block_buffer, &ROOT, sizeof(DirectoryEntry));
    memcpy(root_dir_block_buffer + 1, &PARENT, sizeof(DirectoryEntry));
    assert(write_block(0, root_dir_block_buffer, BLOCK) == BLOCK);
    free(root_dir_block_buffer);
    close(fd);
    return 0;
}

int read_block(size_t block_num, void *buffer, size_t size)
{
    assert(lseek(fd, (block_num)*BLOCK, SEEK_SET) != -1);
    int bytes_read = read(fd, buffer, size);
    return bytes_read;
}

int ls(size_t block_num)
{
    lseek(fd, block_num * BLOCK, SEEK_SET);
    DirectoryEntry *dir = (DirectoryEntry *)malloc(BLOCK);
    read(fd, dir, BLOCK);
    for (int i = 0; i < BLOCK / sizeof(DirectoryEntry); ++i)
    {
        if (dir[i].name[0] != '\0')
        {
            cout << (dir[i].is_directory ? BLUE : "") << dir[i].name << (dir[i].is_directory ? END : "") << " ";
        }
    }
    free(dir);
    return 0;
}

string pwd(vector<pair<size_t, string>> cur)
{
    string path = "";
    for (int i = 0; i < cur.size(); ++i)
    {
        path += cur[i].second;
        if (i != cur.size() - 1 && i != 0){
            path += "/";
        }
    }
    return path;
}

int cd(string &path, vector<pair<size_t, string>> &cur, string &cur_path) {
    vector<string> path_list;
    stringstream ss(path);
    string tmp;
    while (getline(ss, tmp, '/')) {
        if (!tmp.empty()) path_list.push_back(tmp);
    }

    vector<pair<size_t, string>> new_path;

    if (path[0] == '/') {
        // absolute path
        new_path.emplace_back(0, "/");
    } else {
        // relative path
        new_path = cur;
    }

    for (const auto& name : path_list) {
        if (name == ".") {
            continue;
        } else if (name == "..") {
            if (new_path.size() > 1) new_path.pop_back();
        } else {
            // 进入子目录
            DirectoryEntry *dir_block = (DirectoryEntry *)malloc(BLOCK);
            read_block(new_path.back().first, dir_block, BLOCK);
            bool found = false;
            for (int j = 0; j < BLOCK / sizeof(DirectoryEntry); ++j) {
                if (strcmp(dir_block[j].name, name.c_str()) == 0 && dir_block[j].is_directory) {
                    new_path.emplace_back(dir_block[j].first_block, name);
                    found = true;
                    break;
                }
            }
            free(dir_block);
            if (!found) {
                cout << RED << "No such directory: " << STRONG<<name << END<<END << endl;
                return -1;
            }
        }
    }

    cur = new_path;
    cur_path = pwd(cur);
    return 0;
}


void input(string &full_input) {
    string line;
    // 清除输入缓冲区中可能残留的换行符
    std::cin.ignore(2147483647, '\n'); 
    cout << CYAN<<"\nEnter text (Enter for new line, press Enter on an empty line to finish):\n"<<END;
    while (true) {
        cout<<CYAN<<"(input)>>> "<<END;
        getline(cin, line);
        if (line.empty()) { 
            break;
        }
        full_input += line + "\n"; 
    }

    if (!full_input.empty()) {
        full_input.pop_back();
    }
    std::cin.clear(); 
}

int create(string &file_path, vector<pair<size_t, string>> cur, uint8_t is_directory){
    string path = file_path.substr(0,file_path.rfind('/'));
    if(path == ""){
        path = "/";
    }
    string _ = "";
    int status = cd(path,cur,_);
    if(status != 0){
        return -1;
    }
    bitset<64> bt(bitmap);
    size_t idx = 0;
    int flag = -1;
    for(;idx<bt.size();++idx){
        if(!bt[idx]){
            flag = 0;
            break;
        }
    }
    if(flag == -1){
        cout<<RED<<"No enough space"<<END<<endl;
        return -1;
    }
    bt[idx]=1;
    if(is_directory){// create directory
        DirectoryEntry *parentDir = (DirectoryEntry *)malloc(BLOCK);
        read_block(cur.back().first, parentDir, BLOCK);
        for(int i = 0;i<BLOCK/sizeof(DirectoryEntry);++i){
            if(strcmp(parentDir[i].name, file_path.substr(file_path.rfind('/')+1).c_str())== 0){
                cout<<RED<<"A directory or file with the same name already exists"<<END<<endl;
                free(parentDir);
                return -1;
            }
        }

        DirectoryEntry *dir = (DirectoryEntry *)malloc(BLOCK);
        memset(dir,0,BLOCK);
        DirectoryEntry ROOT = {".", idx, 0, 1};
        DirectoryEntry PARENT = {"..", cur.back().first, 0, 1};
        memcpy(dir, &ROOT, sizeof(DirectoryEntry));
        memcpy(dir + 1, &PARENT, sizeof(DirectoryEntry));
        write_block(idx,dir,BLOCK);
        free(dir);
        for(int i = 0;i<BLOCK/sizeof(DirectoryEntry);++i){
            if(parentDir[i].name[0] == '\0'){
                DirectoryEntry *newDir = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
                strcpy(newDir->name, file_path.substr(file_path.rfind('/')+1).c_str());
                newDir->is_directory = 1;
                newDir->first_block = idx;
                newDir->size = 0;
                memcpy(parentDir + i, newDir, sizeof(DirectoryEntry));
                free(newDir);
                break;
            }
        }
        write_block(cur.back().first, parentDir, BLOCK);
        free(parentDir);
    }
    else{// create file
        DirectoryEntry *parentDir = (DirectoryEntry *)malloc(BLOCK);
        read_block(cur.back().first, parentDir, BLOCK);
        for(int i = 0;i<BLOCK/sizeof(DirectoryEntry);++i){
            if(strcmp(parentDir[i].name, file_path.substr(file_path.rfind('/')+1).c_str())== 0){
                cout<<RED<<"A directory or file with the same name already exists"<<END<<endl;
                free(parentDir);
                return -1;
            }
        }
        string full_input;
        input(full_input); 
        int file_size = full_input.length();
        int num_blocks = (file_size + BLOCK - 1) / BLOCK;
        vector<int> blocks;
        blocks.push_back(idx);

        // 分配剩余块并更新 FAT、bitmap
        for (int b = 1; b < num_blocks; ++b) {
            int next_block = -1;
            for (int i = 0; i < 64; ++i) {
                if (!bt[i]) {
                    next_block = i;
                    break;
                }
            }
            if(next_block == -1){
                cout<<RED<<"No enough space"<<END<<endl;
                return -1;
            }
            fat_table[blocks.back()] = next_block;
            fat_table[next_block] = FAT_END;
            bt[next_block] = 1;
            blocks.push_back(next_block);
        }

        // 写入文件内容到块
        for (int i = 0; i < blocks.size(); ++i) {
            string block_data = full_input.substr(i * BLOCK, BLOCK);
            write_block(blocks[i], block_data.c_str(), block_data.size());
        }

        // 写入目录项
        for (int i = 0; i < BLOCK / sizeof(DirectoryEntry); ++i) {
            if (parentDir[i].name[0] == '\0') {
                strcpy(parentDir[i].name, file_path.substr(file_path.rfind('/') + 1).c_str());
                parentDir[i].is_directory = 0;
                parentDir[i].first_block = blocks[0];
                parentDir[i].size = file_size;
                break;
            }
        }
        write_block(cur.back().first, parentDir, BLOCK);
        free(parentDir);
    }
    bitmap = (uint64_t)(bt.to_ullong());
    assert(write_block(1, &bitmap, sizeof(bitmap)) == sizeof(bitmap));
    assert(write_block(2, fat_table, sizeof(fat_table)) == sizeof(fat_table));
    return 0;
}

string read_file(size_t first_block, size_t file_size){
    size_t next_block = fat_table[first_block];
    size_t bytes_read_so_far = 0;
    char * buffer = (char *)malloc(BLOCK);
    read_block(first_block, buffer,BLOCK);
    size_t bytes_to_read_from_this_block = min((size_t)BLOCK, file_size - bytes_read_so_far);
    string out(buffer, bytes_to_read_from_this_block);
    bytes_read_so_far += bytes_to_read_from_this_block;
    free(buffer);
    while(next_block!=FAT_END){
        char * buffer1 = (char *)malloc(BLOCK);
        read_block(next_block, buffer1,BLOCK);
        size_t bytes_to_read_from_this_block = min((size_t)BLOCK, file_size - bytes_read_so_far);
        out.append(buffer1, bytes_to_read_from_this_block);
        bytes_read_so_far += bytes_to_read_from_this_block;
        free(buffer1); // Free buffer1 after its use in the current iteration
        if (bytes_read_so_far >= file_size) break; // Stop if all file content is read
        next_block = fat_table[next_block];
    }
    return out;
}

void output(string &out,string name){

    vector<string> out_list;
    stringstream ss(out);
    string tmp;
    while(getline(ss, tmp, '\n')){
        out_list.push_back(tmp);
    }

    cout<<CYAN<<"below is the content of the file:"<<STRONG<<name<<END<<"\n\n"<<END;
    int line_count = out_list.size();
    int max_digits = 0;
    if (line_count > 0) {
        max_digits = to_string(line_count - 1).length();
    }
    if (max_digits == 0) { // Handle case with 0 or 1 line, or if all lines are empty and filtered out previously
        max_digits = 1;
    }

    for(int i = 0; i < out_list.size(); ++i){
        cout << CYAN << setw(max_digits) << i << "| " << out_list[i] << END << "\n";
    }
}

int cat(string &file_path, vector<pair<size_t, string>> cur){
    string path;
    string file_name;
    size_t last_slash_pos = file_path.rfind('/');

    if (last_slash_pos == string::npos) { // No slash in path, file is in current dir
        path = cur.back().second; 
        file_name = file_path;
    } else if (last_slash_pos == 0) { // Path starts with '/', e.g., "/file.txt"
        path = "/";
        file_name = file_path.substr(1);
    } else { // Path like "dir/file.txt" or "/dir/file.txt"
        path = file_path.substr(0, last_slash_pos);
        file_name = file_path.substr(last_slash_pos + 1);
    }

    string _ = ""; 
    int status = cd(path, cur, _);
    if (status != 0) {
        return -1;
    }

    DirectoryEntry *dir_entries = (DirectoryEntry *)malloc(BLOCK);
    read_block(cur.back().first, dir_entries, BLOCK);

    int find = 0;
    for (int i = 0; i < BLOCK / sizeof(DirectoryEntry); ++i) {
        if (strcmp(dir_entries[i].name, file_name.c_str()) == 0 && dir_entries[i].is_directory == 0) {
            string out = read_file(dir_entries[i].first_block, dir_entries[i].size);
            output(out,string(dir_entries[i].name));
            find = 1;
            break;
        }
    }

    if (!find) {
        cout << RED << "cat: '" << file_path << "': No such file\n" << END;
    }

    free(dir_entries);
    return -find; 
}

void rm(size_t fileFirstBlock){
    size_t current = fileFirstBlock;
    while (current != FAT_END) {
        size_t next = fat_table[current];
        
        // 清空数据块
        char zero[BLOCK] = {0};
        write_block(current, zero, BLOCK);

        // 清空 FAT 和 bitmap
        fat_table[current] = FAT_END;
        bitmap &= ~(1ULL << current);

        current = next;
    }

    // 更新 FAT 和 bitmap 到磁盘
    write_block(1, &bitmap, sizeof(bitmap));
    write_block(2, fat_table, sizeof(fat_table));
}


void rm_r(size_t rmDirBlock){
    DirectoryEntry *rmDir = (DirectoryEntry *)malloc(BLOCK);
    read_block(rmDirBlock, rmDir, BLOCK);
    for(int i = 0;i<BLOCK/sizeof(DirectoryEntry);++i){
        if(rmDir[i].name[0] == '\0') continue;
        if(rmDir[i].is_directory && strcmp(rmDir[i].name,".")!=0 && strcmp(rmDir[i].name,"..")!=0){
            rm_r(rmDir[i].first_block);
        }
       if(rmDir[i].is_directory == 0){
            rm(rmDir[i].first_block);
       }
        memset(&rmDir[i], 0, sizeof(DirectoryEntry));
    }
    
    bitmap &= ~(1ULL << rmDirBlock);
    fat_table[rmDirBlock] = FAT_END;

    write_block(rmDirBlock, rmDir, BLOCK);
    write_block(2, fat_table, sizeof(fat_table));
    write_block(1, &bitmap, sizeof(bitmap));
    free(rmDir);
}

size_t findParentDirBlock(string &file_path, vector<pair<size_t, string>> cur){
    string path;
    size_t last_slash_pos = file_path.rfind('/');

    if (last_slash_pos == string::npos) { // No slash in path, file is in current dir
        path = cur.back().second; 
    } else if (last_slash_pos == 0) { // Path starts with '/', e.g., "/file"
        path = "/";
    } else { // Path like "dir/file" or "/dir/file"
        path = file_path.substr(0, last_slash_pos);
    }

    string _ = ""; 
    cd(path, cur, _);
    return cur.back().first;
}

size_t findRmBlock(string &file_path, vector<pair<size_t, string>> cur, uint8_t is_dir){
    size_t parentBlock = findParentDirBlock(file_path, cur);
    DirectoryEntry *parentDir = (DirectoryEntry *)malloc(BLOCK);
    read_block(parentBlock, parentDir, BLOCK);
    string rmName;
    size_t last_slash_pos = file_path.rfind('/');
    if (last_slash_pos == string::npos) { // No slash in path, file is in current dir
        rmName = file_path;
    } else { // Path like "dir/file" or "/dir/file"
        rmName = file_path.substr(last_slash_pos + 1);
    }
    size_t rmBlock = FAT_END;
    for (int i = 0; i < BLOCK / sizeof(DirectoryEntry); ++i) {
        if (strcmp(parentDir[i].name, rmName.c_str()) == 0 && parentDir[i].is_directory == is_dir) {
            rmBlock = parentDir[i].first_block;
            memset(&parentDir[i], 0, sizeof(DirectoryEntry));
            break;
        }
    }
    if(rmBlock == FAT_END){
        cout << RED <<(is_dir == 1?"No such directory: ":"No such file: ") <<STRONG<<rmName<<'\n'<<END<< END;
    }
    write_block(parentBlock, parentDir, BLOCK);
    free(parentDir);
    return rmBlock;
}

#endif //VMFS_HPP
