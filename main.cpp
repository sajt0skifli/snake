#include "graphics.hpp"
#include <vector>

using namespace genv;
using namespace std;

const int w=600,h=600,
          blocksize=20, //grid négyzet mérete (pixelben)
          falvastag=2;  //pályaszéli fal vastagsága (blockban)

struct block
{
    int x,y,status=0;
};

void gridbuild(vector<vector<block>> &grid)
{
    for(int i=0;i<grid.size();i++)
    {
        for(int j=0;j<grid[i].size();j++)
        {
            if(j==0)
            {
                grid[i][j].x=0;
            }
            else
            {
                grid[i][j].x=grid[i][j-1].x+blocksize;
            }
            if(i==0)
            {
                grid[i][j].y=0;
            }
            else
            {
                grid[i][j].y=grid[i-1][j].y+blocksize;
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
            if(i<falvastag || j<falvastag || i>grid.size()-falvastag-1 || j>grid[i].size()-falvastag-1)
            {
                grid[i][j].status=1;
                hatter << color(100,100,100);
            }
            else
            {
                hatter << color(0,200,0);
            }
            hatter << move_to(grid[i][j].x,grid[i][j].y) << box(blocksize,blocksize);
        }
    }
}

int main()
{
    vector<vector<block>> grid(h/blocksize,vector<block>(w/blocksize));
    canvas hatter;
    gout.open(w,h);
    hatter.open(w,h);
    gridbuild(grid);
    hatterbuild(hatter,grid);
    gout << stamp(hatter,0,0) << refresh;
    event ev;
    while(gin >> ev) {
        if(ev.type==ev_key && ev.keycode == key_escape)
        {
            return 0;
        }
    }
    return 0;
}
