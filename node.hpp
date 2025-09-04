#include <vector>
#include <string>

using namespace std;
#pragma once

struct node {
    vector< pair<node*, int> > adj;
    long label;
    string segment;

    bool visited;
    bool open;
    int idx;

    int in_deg, out_deg;

    node(int l, string s) {
        label = l;
        segment = s;

        visited = false;
        open = false;
        idx = 0;

        in_deg = 0;
        out_deg = 0;

        adj.resize(0);
    }
};

struct path {
    vector< pair<int, int> > links;
    string genome;
};
