#include <fstream>
#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <chrono>

#include "node.hpp"
#include "graph.hpp"

using namespace std::chrono;

graph::graph(const string& file_name) {
    
    auto t = duration_cast< microseconds >(system_clock::now().time_since_epoch()).count();
    cout << endl << "making graph...";

    //segments and links are mixed, some links precede the segment calls
    //simple (but twice as slow) solution: read the file twice, once getting segments, once getting links

    ifstream in;
    in.open(file_name.c_str());

    char c;
    long label, l1, l2, overlap;
    char i1, i2;
    string segment, temp;

    bool flag = true;
    while (in >> c && flag) {           //first read: get the segments
        
        switch (c) {
            case 'S':
                in >> label >> segment;
                add_node(label, segment);
                break;

            case 'P':                   //DP:i tags after segments: we ignore those. P for paths: we stop
                in >> c;
                if (c != ':')
                    flag = false;
                break;
        }

    }

    in.close();
    in.clear();
    in.open(file_name.c_str());

    flag = true;
    while (in >> c && flag) {           //second read: get the links
        
        switch (c) {
            case 'P':                   //DP:i tags after segments: we ignore those. P for paths: we stop
                in >> c;
                if (c != ':')
                    flag = false;
                break;

            case 'L':
                in >> l1 >> i1 >> l2 >> i2 >> temp;         //L l1 +/- l2 +/- *
                if (temp[temp.length() - 1] == 'M') {       //if overlap is specified, it comes as integer + M
                    temp.resize(temp.length() - 1);
                    overlap = stoi(temp);
                }
                else                                        //otherwise we assume it's 0
                    overlap = 0;
                    
                add_edge(l1, i1, l2, i2, overlap);
                break;
        }

    }
    in.close();
    in.clear();

    genomes.resize(0);
    sources.resize(0);
    destinations.resize(0);
    singles.resize(0);

    t = duration_cast< microseconds >(system_clock::now().time_since_epoch()).count() - t;
    cout << endl << "graph made in " << t << " us";
}

void graph::add_node(const long& label, const string& segment) {
    auto it = nodes.find(label);
    if (it == nodes.end()) {                        //add node anly if it doesn't already exist
        nodes[label] = new node(label, segment);
    }
}

void graph::add_edge(long from_label, const char& from_invert, long to_label, const char& to_invert, const int& overlap) {
    auto temp_from = nodes.find(from_label);
    auto temp_to = nodes.find(to_label);
    
    if (temp_from != nodes.end() && temp_to != nodes.end()) {               //check that segments do exist

        if (from_invert == '-') {
            add_node(-from_label, invert(temp_from->second->segment));      //if inverted, create new node with new_label = -old_label
            from_label = -from_label;
        }
        if (to_invert == '-') {
            add_node(-to_label, invert(temp_to->second->segment));          //if inverted, create new node with new_label = -old_label
            to_label = -to_label;
        }

        node *from = nodes.find(from_label)->second;
        node *to = nodes.find(to_label)->second;

        if (overlap > from->segment.length() || overlap > to->segment.length())     //check overlap is shorter than segments
            cout << endl << "overlap maggiore della lunghezza dei segmenti";
        else if ( !overlapping(from->segment, to->segment, overlap) )               //check if the overlap is true
            cout << endl << "overlap falso";
        else {
            pair<node*, int> edge = make_pair(to, overlap);                         //add edge to node adjacency list
            from->adj.push_back(edge);
        }
    }
    else {
        cout << endl << "nodi dell'arco non esistono";
    }
}

void graph::print_graph() {
    cout << endl;
    for (auto it = nodes.begin(); it != nodes.end(); it++) {
        cout << it->first << "\t" << it->second->segment << "\t:\t";

        for (auto x : it->second->adj) {
            cout << "("<< x.first->label << "," << x.second << "), ";
        }
        cout << endl;
    }
    cout << endl;
}

