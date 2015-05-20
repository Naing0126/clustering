#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<algorithm>
#include<vector>
#include<string>
#include<math.h>
#include<cstdlib>
#include<ctime>
#include<sstream>

using namespace std;

#define OUTLIER_THRESHOLD 0.5

FILE *in;
FILE *outlier;

vector<FILE*> outputFiles;

int K;
int O;
string output = "input";

int knn[10000][10000];
int map[1000][1000];

/*
objects list
*/

typedef struct _Object{
    int id;
    double x;
    double y;
    int cid; // cluster id
}Object;

vector<Object> objectList; // objects list

typedef struct _Cluster{
    double c_x; // centroid x
    double c_y; // centroid y
    vector<Object> objectList; // clustered objects list
}Cluster;

vector <Cluster> clusterList;

void printObjects(){
    int i;
    for (i = 0; i < objectList.size(); i++){
        printf("%d : (%.8lf,%.8lf)\n", objectList[i].id, objectList[i].x, objectList[i].y);
    }
    cout << "objects size :" << objectList.size() << endl;
}

double calculateDistance(double ax, double ay, double bx, double by){
    return sqrt((ax - bx)*(ax - bx) + (ay - by)*(ay - by));
}

void DetectingOutlier(){
    int i,j;

    printf("Detecting Outliers...\n");

    printf("Making knn Matrix...\n");

    double min1, min2, min3, min4;
    int midx1, midx2, midx3, midx4;

    for (i = 0; i < objectList.size(); i++){
        min1 = min2 = min3 = min4 = 1000000;
        midx1 = midx2 = midx3 = midx4 = 0;
        for (j = 0; j < objectList.size(); j++){
            knn[i][j] = 0;
            if (j != i){
                double temp = calculateDistance(objectList[i].x, objectList[i].y, objectList[j].x, objectList[j].y);
                if (min1 > temp){
                    min3 = min2;
                    midx3 = midx2;

                    min2 = min1;
                    midx2 = midx1;

                    min1 = temp;
                    midx1 = j;
                }
                else if (min2 > temp){
                    min3 = min2;
                    midx3 = midx2;

                    min2 = temp;
                    midx2 = j;
                }
                else if (min3 > temp){
                    min3 = temp;
                    midx3 = j;
                }
            }
        }
        knn[i][midx1] = 3;
        knn[i][midx2] = 2;
        knn[i][midx3] = 1;
    }

    printf("Calculating score...\n");

    int score[10000];

    outlier = fopen("outliers.txt", "w");
    int ocnt = 0;

    for (i = 0; i < objectList.size(); i++){
        int sum = 0; // score
        for (j = 0; j < objectList.size(); j++){
            if (knn[j][i] > 0){
                sum++;
            }
        }
        score[i] = sum;

        if ((double)1/((double)score[i]+1) >= OUTLIER_THRESHOLD){
            ocnt++;
            objectList.erase(objectList.begin() + i);
            fprintf(outlier, "%d\t%d\n", i, score[i]);
        }
    }
    /*
    printf("Calculating Centrality score...\n");

    int cscore[10000];


  
    for (i = 0; i < objectList.size(); i++){
        int sum = score[i]; // Centrality score
        for (j = 0; j < objectList.size(); j++){
            if (knn[j][i] > 0){
                sum += score[j];
            }
        }
        cscore[i] = sum;

        
        if ((double)cscore[i] < (double)O*0.003){
            ocnt++;
            objectList.erase(objectList.begin() + i);
            fprintf(outlier,"%d\t%d\n", i, cscore[i]);
        }
    }
    */

    printf("# of outliers : %d\n", ocnt);
    printf("score threshold is %lf\n", OUTLIER_THRESHOLD);

}

void initClusterCentroid(){
    int i,j;
    Cluster temp;

     for (i = 0; i < K; i++){
        Object temp_obj;
        // random
        int flag = 1; // 0 : in threshold, 1 : over threshold
        while (flag){
            srand((unsigned int)time(NULL));
            temp_obj = objectList[rand()%objectList.size()];

            temp.c_x = temp_obj.x;
            temp.c_y = temp_obj.y;

            flag = 0;
            for (j = 0; j < i; j++){
                if (calculateDistance(temp.c_x, temp.c_y, clusterList[j].c_x, clusterList[j].c_y)<20){
                    flag = 1;
                    break;
                }
            }
        }
        clusterList.push_back(temp);

        printf("cluster #%d's centroid : (%.8lf,%.8lf)\n", i, clusterList[i].c_x, clusterList[i].c_y);

    }
}

