#include <iostream>
#include <vector>
#include <string>

#include "node.hpp"
#include "graph.hpp"

using namespace std;

int main() {
    char ans = 'n';

    string file_name = "example.gfa";
    cout << endl << "input data file name: ";
    cin >> file_name;
    
    graph pangenome(file_name);
    cout << endl << "print graph (y/n)? ";
    cin >> ans;
    if (ans == 'y')
        pangenome.print_graph();

    pangenome.decyclify();
    cout << endl << "print acyclic graph (y/n)? ";
    cin >> ans;
    if (ans == 'y')
        pangenome.print_graph();
    
    cout << endl << "print sources and destinations (y/n)? ";
    cin >> ans;
    if (ans == 'y')
        pangenome.print_extr();
    
    int n = 42;
    cout << endl << "number of genomes: ";
    cin >> n;

    long dest, sour = 0;
    cout << endl << "source: ";
    cin >> sour;
    cout << endl << "destination: ";
    cin >> dest;

    pangenome.find_genomes(n, sour, dest);
    cout << endl << "print genomes (y/n)? ";
    cin >> ans;
    if (ans == 'y')
        pangenome.print_genomes();

    string pattern = "A";
    cout << endl << "input pattern to find in genomes: ";
    cin >> pattern;

    pangenome.find_pattern(pattern);
    cout << endl << "print pattern positions (y/n)? ";
    cin >> ans;
    if (ans == 'y')
        pangenome.print_pattern_pos();

    return 0;
}