void graph::print_genomes() {
    cout << endl;
    for (auto gen : genomes) {
        for (auto link : gen.links) {
            cout << link.first << ", ";
        }
        cout << endl << gen.genome << endl;
    }
}

void graph::print_extr() {
    cout << endl << "sources:\t";
    for (auto x : sources)
        cout << x->label << "\t";
    cout << endl << "destinations:\t";
    for (auto x : destinations)
        cout << x->label << "\t";
}

void graph::print_pattern_pos() {
    for (int i = 0; i < positions.size(); i++) {
        cout << endl << i + 1 << "\t->\t" << positions[i].size() << "\t:";
        for (auto y : positions[i])
            cout << "\t" << y;
    }
}

void graph::decyclify() {

    auto t = duration_cast< microseconds >(system_clock::now().time_since_epoch()).count();
    cout << endl << "making graph acyclic...";

    for (auto it = nodes.begin(); it != nodes.end(); it++) {            //set all nodes to unvisited and closed
        it->second->visited = false;
        it->second->open = false;
        it->second->idx = 0;                                            //dfs will start checking from first adjacent node
        it->second->in_deg = 0;
        it->second->out_deg = 0;
    }

    for (auto it = nodes.begin(); it != nodes.end(); it++) {            //start dfs from all nodes that haven't been visited to get all connected components
        if ( !it->second->visited && it->second->adj.size() > 0 ) {
            source = it->first;                                         //the starting point of the last dfs search is necessarily a source (after back edge removal)
	        decyclify_from(it->second);                                 //"recursive" dfs call
	    }
    }
    destination = source;
    for (auto it = nodes.begin(); it != nodes.end(); it++) {
        if (it->second->in_deg == 0 && it->second->out_deg != 0)
            sources.push_back(it->second);
        else if (it->second->in_deg != 0 && it->second->out_deg == 0)
            destinations.push_back(it->second);
        else if (it->second->in_deg == 0 && it->second->out_deg == 0)
            singles.push_back(it->second);
    }

    t = duration_cast< microseconds >(system_clock::now().time_since_epoch()).count() - t;
    cout << endl << "graph made acyclic in " << t << " us";
}

void graph::decyclify_from(node* root) {        //iterative dfs istead of recursive dfs for stack size reasons
    stack<node*> stack;                         //using a stack structure means the algorithm is the same, but it takes place on the heap
    stack.push(root);

    while (!stack.empty()) {
        node* root = stack.top();
                    
        root->visited = true;                           //we visit the node
        root->open = true;                              //open -> we are looking at the decendants of this node

        bool flag = true;
        int i = root->idx;                              //we need to restart the iteration at the same point we left it at
        while (i < root->adj.size()) {
            if (root->adj[i].first->open)          //remove back edges, i doesn't advance
                root->adj.erase(root->adj.begin() + i);
            else {
                root->adj[i].first->in_deg++;
                root->out_deg++;
                if (!root->adj[i].first->visited) {
                    stack.push(root->adj[i].first);

                    root->idx = i + 1;
                    i = root->adj.size();                   //escape loop to move to child
                    flag = false;
                }
                else
                    i++;
            }
        }

        if (flag) {                                     //flag = false -> we are done visiting all decendants
            stack.pop();
            root->open = false;                         //we close the node when we have visited all decendants
        }
    }
}

