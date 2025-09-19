#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

template<typename T>
class CustomList {
private:
    T *data = nullptr;
    size_t capacity = 0;
    size_t size = 0;

public:
    CustomList() {}

    CustomList(size_t initialCapacity) {
        capacity = initialCapacity;
        size = 0;
        data = new T[capacity];
    }

    ~CustomList() {
        delete[] data;
    }


    CustomList(const CustomList &other) {
        capacity = other.capacity;
        size = other.size;

        data = new T[capacity];

        for (size_t i = 0; i < size; i++) {
            data[i] = other.data[i];
        }
    }


    CustomList &operator=(const CustomList &other) {
        if (this != &other) {
            delete[] data;
            capacity = other.capacity;
            size = other.size;
            data = new T[capacity];
            for (size_t i = 0; i < size; i++) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }


    void reserve(size_t newCapacity) {
        if (newCapacity > capacity) {
            T *newData = new T[newCapacity];
            for (size_t i = 0; i < size; i++) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }
    }

    void insert(const T &value) {
        if (size >= capacity) {
            capacity = capacity == 0 ? 16 : capacity * 2;
            T *newData = new T[capacity];
            for (size_t i = 0; i < size; i++) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
        }
        data[size++] = value;
    }

    void removeByIndex(size_t index) {
        if (index >= size) return;
        if (index != size - 1) {
            data[index] = data[size - 1];
        }
        size--;
    }


    void removeItem(const T &value) {
        for (size_t i = 0; i < size; i++) {
            if (data[i] == value) {
                removeByIndex(i);
                return;
            }
        }
    }

    void clear() {
        size = 0;
    }


    T &operator[](size_t index) {
        return data[index];
    }

    const T &operator[](size_t index) const {
        return data[index];
    }

    size_t getSize() const {
        return size;
    }

    bool empty() const {
        return size == 0;
    }

    void resize(size_t newSize) {
        if (newSize > capacity) {
            reserve(newSize);
        }
        size = newSize;
    }

    void resize(size_t newSize, const T &defaultValue) {
        size_t oldSize = size;
        resize(newSize);
        for (size_t i = oldSize; i < newSize; i++) {
            data[i] = defaultValue;
        }
    }
};


CustomList<CustomList<int>> dataset;
int dim;
int maxTime = 0;

struct node {
    int index = -1;
    node *parent = nullptr;
    CustomList<node *> children;

    node() {
        children.reserve(4);
    }
};

inline void swap(int &a, int &b) {
    int temp = a;
    a = b;
    b = temp;
}

inline int max(int a, int b) {
    return (a > b) ? a : b;
}


void heapify(CustomList<int> &arr, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest])
        largest = left;

    if (right < n && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {
        swap(arr[i], arr[largest]);
        heapify(arr, n, largest);
    }
}

void heapSort(CustomList<int> &arr) {
    int n = arr.getSize();
    if (n <= 1) return;

    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    for (int i = n - 1; i > 0; i--) {
        swap(arr[0], arr[i]);
        heapify(arr, i, 0);
    }
}


inline bool dominates(int a, int b, const CustomList<int> &efflist) {
    bool hasEfflist = !efflist.empty();
    bool result = false;

    for (int i = 0; i < dim; i++) {
        int eff = hasEfflist ? efflist[i] : 1;

        if (eff > 0) {
            if (dataset[a][i] > dataset[b][i]) return false;
            else if (dataset[a][i] < dataset[b][i]) result = true;
        } else if (eff < 0) {
            if (dataset[a][i] < dataset[b][i]) return false;
            else if (dataset[a][i] > dataset[b][i]) result = true;
        }
    }
    return result;
}

void inject(node *parent, node *newNode) {
    CustomList<node *> childrenL = parent->children;
    size_t numOfChild = childrenL.getSize();
    for (size_t i = 0; i < numOfChild; i++) {
        node *child = childrenL[i];
        if (dominates(child->index, newNode->index, CustomList<int>())) {
            inject(child, newNode);
            return;
        }
    }

    newNode->parent = parent;
    parent->children.insert(newNode);


    CustomList<node *> toMove;
    toMove.reserve(numOfChild);


    for (int i = parent->children.getSize() - 1; i >= 0; i--) {
        node *nodeToRem = parent->children[i];
        if (nodeToRem != newNode && dominates(newNode->index, nodeToRem->index, CustomList<int>())) {
            toMove.insert(nodeToRem);
            parent->children.removeByIndex(i);
        }
    }

    size_t toMoveCount = toMove.getSize();
    for (size_t i = 0; i < toMoveCount; i++) {
        node *sibling = toMove[i];
        sibling->parent = newNode;
        newNode->children.insert(sibling);
    }
}

