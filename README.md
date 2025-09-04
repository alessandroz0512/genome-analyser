# Genome Sequence Graph Analyzer

This project is a **C++ tool for genomic sequence analysis** using graph-based data structures. It is designed to read genome assembly files, construct a graph of segments and overlaps, decyclify the graph to remove loops, and extract all possible genome sequences. Additionally, it supports efficient pattern searching within the reconstructed genomes using rolling hash techniques.

---

## Short Description

This genome analyzer parses genome assembly graphs, identifies valid genome sequences, and searches for patterns within them. The tool leverages graph traversal and hashing for fast, large-scale genome analysis, providing insights into sequence connectivity and motif distribution.

---

## Methodology

1. **Graph Construction**
   - Reads an assembly file twice: once to extract genome segments (nodes) and once to establish links (edges).
   - Supports inversion of segments and validates overlaps to ensure correct adjacency.

2. **Graph Processing**
   - Implements a **decylification process** to remove cycles, producing a directed acyclic graph suitable for genome reconstruction.
   - Detects sources, destinations, and isolated nodes for genome extraction.

3. **Genome Extraction**
   - Uses iterative depth-first search (DFS) to enumerate all possible genome paths up to a specified limit.
   - Builds genome sequences efficiently by appending only the non-overlapping portions of segments along each path.

4. **Pattern Searching**
   - Employs a **rolling hash** technique for rapid identification of subsequence patterns across multiple genomes.
   - Handles overlaps between consecutive genomes to ensure motifs spanning boundaries are detected.
   - Reports positions of all pattern occurrences.

5. **Utilities**
   - Supports inversion and complement of DNA segments.
   - Provides functions to check segment overlaps and compute nucleotide-based hashes.

---

## Analysis and Insights

- The tool allows **structural exploration** of genome assemblies, identifying how segments connect and overlap.
- Pattern searching is optimized for large genome datasets, leveraging modular arithmetic and rolling hash methods for speed.
- By decyclifying the graph, the method ensures **biologically meaningful sequences** are extracted without artifacts from cyclic paths.

---

## Results (Speculative)

- The analyzer can **reconstruct multiple genome variants** from a single assembly graph, revealing potential alternative splicing or assembly ambiguities.
- Preliminary tests suggest **high sensitivity in motif detection**, including motifs spanning multiple segments.
- The framework hints at **novel genome connectivity patterns**, which may be leveraged for evolutionary or comparative genomics studies.
- Future extensions could integrate statistical analysis of motif frequency or connectivity to predict functional genomic regions.

---

## Usage

```bash
# Compile
g++ main.cpp node.cpp graph.cpp -o genome_analyzer -std=c++17

# Run
./genome_analyzer <assembly_file.gfa>