void initClusterList(){
    int i;
    for (i = 0; i < K; i++){
        clusterList[i].objectList.clear();
    }
}

void Clustering(){
    // find min distance's cluster
    int i, j;
    for (i = 0; i < objectList.size();i++){
        double min = 10000;
        for (j = 0; j < K; j++){
            double temp = calculateDistance(objectList[i].x, objectList[i].y, clusterList[j].c_x, clusterList[j].c_y);
            if (temp < min){
                min = temp;
                objectList[i].cid = j;
            }
        }
        if (min < 200)
            clusterList[objectList[i].cid].objectList.push_back(objectList[i]);
    }
}

// find new centroid
int FindNewCentroid(){
    int i,j;
    double tempx;
    double tempy;
    int flag = 0;

    printf("\n");

    for (i = 0; i < K; i++){
        double sumx = 0;
        double sumy = 0;
        for (j = 0; j < clusterList[i].objectList.size(); j++){
            sumx += clusterList[i].objectList[j].x;
            sumy += clusterList[i].objectList[j].y;
        }

        tempx = sumx / clusterList[i].objectList.size();
        tempy = sumy / clusterList[i].objectList.size();

        if (tempx != clusterList[i].c_x || tempy != clusterList[i].c_y)
            flag = 1;

        clusterList[i].c_x = tempx;
        clusterList[i].c_y = tempy;

        printf("cluster #%d's new centroid : (%.8lf,%.8lf)\n", i, clusterList[i].c_x, clusterList[i].c_y);
    }

    return flag ;
}

void writeResult(){
    int i,j;
    for (i = 0; i < K; i++){
        FILE* temp_file;
        stringstream ss;
        string str_cnum = "_cluster";

        ss << output << str_cnum << i << ".txt";

        temp_file = fopen(ss.str().c_str(), "w");

        for (j = 0; j < clusterList[i].objectList.size(); j++){
            fprintf(temp_file, "%d\n", clusterList[i].objectList[j].id);
        }
    }
}

void drawCluster(){
    int i, j;
    FILE* cluster;
    cluster = fopen("cluster.txt", "w");

    for (i = 0; i < 1000; i++){
        for (j = 0; j < 1000; j++){
            map[i][j] = 9;
        }
    }

    for (i = 0; i < objectList.size(); i++){
        int x = objectList[i].x;
        int y = objectList[i].y;
        map[x][y] = objectList[i].cid;
    }

    for (i = 0; i < 1000; i++){
        for (j = 0; j < 1000; j++){
            fprintf(cluster,"%d ", map[i][j]);
        }
    }
}

int main(int argc, char* argv[]){
    char str[100];
    char *token;
    int i, j;

    if (argc < 2){
        printf("check input!\n");
        return 0;
    }

    output.append(argv[1], 5, 1);

    in = fopen(argv[1], "r");
    K = atoi(argv[2]);

    // parsing input
    while (!feof(in)){
        if (fgets(str, sizeof(str), in)){
            Object tobject;
            
            string::size_type sz;     // alias of size_t
            string temp;

            token = strtok(str, "\t");
            temp = token;
            tobject.id = stoi(temp, &sz);

            token = strtok(NULL, "\t");
            temp = token;
            tobject.x = stod(temp, &sz);

            token = strtok(NULL, "\n");
            temp = token;
            tobject.y = stod(temp, &sz);

            objectList.push_back(tobject);

        }
    }

    O = objectList.size();

//    printObjects();

//    DetectingOutlier();

    initClusterCentroid();

    int flag = 1; // 1 : any centroid is change. 0 : all centroid is not changed

    while (flag){
        initClusterList();
        Clustering(); // clustering all objects
        flag = FindNewCentroid(); // find new centroid by new clusters
    }
    
    // write to output
    writeResult();
  
    return 0;
}