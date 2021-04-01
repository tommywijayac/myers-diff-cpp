#include <iostream>
#include <tuple>
#include <cmath>
#include <set>

// Circular array
class V {
public:
    V(int start, int end) : start_(start), end_(end), i_(new int[end - start + 1]) {}

    virtual ~V() {
        delete[](i_);
    }
    int& operator[](int index) {
        return i_[index - start_];
    }
private:
    int* i_;
    int start_;
    int end_;
};

// Difference Result
typedef std::multiset<std::pair<int, std::string>> Diff;

/*
This function is a concrete implementation of the algorithm for 'finding the middle snake' presented
similarly to the pseudocode on page 11 of 'An O(ND) Difference Algorithm and Its Variations' by EUGENE W.MYERS.
This algorithm is a centeral part of calculating either the smallest edit script for a pair of
sequences, or finding the longest common sub - sequence(these are known to be dual problems).

The worst - case (and expected case) space requirement of this function is O(N + M), where N is
the length of the first sequence, and M is the length of the second sequence.

The worst - case run time of this function is O(MN) and this occurs when both string have no common
sub - sequence.Since the expected case is for the sequences to have some similarities, the expected
run time is O((M + N)D) where D is the number of edits required to transform sequence A into sequence B.

The space requirement remains the same in all cases, but less space could be used with a modified version
of the algorithm that simply specified a user - defined MAX value less than M + N.In this case, the
algorithm would stop earlierand report a D value no greater than MAX, which would be interpreted as
'there is no edit sequence less than length D that produces the new_sequence from old_sequence'.

Note that(if I have understood the paper correctly), the k values used for the reverse direction
of this implementation have opposite sign compared with those suggested in the paper.I found this made
the algorithm easier to implement as it makes the forwardand reverse directions more symmetric.

@old_sequence  This represents a sequence of something that can be compared against 'new_sequence'
using the '==' operator.  It could be characters, or lines of text or something different.

@N  The length of 'old_sequence'

@new_sequence  The new sequence to compare 'old_sequence' against.

@M  The length of 'new_sequence'

There are 5 return values for this function:

The first is an integer representing the number of edits(delete or insert) that are necessary to
produce new_sequence from old_sequence.

The next two parts of the return value are the point(x, y) representing the starting coordinate of the
middle snake.

The next two return values are the point(u, v) representing the end coordinate of the middle snake.
It is possible that(x, y) == (u, v)
*/
std::tuple<int, int, int, int, int> FindMiddleSnake(const int old_sequence[], int N, const int new_sequence[], int M) {
    // The difference between the length of the sequences
    int Delta = N - M;

    // The sum of the length of the sequences
    int MAX = M + N;

    // The array that holds the 'best possible x values' in search from top left to bottom right
    static V Vf(-MAX, MAX);
    // The array that holds the 'best possible x values' in search from bottom right to top left
    static V Vb(-MAX, MAX);

    // The initial point at (0, -1)
    Vf[1] = 0;
    // The initial point at (N, M+1)
    Vb[1] = 0;

    int x, y;
    int x_i, y_i;

    // We only need to iterate to ceil('max edit length'/2) because we're searching in both directions
    for (int D = 0; D <= std::ceil(MAX / 2.0); D++) {
        for (int k = -D; k <= D; k += 2) {
            if (k == -D || k != D && Vf[k - 1] < Vf[k + 1]) {
                // Did not increase x, but we'll take the better (or only) x value from the k line above
                x = Vf[k + 1];
            }
            else {
                // We can increase x by building on the best path from the k line above
                x = Vf[k - 1] + 1;
            }
            // From fundamental axiom of this algorithm: x - y = k
            y = x - k;
            // Remember the initial point before the snake so we can report it
            x_i = x;
            y_i = y;
            // While these sequences are identical, keep moving through the graph with no cost
            while (x < N && y < M && old_sequence[x] == new_sequence[y]) {
                x += 1;
                y += 1;
            }
            // This is the new best x value
            Vf[k] = x;
            // Only check for connections from the forward search when N - M is odd
            // and when there is a reciprocal k line coming from the other direction.
            if ((_abs64(Delta % 2) == 1) && (-(k - Delta)) >= -(D - 1) && (-(k - Delta)) <= (D - 1)) {
                if (Vf[k] + Vb[-(k - Delta)] >= N) {
                    return std::make_tuple(2 * D - 1, x_i, y_i, x, y);
                }
            }
        }
        for (int k = -D; k <= D; k += 2) {
            if (k == -D || k != D && Vb[k - 1] < Vb[k + 1]) {
                x = Vb[k + 1];
            }
            else {
                x = Vb[k - 1] + 1;
            }
            y = x - k;
            x_i = x;
            y_i = y;
            while (x < N && y < M && old_sequence[N - x - 1] == new_sequence[M - y - 1]) {
                x += 1;
                y += 1;
            }
            Vb[k] = x;
            if (Delta % 2 == 0 && (-(k-Delta)) >= -D && (-(k-Delta)) <= D) {
                if (Vb[k] + Vf[(-(k-Delta))] >= N) {
                    return std::make_tuple(2 * D, N - x, M - y, N - x_i, M - y_i);
                }
            }
        }
    }
    return {};
}

