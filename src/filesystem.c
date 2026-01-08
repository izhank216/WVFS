// Web Virtual Filesystem (WVFS)
// Licensed under the MIT License
//
// MIT License
// Copyright (c) 2026 Izhan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wvfs.h"



#define MAX_NAME 256
#define MAX_CHILDREN 128

typedef struct Node {
    char name[MAX_NAME];
    NodeType type;
    struct Node* parent;
    struct Node* children[MAX_CHILDREN];
    int child_count;
    char* content;    // only for files
    int protected;    // if set, cannot delete/rename/move
    // special hooks for dev/null
    size_t (*write_hook)(const void*, size_t);
    size_t (*read_hook)(void*, size_t);
} Node;

static Node* root = NULL;
static Node* cwd = NULL;

// Create a node
static Node* wvfs_create_node(const char* name, NodeType type, Node* parent) {
    Node* node = (Node*)malloc(sizeof(Node));
    strncpy(node->name, name, MAX_NAME);
    node->type = type;
    node->parent = parent;
    node->child_count = 0;
    node->content = (type == FILE_NODE) ? strdup("") : NULL;
    node->protected = 0;
    node->write_hook = NULL;
    node->read_hook = NULL;
    return node;
}

// Find child by name
static Node* wvfs_find_child(Node* dir, const char* name) {
    for (int i = 0; i < dir->child_count; i++) {
        if (strcmp(dir->children[i]->name, name) == 0) return dir->children[i];
    }
    return NULL;
}

// Initialize WVFS
void wvfs_init() {
    root = wvfs_create_node("/", DIR_NODE, NULL);
    cwd = root;

    // Create protected /dev folder
    Node* dev = wvfs_create_node("dev", DIR_NODE, root);
    dev->protected = 1;
    root->children[root->child_count++] = dev;

    // Create /dev/null
    Node* dev_null = wvfs_create_node("null", FILE_NODE, dev);
    dev_null->write_hook = dev_null_write;
    dev_null->read_hook = dev_null_read;
    dev->children[dev->child_count++] = dev_null;
}

// Directory operations
int wvfs_mkdir(const char* name) {
    if (strcmp(name, "dev") == 0) return -2; // cannot create dev folder manually
    if (cwd->child_count >= MAX_CHILDREN) return -1;
    if (wvfs_find_child(cwd, name)) return -2;
    Node* node = wvfs_create_node(name, DIR_NODE, cwd);
    cwd->children[cwd->child_count++] = node;
    return 0;
}

int wvfs_cd(const char* name) {
    if (strcmp(name, "/") == 0) {
        cwd = root;
        return 0;
    } else if (strcmp(name, "..") == 0) {
        if (cwd->parent) cwd = cwd->parent;
        return 0;
    } else {
        Node* dir = wvfs_find_child(cwd, name);
        if (!dir || dir->type != DIR_NODE) return -1;
        cwd = dir;
        return 0;
    }
}

void wvfs_ls() {
    for (int i = 0; i < cwd->child_count; i++) {
        Node* n = cwd->children[i];
        printf("%s%s\n", n->name, n->type == DIR_NODE ? "/" : "");
    }
}

void wvfs_pwd() {
    Node* tmp = cwd;
    char path[1024] = "";
    while (tmp) {
        char buf[1024];
        snprintf(buf, sizeof(buf), "/%s%s", tmp->parent ? tmp->name : "", path);
        strncpy(path, buf, sizeof(path));
        tmp = tmp->parent;
    }
    printf("%s\n", path[0] ? path : "/");
}

// File operations
int wvfs_touch(const char* name) {
    if (cwd->child_count >= MAX_CHILDREN) return -1;
    if (wvfs_find_child(cwd, name)) return -2;
    Node* node = wvfs_create_node(name, FILE_NODE, cwd);
    cwd->children[cwd->child_count++] = node;
    return 0;
}

int wvfs_write_file(const char* name, const char* data) {
    Node* file = wvfs_find_child(cwd, name);
    if (!file || file->type != FILE_NODE) return -1;
    if (file->write_hook) {
        file->write_hook(data, strlen(data));
        return strlen(data);
    }
    free(file->content);
    file->content = strdup(data);
    return strlen(data);
}

void wvfs_read_file(const char* name) {
    Node* file = wvfs_find_child(cwd, name);
    if (!file || file->type != FILE_NODE) {
        printf("File not found\n");
        return;
    }
    if (file->read_hook) {
        char buf[1024];
        size_t n = file->read_hook(buf, sizeof(buf));
        fwrite(buf, 1, n, stdout);
        return;
    }
    printf("%s\n", file->content);
}
