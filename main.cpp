#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <thread>
#define M_PI 3.14159265358978 //Hello !
#define ll long long
#define ull unsigned long long
#define next(N) N*=25214903917;N+=11;N%=281474976710656

using namespace std;

int getEyesFromChunkseed(ull chunkseed);
void calculateLUTs();
void getBestNumberOfThreadsAndSpeed();
void checkSeeds(ull baseSeed,ull nbSeedsToCheck,int stepSize=1);
void checkSeedsWithThreads(ull baseSeed,ull nbSeedsToCheck,int nbThreads=4);

int sinLUT[1024];
int cosLUT[1024];

int main()
{
    calculateLUTs();

    //getBestNumberOfThreadsAndSpeed();
    checkSeedsWithThreads(1,5,1);

    return 0;
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
    while(improved)
    {
        clock_gettime(CLOCK_MONOTONIC,&startTime);
        checkSeedsWithThreads(0,10000000,nbThreads);
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

void checkSeedsWithThreads(ull baseSeed,ull nbSeedsToCheck,int nbThreads)
{
    int iThread;
    thread myThreadArray[nbThreads];
    for(iThread=0; iThread<nbThreads; iThread++)
    {
        myThreadArray[iThread]=thread(checkSeeds,baseSeed+iThread,nbSeedsToCheck,nbThreads);
    }
    for(iThread=0; iThread<nbThreads; iThread++)
    {
        myThreadArray[iThread].join();
    }
}

void checkSeeds(ull baseSeed,ull nbSeedsToCheck,int stepSize)
{
    ull seed,RNGseed,chunkseed;
    ll var8,var10;
    int baseX,baseZ,chunkX,chunkZ,nbEyes,t(time(0)),angle;
    double dist;
    for(seed=baseSeed; seed<baseSeed+nbSeedsToCheck; seed+=stepSize)
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

//        cout<<seed << ": " << baseX << ", " <<baseZ << endl;
        for(chunkX=min(baseX-6,baseX+6); chunkX<=max(baseX-6,baseX+6); chunkX++)
        {
            for(chunkZ=min(baseZ-6,baseZ+6); chunkZ<=max(baseZ-6,baseZ+6); chunkZ++)
            {
                chunkseed=(var8*chunkX+var10*chunkZ)^seed;
                nbEyes=getEyesFromChunkseed(chunkseed);
                if(nbEyes>=11)
                {
           //         ofstream flow("log.txt",ios::app);
                    cout<<seed<<" "<<nbEyes<<" "<<chunkX<<" "<<chunkZ<<endl;
           //         flow<<seed<<" "<<nbEyes<<" "<<chunkX<<" "<<chunkZ<<endl;
                }
            }
        }
    //    if(seed%50000000==49999999)
    //    {
    //        cout<<seed+1<<" time : "<<time(0)-t<<endl;
    //    }
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
