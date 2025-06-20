#include "vmfs.hpp"

int main()
{
    if (access("disk", F_OK) == -1)
    {
        cout << "Disk does not exist, init it..." << endl;
        init();
    }
    fd = open("disk", O_RDWR, 0644);
    assert(fd > -1);
    vector<pair<size_t, string>> cur; // current directory
    cur.emplace_back(0, "/");
    string cur_path = pwd(cur); // current path
    read_block(1, &bitmap, sizeof(bitmap));
    read_block(2, fat_table, sizeof(fat_table));
    cout << CYAN <<"Welcome to My Virtual File System!😋\n"<< END;

    cout << CYAN << cur_path << " >>>" << END;
    string cmd;
    cin >> cmd;
    while (cmd != "quit" && cmd != "q")
    {
        if (cmd == "ls")
        {
            ls(cur.back().first);
        }
        else if (cmd == "cd")
        {
            string path;
            cin >> path;
            cd(path, cur, cur_path);
        }
        else if (cmd == "pwd")
        {
            cout << cur_path;
        }
        else if(cmd == "mkdir"){
            string dir_path;
            cin>>dir_path;
            create(dir_path, cur, 1);
        }
        else if(cmd == "touch"){
            string file_path;
            cin>>file_path;
            create(file_path, cur, 0);
        }
        else if(cmd == "cat"){
            string file_path;
            cin>>file_path;
            cat(file_path,cur);
        }
        else if(cmd == "rm"){
            string next_arg;
            cin >> next_arg;
            if (next_arg == "-r") {
                string file_path;
                cin >> file_path;
                size_t offset = findRmBlock(file_path, cur, 1);
                rm_r(offset);
            } 
            else {
                size_t offset = findRmBlock(next_arg, cur, 0);
                rm(offset);
            }
        }
        else if(cmd == "man"){
            cout<<
            "ls: list files in current directory -> ls\n"
            "cd: change directory -> cd examplePath\n"
            "pwd: print current directory -> pwd\n"
            "mkdir: create a directory mkdir exampleDir -> mkdir exampleDir\n"
            "touch: create a file touch exampleFile -> touch exampleFile\n"
            "cat: print file content cat exampleFile -> cat exampleFile\n"
            "rm: remove a file rm exampleFile -> rm exampleFile\n"
            "rm -r: remove a directory rm -r exampleDir -> rm -r exampleDir\n"
            "quit: quit this program -> quit\n"
            "q: quit this program -> q\n"
            "man: print this help -> man\n"
            <<endl;
        }
        else{
            cin.ignore(999,'\n');
            cout<<RED<<"Unknown command: "<<cmd<<END<<endl;
        }
        cout << "\n"<< CYAN << cur_path << " >>>" << END;
        cin >> cmd;
    }
    close(fd);
    cout << "Goodbye!😋";

    // unlink("disk"); //debug
    return 0;
}
