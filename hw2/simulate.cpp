#include <bits/stdc++.h>
using namespace std;

struct node
{
    int space;
    bool idle;
    node *next, *pre;
    node(int s,bool idle) : space(s), next(NULL), idle(idle), pre(NULL) {}
    node(int s) : space(s), next(NULL), idle(false), pre(NULL) {}
    node() : space(0), idle(false), next(NULL), pre(NULL) {}
};

node *memory_ff = new node(640,true); // FF专用内存
node *head_ff = new node;
map<int, node *> record_ff;

node *memory_bf = new node(640,true); // BF专用内存
node *head_bf =  new node;
map<int, node *> record_bf;

void showSpace(node* head)
{
    int id = 0;
    for (node *p = head->next; p != NULL; p = p->next)
    {
        cout << id++ << ": " << p->space << " K " << (p->idle ? "free" : "occupied") << '\n';
    }
    cout << endl;
}

void firstFit(int id, int task)
{
    for (node *p = head_ff->next; p != NULL; p = p->next)
    {
        if (p->idle && p->space >= task)
        {
            p->space -= task;
            if (p->space == 0)
            {
                p->idle = false;
                record_ff[id] = p;
            }
            else
            {
                node *tmp = new node(task);
                tmp->next = p;
                tmp->pre = p->pre;
                p->pre->next = tmp;
                p->pre = tmp;
                record_ff[id] = tmp;
            }
            break;
        }
    }
    showSpace(head_ff);
}

void bestFit(int id, int task)
{
    node *min = new node(641);
    for (node *p = head_bf->next; p != NULL; p = p->next)
    {
        if (p->idle && p->space >= task)
        {
            min = p->space < min->space ? p : min;
        }
    }
    min->space -= task;
    if (min->space == 0)
    {
        min->idle = false;
        record_bf[id] = min;
    }
    else
    {
        node *tmp = new node(task);
        tmp->next = min;
        tmp->pre = min->pre;
        if(min->pre)min->pre->next = tmp;
        min->pre = tmp;
        record_bf[id] = tmp;
    }
    showSpace(head_bf);
}

void memoryManage(int id, int task, int type)
{
    if (type == 0)
    {
        firstFit(id, task);
        bestFit(id, task);
    }
    else
    {
        // 释放FF内存
        if(record_ff.find(id) != record_ff.end()) {
            node *p = record_ff[id];
            if (p->next != NULL && p->next->idle)
            {
                node *tmp = p->next;
                p->space += tmp->space;
                p->next = tmp->next;
                if (p->next != NULL)
                    p->next->pre = p;
                delete tmp;
            }
            if (p->pre != NULL && p->pre->idle)
            {
                node *tmp = p->pre;
                tmp->space += p->space;
                tmp->next = p->next;
                if (p->next != NULL)
                    p->next->pre = tmp;
                delete p;
                p = tmp;
            }
            p->idle = true;
            record_ff.erase(id);
            showSpace(head_ff);
        }

        // 释放BF内存
        if(record_bf.find(id) != record_bf.end()) {
            node *p = record_bf[id];
            if (p->next != NULL && p->next->idle)
            {
                node *tmp = p->next;
                p->space += tmp->space;
                p->next = tmp->next;
                if (p->next != NULL)
                    p->next->pre = p;
                delete tmp;
            }
            if (p->pre != NULL && p->pre->idle)
            {
                node *tmp = p->pre;
                tmp->space += p->space;
                tmp->next = p->next;
                if (p->next != NULL)
                    p->next->pre = tmp;
                delete p;
                p = tmp;
            }
            p->idle = true;
            record_bf.erase(id);
            showSpace(head_bf);
        }
    }
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    head_bf->next = memory_bf;
    memory_bf->pre = head_bf;
    head_ff->next = memory_ff;
    memory_ff->pre = head_ff;
    int n;
    cin >> n;
    while (n--)
    {
        int id, task, type;
        cin >> id >> task >> type;
        memoryManage(id, task, type);
    }
}