void graph::find_genomes(const int n, long new_source = 0l, long new_destination = 0l) {

    auto t = duration_cast< microseconds >(system_clock::now().time_since_epoch()).count();
    cout << endl << "finding genomes...";

    if (new_source != 0l) {
        source = new_source;
        destination = source;
    }
    if (new_destination != 0l)
        destination = new_destination;

    for (auto it = nodes.begin(); it != nodes.end(); it++) {
        it->second->visited = false;
        it->second->open = false;                               //open will be used as a "has a path to the destination" indicator
        it->second->idx = 0;
    }

    genomes.resize(1);
    genomes[0].genome = "";
    genomes[0].links = {make_pair(source, 0)};

    add_genes_from(nodes.find(source)->second, n);              //"recursive" dfs call

    genomes.pop_back();                                         //last genome is either n+1 or empty
    for (int i = 0; i < genomes.size(); i++) {
        for (int j = 0; j < genomes[i].links.size(); j++) {
            genomes[i].genome.append(nodes.find(genomes[i].links[j].first)->second->segment.substr(genomes[i].links[j].second));
            //we construct strings separately for time efficiency
        }
    }

    t = duration_cast< microseconds >(system_clock::now().time_since_epoch()).count() - t;
    cout << endl << genomes.size() << " genomes found in " << t << " us";
}

void graph::add_genes_from(node* root, const int n) {
    stack<node*> stack;
    stack.push(root);

    while (!stack.empty()) {
        root = stack.top();
        root->visited = true;

        if ( destination == source && root->adj.size() == 0 ) {         //if a destination has not been found and
            destination = root->label;                                  //this is a dead end, then this is the destination
            root->open = true;                                          //destination has a path to destination
        }
        if (root->idx > 0)                                                      //if this isn't the first time we visit, we check
            root->open = root->open || root->adj[root->idx - 1].first->open;    //if last visited child has path to destination
        
        if ( root->label == destination && root->label != source ) {    //if this is destination, copy genome and start working on the copy
            genomes.push_back(genomes[genomes.size() - 1]);
            root->open = true;
        }

        bool flag = true;
        int i = root->idx;
        while (i < root->adj.size() && genomes.size() <= n) {
            node* next = root->adj[i].first;
            int overlap = root->adj[i].second;

            if ( !next->visited || next->open ) {                       //explore a node only if it's new or has a path to destination
                genomes[genomes.size() - 1].links.push_back(make_pair(next->label, overlap));
                stack.push(next);
                
                root->idx = i + 1;
                i = root->adj.size();
                flag = false;
            }
            else
                i++;
        }

        if (flag) {
            root->idx = 0;
            stack.pop();
            genomes[genomes.size() - 1].links.pop_back();   //remove node from working path after we have explored all down-stream paths
        }
    }
}
const unsigned long long mod = 1e9 + 7;
unsigned long long char_to_hash_value(char c) {
    return static_cast<unsigned long long>(c - 'A'); // Supponendo che i caratteri siano tra 'A' e 'Z'
}
void find_pattern(const string &pattern, const vector<string> &genomes) {
    auto start_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    cout << endl;

    vector<vector<int>> positions(genomes.size()); // Vettore di posizioni per ogni genoma
    int l = pattern.length();
    unsigned long long pattern_hash = char_to_hash_value(pattern[l - 1]), pow_5 = 1ull;

    // Costruiamo l'hash del pattern
    for (int i = 1; i < l; i++) {
        pow_5 = (pow_5 * 5ull) % mod;
        pattern_hash = (pattern_hash + pow_5 * char_to_hash_value(pattern[l - 1 - i])) % mod;
    }

    unsigned long long last_genome_suffix_hash = 0;
    int overlap_length = 0;

    // Per ogni genoma
    for (int g = 0; g < genomes.size(); g++) {
        const string &genome = genomes[g];
        if (genome.size() < l) continue; // Saltiamo se il genoma è più corto del pattern

        unsigned long long roll_hash = 0;
        pow_5 = 1ull;

        // Se c'è una sovrapposizione tra il genoma precedente e l'attuale
        if (g > 0 && overlap_length > 0) {
            // Usa l'hash della fine di g-1 per inizializzare l'hash rolling di g
            roll_hash = last_genome_suffix_hash;
            pow_5 = 1;
            for (int i = overlap_length; i < l; i++) {
                pow_5 = (pow_5 * 5ull) % mod;
                roll_hash = (roll_hash + pow_5 * char_to_hash_value(genome[i])) % mod;
            }
        } else {
            // Calcoliamo l'hash rolling per la prima finestra se non c'è sovrapposizione
            roll_hash = char_to_hash_value(genome[l - 1]);
            pow_5 = 1ull;

            for (int i = 1; i < l; i++) {
                pow_5 = (pow_5 * 5ull) % mod;
                roll_hash = (roll_hash + pow_5 * char_to_hash_value(genome[l - 1 - i])) % mod;
            }
        }

        unordered_map<int, bool> seen_positions; // Usare una mappa per memorizzare le posizioni viste

        // Verifica iniziale
        if (roll_hash == pattern_hash) {
            bool equal = true;
            for (int j = 0; j < l; j++) {
                if (pattern[j] != genome[j]) {
                    equal = false;
                    break;
                }
            }
            if (equal) {
                positions[g].push_back(0);
                seen_positions[0] = true; // Memorizza la posizione 0 come già vista
            }
        }

        // Aggiorniamo l'hash rolling per le altre finestre
        for (int i = l; i < genome.size(); i++) {
            roll_hash = (5ull * (roll_hash + mod - char_to_hash_value(genome[i - l]) * pow_5 % mod) + char_to_hash_value(genome[i])) % mod;

            // Controlla se l'hash corrisponde e se la posizione non è già stata vista
            if (roll_hash == pattern_hash && seen_positions.find(i - l + 1) == seen_positions.end()) {
                bool equal = true;
                for (int j = 0; j < l; j++) {
                    if (pattern[j] != genome[i - l + 1 + j]) {
                        equal = false;
                        break;
                    }
                }
                if (equal) {
                    positions[g].push_back(i - l + 1);
                    seen_positions[i - l + 1] = true; // Memorizza la posizione come già vista
                }
            }
        }

        // Salva l'hash della fine di questo genoma per la sovrapposizione con il prossimo
        if (genome.size() >= l) {
            last_genome_suffix_hash = 0;
            pow_5 = 1ull;
            overlap_length = min(l, (int)genome.size());

            // Calcoliamo l'hash per l'ultima parte lunga "overlap_length"
            for (int i = 0; i < overlap_length; i++) {
                if (i > 0) pow_5 = (pow_5 * 5ull) % mod;
                last_genome_suffix_hash = (last_genome_suffix_hash + pow_5 * char_to_hash_value(genome[genome.size() - 1 - i])) % mod;
            }
        }
    }

    auto end_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    cout << endl << "Pattern trovato in " << end_time - start_time << " us" << endl;
}