void eject(node *nodeToRem) {
    node *parent = nodeToRem->parent;
    CustomList<node *> childrenL = nodeToRem->children;

    parent->children.removeItem(nodeToRem);

    CustomList<node *> childrenCopy = nodeToRem->children;
    size_t numOfChild = childrenCopy.getSize();
    for (size_t i = 0; i < numOfChild; i++) {
        node *child = childrenCopy[i];
        inject(parent, child);
    }

    delete nodeToRem;
}

CustomList<CustomList<int>> readMatrixFromFile(const string &filename) {
    CustomList<CustomList<int>> matrix;
    ifstream file(filename);

    matrix.reserve(800000);

    string line;
    line.reserve(256);

    while (getline(file, line)) {
        CustomList<int> row;
        stringstream ss(line);
        int num;

        row.reserve(7);

        while (ss >> num) {
            row.insert(num);
        }

        if (!row.empty()) {
            matrix.insert(row);
        }
    }

    file.close();
    return matrix;
}


CustomList<CustomList<CustomList<int>>> readTimesFromFile(const string &filename) {
    CustomList<CustomList<int>> starts;
    CustomList<CustomList<int>> ends;
    CustomList<CustomList<CustomList<int>>> result;
    ifstream file(filename);

    starts.resize(800000);
    ends.resize(800000);


    string line;
    line.reserve(64);

    int pointIndex = 0;
    while (getline(file, line)) {
        stringstream ss(line);
        int start, end;
        if (ss >> start >> end) {
            starts[start].insert(pointIndex);
            ends[end].insert(pointIndex);
            pointIndex++;

            if (end > maxTime) {
                maxTime = end;
            }
        }
    }

    file.close();
    result.insert(starts);
    result.insert(ends);
    return result;
}


int main() {
    auto start = std::chrono::high_resolution_clock::now();

    string inputSize = "large";

    CustomList<CustomList<int>> matrix = readMatrixFromFile("dataset/" + inputSize + "/" + inputSize + ".input");
    CustomList<CustomList<CustomList<int>>> times = readTimesFromFile(
            "dataset/" + inputSize + "/" + inputSize + ".times");

    CustomList<CustomList<int>> starts = times[0];
    CustomList<CustomList<int>> ends = times[1];


    dataset = matrix;
    dim = matrix[0].getSize();

    node *origin = new node();

    CustomList<node *> activeNodes(matrix.getSize());
    activeNodes.resize(matrix.getSize(), nullptr);


    ofstream outFile("output.txt");
    outFile.tie(nullptr);

    
    for (int t = 0; t <= maxTime; t++) {
        size_t startCount = starts[t].getSize();
        for (size_t i = 0; i < startCount; i++) {
            int pointIdx = starts[t][i];
            node *newNode = new node();
            newNode->index = pointIdx;
            inject(origin, newNode);
            activeNodes[pointIdx] = newNode;
        }

        size_t endCount = ends[t].getSize();
        for (size_t i = 0; i < endCount; i++) {
            int pointIdx = ends[t][i];
            if (activeNodes[pointIdx]) {
                eject(activeNodes[pointIdx]);
                activeNodes[pointIdx] = nullptr;
            }
        }

        CustomList<int> skyline;
        size_t childrenCount = origin->children.getSize();
        skyline.reserve(childrenCount);

        for (size_t i = 0; i < childrenCount; i++) {
            node *child = origin->children[i];
            skyline.insert(child->index);
        }

        heapSort(skyline);

        size_t skylineSize = skyline.getSize();
        for (size_t i = 0; i < skylineSize; i++) {
            outFile << skyline[i];
            outFile << " ";
        }
        outFile << "\n";
        
    }

    outFile.close();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;

    delete origin;

    return 0;
}