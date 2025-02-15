#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pixel { //structura pentru canalele RGB
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Pixel;

typedef struct quadtreeNode { //structura pentru nod
    Pixel color;
    struct quadtreeNode* child[3];
} Node;

Node* createnode(Pixel color) { //creez nod in arbore
    Node* new_node = (Node*)malloc(sizeof(Node)); //aloc dinamic
    new_node->color = color; //atribui culorile nodului
    int i;
    for (i = 0; i < 4; i++) {
        new_node->child[i] = NULL; //creez copiii nodului
    }
    return new_node;
}

Pixel color (Pixel **grid, int x, int y, unsigned int size) { 
    unsigned long long Red = 0;
    unsigned long long Green = 0;
    unsigned long long Blue = 0;
    int i, j;
    for (i = x; i < x + size; i++) { //parcurg fiecare pixel pentru a calcula
    //totalul de rosu, verde si albastru
        for (j = y; j < y + size; j++) {
            Red = Red + grid[i][j].r; 
            Green = Green + grid[i][j].g;
            Blue = Blue + grid[i][j].b;
        }
    }
    Pixel color;
    color.r = Red / (size * size); //calculez media aritmetica a culorilor
    color.b = Blue / (size * size);
    color.g = Green / (size * size);

    return color;
}

unsigned long long meancalc (Pixel **grid, int x, int y, unsigned int size, 
Pixel color) {
    unsigned long long mean = 0;
    unsigned long long redtotal, greentotal, bluetotal;
    int i, j;
    for (i = x; i < x + size; i++) { //parcurg fiecare pixel din imagine
        for (j = y; j < y + size; j++) {
            //aplic formula
            redtotal = (color.r - grid[i][j].r) * (color.r - grid[i][j].r);
            greentotal = (color.g - grid[i][j].g) * (color.g - grid[i][j].g);
            bluetotal = (color.b - grid[i][j].b) * (color.b - grid[i][j].b);

            mean = mean + redtotal + greentotal + bluetotal;
        }
    }
    mean = mean/(3 * size * size); //calculez culoarea medie a imaginii
    return mean;
}

Node* Creare_quadtree(Pixel **grid, int x, int y, unsigned int size, 
int factor) {
    if(size == 1) { //imaginea nu mai poate fi divizata
        return createnode(grid[x][y]); //creez nodul frunza
    }

    Pixel colors;
    colors = color(grid, x, y, size); //calculez canalul de culoare pentru nod
    int mean = meancalc(grid, x, y, size, colors); //calculez media de culoare
    if (mean <= factor) { //daca media este mai mica sau egala decat factorul
    //imaginea nu mai trebuie divizata
        return createnode(colors); //creez nodul frunza
    }
    //daca nodul trebuie divizat
    Node *parent;
    parent = createnode(colors); //creez nodul intern
    size = size / 2; //modific dimensiunea pentru nodurile copii
    //apelez functia recursiv pentru cei 4 descendenti ai nodului parinte si 
    //modific coordonatele astfel incat ordinea sa fie stanga sus, dreapta sus, 
    //dreapta jos si stanga jos
    parent->child[0] = Creare_quadtree(grid, x, y, size, factor); 
    parent->child[1] = Creare_quadtree(grid, x, y + size, size, factor);
    parent->child[2] = Creare_quadtree(grid, x + size, y + size, size, factor);
    parent->child[3] = Creare_quadtree(grid, x + size, y, size, factor);

    return parent; //returnez radacina arborelui
}

int level (Node *parinte) {
    if (parinte == NULL) { //am ajuns la un nod frunza
        return 0;
    }
    int level0, level1, level2, level3;
    //calculez nivelul maxim al fiecarui copil recursiv
    level0 = level(parinte->child[0]); 
    level1 = level(parinte->child[1]);
    level2 = level(parinte->child[2]);
    level3 = level(parinte->child[3]);

    int level_max; //aflu maximul dintre aceste valori
    if (level0 > level1) {
        level_max = level0;
    }
    else {
        level_max = level1;
    }
    if (level_max < level2) {
        level_max = level2;
    }
    if (level_max < level3) {
        level_max = level3;
    }
    return level_max + 1; //adaug 1, adica radacina
}

int nr_blocuri (Node* parinte) {
    if (parinte->child[0] == NULL) { //am ajuns la un nod frunza 
        return 1; //blocul este format din singurul nod parinte
    }
    else {
        int nr = 0, i;
        for (i = 0; i < 4; i++) { 
            nr = nr + nr_blocuri(parinte->child[i]);
            //calculez recursiv nr de blocuri pentru fiecare copil
        }
        return nr; //returnez numarul de blocuri
    }
}