unsigned long long graph::to_uns_long(char c) {         //base to int conversion for hash function
    unsigned long long ll = 4ull;
    switch (c) {
        case 'A':
            ll = 0ull;
            break;

        case 'T':
            ll = 1ull;
            break;
            
        case 'C':
            ll = 2ull;
            break;

        case 'G':
            ll = 3ull;
            break;
        
        case 'a':
            ll = 0ull;
            break;

        case 't':
            ll = 1ull;
            break;
            
        case 'c':
            ll = 2ull;
            break;

        case 'g':
            ll = 3ull;
            break;
    }
    return ll;
}

bool graph::overlapping(string first, string second, const int& overlap) {    //direct check of overlap
    for (int i = 0; i < overlap; i++) {
        if ( first[first.length() - overlap + i] != second[i] )
            return false;
    }
    return true;
}

string graph::invert(const string& seg) {       //base complement and segment inversion
    string new_seg = seg;
    int len = seg.length();

    for (int i = 0; i < len; i++) {
        char base = 'N';

        switch (seg[i]) {
            case 'A':
                base = 'T';
                break;

            case 'T':
                base = 'A';
                break;
            
            case 'C':
                base = 'G';
                break;

            case 'G':
                base = 'C';
                break;
        }

        new_seg[len - i - 1] = base;
    }

    return new_seg;
}