/*
This function is a concrete implementation of the algorithm for finding the shortest edit script that was
'left as an exercise' on page 12 of 'An O(ND) Difference Algorithm and Its Variations' by EUGENE W.MYERS.

@old_sequence  This represents a sequence of something that can be compared against 'new_sequence'
using the '==' operator.  It could be characters, or lines of text or something different.

@N  The length of 'old_sequence'

@new_sequence  The new sequence to compare 'old_sequence' against.

@M  The length of 'new_sequence'

The return value is a sequence of objects that contains the indicies in old_sequnce and new_sequnce that
you could use to produce new_sequence from old_sequence using the minimum number of edits.

The format of this function as it is currently written is optimized for clarity, not efficiency.It is
expected that anyone wanting to use this function in a real application would modify the 2 lines noted
below to produce whatever representation of the edit sequence you wanted.
*/
Diff ShortestEditScript(const int old_sequence[], int N, const int new_sequence[], int M, int current_x, int current_y) {
    Diff rtn;
    
    if (N > 0 && M > 0) {
        int D, x, y, u, v;
        std::tie(D, x, y, u, v) = FindMiddleSnake(old_sequence, N, new_sequence, M);
        // If the graph represented by the current sequences can be further subdivided
        if (D > 1 || (x != u && y != v)) {
            // Collection delete/inserts before the snake
            Diff _rtn;
            _rtn.clear();
            _rtn = ShortestEditScript(old_sequence, x, new_sequence, y, current_x, current_y);
            rtn.insert(_rtn.begin(), _rtn.end());
            // Collection delete/inserts after the snake
            _rtn.clear();
            _rtn = ShortestEditScript(old_sequence + u, N - u, new_sequence + v, M - v, current_x + u, current_y + v);
            rtn.insert(_rtn.begin(), _rtn.end());
        }
        else if (M > N) {
            // M is longer than N, but we know there is a maximum of one edit to transform old_sequence into new_sequence
            // The first N elements of both sequences in this case will represent the snake, and the last element
            // will represent a single insertion
            Diff _rtn = ShortestEditScript(old_sequence + N, N - N, new_sequence + N, M - N, current_x + N, current_y + N);
            rtn.insert(_rtn.begin(), _rtn.end());
        }
        else if (M < N) {
            // N is longer than (or equal to) M, but we know there is a maximum of one edit to transform old_sequence to new_sequence
            // The first M elements of both sequences in this case will represent the snake, and the last element
            // will represent a single deletion. If M == N, then this reduces to a snake which does not contain any edits
            Diff _rtn = ShortestEditScript(old_sequence + M, N - M, new_sequence + M, M - M, current_x + M, current_y + M);
            rtn.insert(_rtn.begin(), _rtn.end());
        }
    }
    else if (N > 0) {
        // This area of the graph consist of only horizontal edges that represent deletions
        for (int i = 0; i < N; i++) {
            std::cout << "{del, pos_old: " << current_x + i << "val: " << old_sequence[i] << "}\n";
            rtn.insert(std::make_pair(current_x + i, "del"));
        }
    }
    else if (M > 0) {
        // This area of the graph consist of only vertical edges that represent insertions
        for (int i = 0; i < M; i++) {
            std::cout << "{add, pos_old: " << current_x << " pos_new: " << current_y+i << " val: " << new_sequence[i] << "}\n";
            rtn.insert(std::make_pair(current_y + i, "add"));
        }
    }

    return rtn;
}

int main() {
    int a[] = { 1,4,27,21,23,24,26,28,13 }; //old
    int b[] = { 1,4,20,21,22,23,24,25,26,13 }; //new


    Diff result = ShortestEditScript(a, sizeof(a) / sizeof(int), b, sizeof(b) / sizeof(int), 0, 0);
    for (Diff::iterator it = result.begin(); it != result.end(); it++)
    {
        std::cout << it->first << it->second << "\n";
    }
        
    Diff::iterator it = result.begin();
    int len_a = sizeof(a) / sizeof(int);
    int len_b = sizeof(b) / sizeof(int);
    int i = 0, j = 0;
    while (i < len_a && j < len_b)
    {
        if (it!=result.end() && it->second.compare("add") == 0 && it->first==j)
        {
            std::cout << "+ " << b[j];
            j++;

            it++;
        }
        else if (it!=result.end() && it->second.compare("del") == 0 && it->first==i)
        {
            std::cout << "- \t" << a[i];
            i++;

            it++;
        }
        else
        {
            std::cout << " " << a[i] << "\t" << b[j]; //or b[j], same
            i++;
            j++;
        }
        std::cout << "\n";
    }

    // means old_sequence is longer than new_sequence
    while (i < len_a)
    {
        // leftover deletion
        std::cout << "- \t" << a[i];
        i++;
    }

    while (j < len_b)
    {
        // leftover addition
        std::cout << "+ " << b[j];
        j++;
    }
}
