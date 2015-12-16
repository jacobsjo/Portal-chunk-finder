#include <iostream>
#include <fstream>
#include <cmath>
#include <string.h>
#include <ctime>
#include <thread>
#define M_PI 3.14159265358978 //Hello !
#define ll long long
#define ull unsigned long long
#define next(N) N*=25214903917;N+=11;N%=281474976710656

using namespace std;

int getEyesFromChunkseed(ull chunkseed);
void calculateLUTs();
void printUseage();
void getBestNumberOfThreadsAndSpeed();
void checkSeeds(ull baseSeed,ull nbSeedsToCheck,int stepSize=1, int minCount = 11);
void checkSeedsWithThreads(ull baseSeed,ull nbSeedsToCheck,int nbThreads=4, int minCount = 11);

int sinLUT[1024];
int cosLUT[1024];

int main(int argc, char* argv[])
{
    if (argc <= 1){
        printUseage();
        return 1;
    }
    calculateLUTs();
    if (strcmp(argv[1], "-t")== 0 || strcmp( argv[1], "--getBestThreadCount") == 0){
        getBestNumberOfThreadsAndSpeed();
        return 0;
    }
    if (argc < 5){
        printUseage();
        return 1;
    }

    char* e;

    ull seed = strtoll(argv[1], &e, 10);
    if (strcmp(e,"")){
        printUseage();
        return 1;
    }

    ull seedCount = strtoll(argv[2], &e, 10);
    if (strcmp(e,"")){
        printUseage();
        return 1;
    }

    int threadCount = strtol(argv[3], &e, 10);
    if (strcmp(e,"")){
        printUseage();
        return 1;
    }

    int minCount = strtol(argv[4], &e, 10);
    if (strcmp(e,"")){
        printUseage();
        return 1;
    }


    if (seed + seedCount > 281474976710656){
        cout << "[WARNING] Protal-room Generation is the same for every 2^48 Seeds. You may only check seeds < 2^48" << endl;
    }

    if (threadCount > 16){
        cerr << "[ERROR] You may only use up to 32 Threads" << endl;
        return 1;
    }

    checkSeedsWithThreads(seed, seedCount, threadCount, minCount);

    return 0;
}

void printUseage(){
        cerr << "Usage: seedFinder -t or seedFinder --getBestThreadCount" << endl
             << "               Tests the Program with different amount of Threads." << endl
             << "       seedFinder <startSeed> <seedCount> <threadCount> <min Count>" << endl
             << "               Starts the SeedFinder." << endl
             << "               If seedCount is 0 the Program will run infinitly until you interrupt it (Ctrl+C)" << endl;
}

void calculateLUTs(){
    for (int i = 0 ; i< 1024 ; i++){
        sinLUT[i] = round(sin((i* M_PI) / 512.0)*2048);
        cosLUT[i] = round(cos((i* M_PI) / 512.0)*2048);
    }
}

void getBestNumberOfThreadsAndSpeed()
{
    int minTime(10000000000),nbThreads(1);
    int elapsed;
    timespec startTime,endTime;
    bool improved(true);
    while(improved && nbThreads <= 32)
    {
        clock_gettime(CLOCK_MONOTONIC,&startTime);
        checkSeedsWithThreads(0,10000000,nbThreads,13); // Do not find anything (there also is none with 11 in the first 10 million seeds but just to be sure)
        clock_gettime(CLOCK_MONOTONIC,&endTime);
        elapsed = (endTime.tv_sec - startTime.tv_sec) * 1000;
        elapsed += (endTime.tv_nsec - startTime.tv_nsec) / 1000000;
        if(elapsed<minTime)
        {
            cout<<nbThreads<<" thread(s) : "<<elapsed<<" milliseconds taken to check 10 million seeds, speed : "<<10000000000/elapsed<<" seeds/second.\n";
            nbThreads*=2;
            minTime = elapsed;
        }
        else
        {
            improved=false;
        }
    }
}

void checkSeedsWithThreads(ull baseSeed,ull nbSeedsToCheck,int nbThreads, int minCount)
{
    int iThread;
    thread myThreadArray[nbThreads];
    for(iThread=0; iThread<nbThreads; iThread++)
    {
        myThreadArray[iThread]=thread(checkSeeds,baseSeed+iThread,nbSeedsToCheck,nbThreads, minCount);
    }
    for(iThread=0; iThread<nbThreads; iThread++)
    {
        myThreadArray[iThread].join();
    }
}

void checkSeeds(ull baseSeed,ull nbSeedsToCheck,int stepSize, int minCount)
{
    ull seed,RNGseed,chunkseed;
    ll var8,var10;
    int baseX,baseZ,chunkX,chunkZ,nbEyes,t(time(0)),angle;
    double dist;
    for(seed=baseSeed; (nbSeedsToCheck == 0) || seed<baseSeed+nbSeedsToCheck; seed+=stepSize)
    {
        RNGseed=seed^25214903917;
        next(RNGseed);
        var8=(RNGseed>>16)<<32;
        angle=RNGseed/274877906944;
        next(RNGseed);
        var8+=(int)(RNGseed>>16);//Don't ask me why there is a conversion to int here, I don't know either.
        var8=var8/2*2+1;
        next(RNGseed);
        var10=(RNGseed>>16)<<32;
        dist=160+(RNGseed/2199023255552);
        next(RNGseed);
        var10+=(int)(RNGseed>>16);
        var10=var10/2*2+1;
        baseX=(cosLUT[angle] * dist) / 8192;
        baseZ=(sinLUT[angle] * dist) / 8192;

        for(chunkX=baseX-6; chunkX<=baseX+6; chunkX++)
        {
            for(chunkZ=baseZ-6; chunkZ<=baseZ+6; chunkZ++)
            {
                chunkseed=(var8*chunkX+var10*chunkZ)^seed;
                nbEyes=getEyesFromChunkseed(chunkseed);
                if(nbEyes>=minCount)
                {
                    ofstream flow("log.txt",ios::app);
                    cout<<seed<<" "<<nbEyes<<" "<<chunkX<<" "<<chunkZ<<endl;
                    flow<<seed<<" "<<nbEyes<<" "<<chunkX<<" "<<chunkZ<<endl;
                }
            }
        }
    }
}

int getEyesFromChunkseed(ull chunkseed)//This function is full of Azelef math magic
{
    int iEye,nbEyes(0);
    chunkseed=chunkseed^25214903917;//This is the equivalent of starting a new Java RNG
    chunkseed*=124279299069389;//This line and the one after it simulate 761 calls to next() (761 was determined by CrafterDark)
    chunkseed+=17284510777187;
    chunkseed%=281474976710656;
    if(chunkseed>253327479039590)
    {
        next(chunkseed);
        if(chunkseed>253327479039590)
        {
            nbEyes=2;
            for(iEye=2; iEye<12; iEye++) //This is the same as calling nextFloat() 10 times and comparing it to 0.9
            {
                next(chunkseed);
                if(chunkseed>253327479039590)
                {
                    nbEyes++;
                }
            }
        }
    }
    return nbEyes;
}
