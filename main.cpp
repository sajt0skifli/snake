#include "graphics.hpp"
#include <vector>
#include <fstream>

using namespace genv;
using namespace std;

const int w=500,h=500,
          blocksize=20, //grid négyzet mérete (pixelben)
          falvastag=2,  //pályaszéli fal vastagsága (blockban)
          beltav=4;     //belsõ fal oldalfaltól való távolsága (blockban)

struct pont
{
    int x,y;
};

struct szin
{
    int r,g,b;
};

struct block
{
    pont koord;
    char status='s';    //s=szabad, f=fal, t=test, a=alma
};

struct fej
{
    private:
        pont koord;
        char irany='j'; //j=jobb, b=bal, l=le, f=fel
};

struct kigyo
{
    private:
        pont koord;
        kigyo* follow;
};

bool operator==(const pont &a, const pont &b)
{
    return (a.x==b.x and a.y==b.y);
}

void gridbuild(vector<vector<block>> &grid)
{
    for(int i=0;i<grid.size();i++)
    {
        for(int j=0;j<grid[i].size();j++)
        {
            if(j==0)
            {
                grid[i][j].koord.x=0;
            }
            else
            {
                grid[i][j].koord.x=grid[i][j-1].koord.x+blocksize;
            }
            if(i==0)
            {
                grid[i][j].koord.y=0;
            }
            else
            {
                grid[i][j].koord.y=grid[i-1][j].koord.y+blocksize;
            }
        }
    }

    for(int i=0;i<grid.size();i++)
    {
        for(int j=0;j<grid[i].size();j++)
        {
            if(i<falvastag || j<falvastag || i>grid.size()-falvastag-1 || j>grid[i].size()-falvastag-1 ||  //oldalfalak
               (i==falvastag+beltav && j>=falvastag+beltav && j<=grid[i].size()-(falvastag+beltav)-1) || (j==grid[i].size()/2 && i>=falvastag+beltav && i<=grid.size()-(falvastag+beltav)-1))    //belsõ fal
            {
                grid[i][j].status='f';
            }
        }
    }
}

void hatterbuild(canvas &hatter, vector<vector<block>> grid)
{
    for(int i=0;i<grid.size();i++)
    {
        for(int j=0;j<grid[i].size();j++)
        {
            if(grid[i][j].status=='f')
            {
                hatter << color(100,100,100);
            }
            else
            {
                hatter << color(0,200,0);
            }
            hatter << move_to(grid[i][j].koord.x,grid[i][j].koord.y) << box(blocksize,blocksize);
        }
    }
}

void pattern(canvas &c ,vector<vector<szin>> kep)
{
    for(int i=0;i<kep.size();i++)
    {
        for(int j=0;j<kep[i].size();j++)
        {
            c << move_to(j,i) << color(kep[i][j].r,kep[i][j].g,kep[i][j].b) << dot;
        }
    }
}

void import(ifstream &be, vector<vector<szin>> &kep)
{
    int szel, mag;
    be >> szel >> mag;
    kep=vector<vector<szin>>(mag,vector<szin>(szel));
    for(int i=0;i<kep.size();i++)
    {
        for(int j=0;j<kep[i].size();j++)
        {
            be >> kep[i][j].r >> kep[i][j].g >> kep[i][j].b;
        }
    }
}

int main()
{
    vector<vector<block>> grid(h/blocksize,vector<block>(w/blocksize));
    vector<vector<szin>> kep;
    ifstream be;

    canvas hatter,fej,test;                 //
    gout.open(w,h);                         //
    hatter.open(w,h);                       //  Grid és háttér létrehozása
    gridbuild(grid);                        //
    hatterbuild(hatter,grid);               //

    be.open("fej.kep");                     //
    import(be,kep);                         //
    fej.open(kep[0].size(),kep.size());     //  Fej textúra importálása
    pattern(fej,kep);                       //
    be.close();                             //

    be.open("test.kep");                    //
    import(be,kep);                         //
    test.open(kep[0].size(),kep.size());    //  Test textúra importálása
    pattern(test,kep);                      //
    be.close();                             //

    gout << stamp(hatter,0,0) << refresh;
    gout << stamp(fej,0,0) << refresh;
    gout << stamp(test,20,0) << refresh;

    event ev;
    while(gin >> ev) {
        if(ev.type==ev_key && ev.keycode == key_escape)
        {
            return 0;
        }
    }
    return 0;
}
