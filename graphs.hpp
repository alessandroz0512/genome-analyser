#include <string>
#include <unordered_map>

#include "node.hpp"

using namespace std;
#pragma once

class graph {
    public:
        graph(const string& file_name);

        void decyclify();
        void find_genomes(const int n, long new_source, long new_destination);

        void find_pattern(const string& pattern);

        void print_graph();
        void print_genomes();
        void print_pattern_pos();
        void print_extr();
        vector<node*> sources, destinations, singles;

    private:
        long source, destination;
        unordered_map<long, node*> nodes;
        vector<path> genomes;
        vector< vector<int> > positions;
        
        

        void add_node(const long& label, const string& segment);
        void add_edge(long from_label, const char& from_invert, long to_label, const char& to_invert, const int& overlap);

        void decyclify_from(node* root);
        void add_genes_from(node* current, const int n);

        bool overlapping(string first, string second, const int& overlap);
        string invert(const string& inv);
        unsigned long long to_uns_long(char c);
};
