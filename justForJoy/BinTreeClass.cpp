#include<iostream>
using namespace std;

struct Node{
    char data = NULL;
    Node* lc;
    Node* rc;
    Node(char c) : data(c), lc(nullptr), rc(nullptr) {}
};

class BinTree{
private:
    Node* root = nullptr;

    string pre = nullptr;
    string in = nullptr;
    string post = nullptr;

    int size = 0;
    int layers = 0;

public:
    BinTree(string pre, string in, string post) : pre(pre), in(in), post(post){ 
        int i = 0;
        while(pre[i] != '\0'){size++; i++;};
    }

    int getSize(){
        return size;
    }

    int getLayer(){
        return layers;
    }

    void preInCreat(){
        // size is already the (endIndex + 1)
        root = preInCreat(0, size, size, pre, in);
    }
    Node* preInCreat(int preRootI, int inRootI, int leafSize, string pre, string in){

        // end condition
        if(leafSize <= 0) return nullptr;

        // return point whatever is lc or rc
        if(leafSize == 1) {
            Node *t = new Node(pre[preRootI]);
            return t;
        }

        Node* r = new Node(pre[preRootI]);
        
        // update the scale
        int i;
        for(i = 0 ; i < leafSize; i++){
            if(pre[preRootI] == in[inRootI - 1 - i]) break ;
        }
        r->lc = preInCreat(
            preRootI + 1, 
            inRootI - 1 - i, 
            leafSize - (i + 1), 

            pre, 
            in
        );

        r->rc = preInCreat(
            preRootI + (leafSize - (i + 1)) + 1, 
            inRootI, // original right boundary
            i, 
            
            pre, 
            in
        );

        return r;

    }

    void printBinTree(){
        cout<<"preOrderTraversePrint: "<<endl;
        preOrderTraversePrint(root);
        cout<<"\n\ninorderTraversePrint: "<<endl;
        inOrderTraversePrint(root);
        cout<<"\n\npostOrderTraversePrint: "<<endl;
        postOrderTraversePrint(root);
    }

    void preOrderTraversePrint(Node* t){
        if(!t) return ;       
        cout<<t->data<<" "; 
        preOrderTraversePrint(t->lc);
        preOrderTraversePrint(t->rc);
    }
    void inOrderTraversePrint(Node* t){
        if(!t) return ;
        inOrderTraversePrint(t->lc);
        cout<<t->data<<" ";
        inOrderTraversePrint(t->rc);
    }
    void postOrderTraversePrint(Node* t){
        if(!t) return ;
        postOrderTraversePrint(t->lc);
        postOrderTraversePrint(t->rc);
        cout<<t->data<<" ";
    }

    ~BinTree(){
        postOrderTraverseKill(root);
    }
    void postOrderTraverseKill(Node* t){
        if(!t) return ;       
        postOrderTraverseKill(t->lc);
        postOrderTraverseKill(t->rc);
        delete t; 
    }
    
};

// global variable
const string
globalPre = "acpobeflnmgjkdhi", 
globalIn = "pcoamnlfegjkbdhi", 
globalPost = "pocmnlfkjgeihdba";

BinTree binTree(globalPre, globalIn, globalPost);
/*

    a - b - d - h - i
    |   |
    |   - - e - g - j - k
    |       |
    |       - - f - l - n
    - - c - o           |
        |               - - m
        - - p   
    
    preorder:  a, c, p, o, b, e, f, l, n, m, g, j, k, d, h, i
    inorder:   p, c, o, a, m, n, l, f, e, g, j, k ,b, d, h, i
    postorder: p, o, c, m, n, l, f, k, j, g, e, i, h, d, b, a

*/


int main(){
    binTree.preInCreat();
    binTree.printBinTree();
    return 0;
}