int bloc_mare (Node* parinte, int size, int max_size) {
    if (parinte->child[0] == NULL) { //nodul este frunza
        if (size > max_size) { //daca dimensiunea frunzei este mai mare decat 
        //dimensiunea maxima curenta actualizez maximul
            max_size = size;
        }
    }
    else { //nodul este intern
        //apelez recursiv functia pentru a afla cea mai mare dimensiune dintre 
        //copii
        //modific dimensiunea initiala
        max_size = bloc_mare(parinte->child[0], size / 2, max_size);
        max_size = bloc_mare(parinte->child[1], size / 2, max_size);
        max_size = bloc_mare(parinte->child[2], size / 2, max_size);
        max_size = bloc_mare(parinte->child[3], size / 2, max_size);
    }
    return max_size; //returnez blocul cel mai mare din arbore
}

void printquadtree (Node* root, FILE *fp) {
    Node *p[1000000];
    int front = 0, rear = 0;
    p[rear] = root; //adaug radacina in coada deoarece e nod intern
    rear = rear + 1;

    while (front < rear) { //cat timp mai sunt elemente in coada
        Node* node = p[front];
        front = front + 1;
        if (node->child[0] == NULL) { //daca nodul este frunza 
            unsigned char var = 1;
            fwrite(&var, sizeof(unsigned char), 1, fp); //tipul nodului
            //canalul de culoare al frunzei
            fwrite(&node->color.r, sizeof(unsigned char), 1, fp);
            fwrite(&node->color.g, sizeof(unsigned char), 1, fp);
            fwrite(&node->color.b, sizeof(unsigned char), 1, fp);
        }
        else { //daca nodul este intern
            unsigned char var = 0; //tipul nodului
            fwrite(&var, sizeof(unsigned char), 1, fp);
        }
        int i;
        for (i = 0; i < 4; i++) {
            if (node->child[i] != NULL) { //daca nodul este intern
                p[rear] = node->child[i]; //adaug copiii in coada
                rear = rear + 1;
            }
        }
    }
}

int main (int argc, char *argv[]) {
    char* optiune; //declar variabilele
    char* inputfile;
    char* outputfile;
    int factor;

    if (argc == 5) { //daca sunt cinci argumente
        optiune = argv[1]; 
        inputfile = argv[3];
        outputfile = argv[4];
        factor = atoi(argv[2]);
    }

    FILE *f;
    f = fopen(inputfile, "rb"); //deschid fisierul de tip ppm

    unsigned char tip_fisier[2];
    fscanf(f, "%s", tip_fisier); //citesc tipul de fisier
    unsigned int width;
    fscanf(f, "%d", &width); //citesc dimensiunile imaginii
    unsigned int height;
    fscanf(f, "%d", &height);
    unsigned int culoare;
    fscanf(f, "%d", &culoare); //citesc numarul de culori
    fseek(f, 1, SEEK_CUR); //mut cursorul la dreapta pentru a citi matricea

    int i; 
    Pixel **image; //declar matricea 
    //aloc dinamic matricea
    image = (Pixel**)malloc(sizeof(Pixel*) * height);
    for (i = 0; i < height; i++) {
        image[i] = (Pixel*)malloc(sizeof(Pixel) * width);
    }
    for (i = 0; i < height; i++) { //citesc matricea din fisier
        fread(image[i], sizeof(Pixel), width, f);
    }
    fclose(f); //inchid fisierul

    Node *root; //variabila de tip pointer la structura Node 
    root = Creare_quadtree(image, 0, 0, height, factor); //creez arborele

    if (strcmp(optiune, "-c1") == 0) { //prima cerinta
        int level_tree = level(root); //numarul de niveluri
        int nr = nr_blocuri(root); //numarul de blocuri
        int sizefinal = bloc_mare(root, height, 0); //dimensiunea celui mai 
        //mare bloc
        FILE *fp;
        fp = fopen(outputfile, "w"); //deschid fisierul
        fprintf(fp, "%d\n", level_tree); //scriu in fisier cele 3 date
        fprintf(fp, "%d\n", nr);
        fprintf(fp, "%d\n", sizefinal);
        fclose(fp); //inchid fisierul
    }
    
    if (strcmp(optiune, "-c2") == 0) { //a doua cerinta
        FILE *fp;
        fp = fopen(outputfile, "wb"); //deschid fisierul binar
        fwrite(&height, sizeof(unsigned int), 1, fp); //scriu in acesta 
        //dimensiunea imaginii
        printquadtree(root, fp); //apelez functia prin care scriu datele 
        //arborelui cuaternar
        fclose(fp); //inchid fisierul
    }
    return 0;
}