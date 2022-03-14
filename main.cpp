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
    char status='s';    //s=szabad, f=fal, t=test, (p=volt test), a=alma
};

bool operator==(const pont &a, const pont &b)
{
    return (a.x==b.x and a.y==b.y);
}

struct Racs
{
    private:
        vector<vector<block>> grid;

    public:
    Racs()
    {
        grid=vector<vector<block>>(h/blocksize,vector<block>(w/blocksize));
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

    void hatter(canvas &hatter)
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

    int xblockszam()
    {
        return grid[0].size();
    }

    int yblockszam()
    {
        return grid.size();
    }

    void testclear()
    {
        for(int i=0;i<grid.size();i++)
        {
            for(int j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].status=='t')
                {
                    grid[i][j].status='p';
                }
                else if(grid[i][j].status=='p')
                {
                    grid[i][j].status='s';
                }
            }
        }
    }

    void testadd(pont testpoz)
    {
        for(int i=0;i<grid.size();i++)
        {
            for(int j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].koord==testpoz)
                {
                    grid[i][j].status='t';
                }
            }
        }
    }

    bool utkoz(pont fejpoz, char irany)
    {
        for(int i=0;i<grid.size();i++)
        {
            for(int j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].koord==fejpoz)
                {
                    if((irany=='j' && (grid[i][j+1].status=='f' || grid[i][j+1].status=='t')) ||
                       (irany=='f' && (grid[i-1][j].status=='f' || grid[i-1][j].status=='t')) ||
                       (irany=='l' && (grid[i+1][j].status=='f' || grid[i+1][j].status=='t')) ||
                       (irany=='b' && (grid[i][j-1].status=='f' || grid[i][j-1].status=='t')))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }
    }

    void ghostmode(canvas c)
    {
        for(int i=0;i<grid.size();i++)
        {
            for(int j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].status=='p')
                {
                    gout << stamp(c,grid[i][j].koord.x,grid[i][j].koord.y);
                }
            }
        }
    }
};

struct Fej
{
    private:
        pont koord;
        char irany='j'; //j=jobb, b=bal, f=fel, l=le
        canvas textura;

    public:
    Fej(int xblock, int yblock, canvas c)
    {
        koord.x=(xblock/2+1)*20;
        koord.y=(yblock*20)-(falvastag*20+(beltav/2)*20);
        textura=c;
    }

    void rajzol()
    {
        gout << stamp(textura,koord.x,koord.y);
    }

    pont pozicio()
    {
        return koord;
    }

    char iranyker()
    {
        return irany;
    }

    void mozog(char c)
    {
        irany=c;
        if(irany=='j')
        {
            koord.x+=blocksize;
        }
        else if(irany=='f')
        {
            koord.y-=blocksize;
        }
        else if(irany=='l')
        {
            koord.y+=blocksize;
        }
        else
        {
            koord.x-=blocksize;
        }
    }

    void texreplace(canvas c)
    {
        textura=c;
    }
};

struct Test
{
    private:
        pont koord;
        Test* follow;
        canvas textura;

    public:
    Test(int x_, int y_, Test* p, canvas c)
    {
        koord.x=x_;
        koord.y=y_;
        follow=p;
        textura=c;
    }

    void rajzol()
    {
        gout << stamp(textura,koord.x,koord.y);
    }

    pont pozicio()
    {
        return koord;
    }

    void mozog(pont fejpoz)
    {
        if(follow==0)
        {
            koord.x=fejpoz.x;
            koord.y=fejpoz.y;
        }
        else
        {
            koord.x=follow->koord.x;
            koord.y=follow->koord.y;
        }
    }
};

void import(ifstream &be, string fajl, canvas &c)
{
    vector<vector<szin>> kep;
    int szel, mag;
    be.open(fajl);
    be >> szel >> mag;
    c.open(szel,mag);
    kep=vector<vector<szin>>(mag,vector<szin>(szel));
    for(int i=0;i<kep.size();i++)
    {
        for(int j=0;j<kep[i].size();j++)
        {
            be >> kep[i][j].r >> kep[i][j].g >> kep[i][j].b;
            c << move_to(j,i) << color(kep[i][j].r,kep[i][j].g,kep[i][j].b) << dot;
        }
    }
    be.close();
}

void rajzol(canvas hatter, Fej fej, vector<Test*> testek)
{
    gout << stamp(hatter,0,0);
    for(int i=0;i<testek.size();i++)
    {
        testek[i]->rajzol();
    }
    fej.rajzol();
}

int main()
{
    ifstream be;
    canvas hatter,fejle,fejfel,fejjobb,fejbal,fejdead,test;
    Racs racs;
    bool gamestart=false, gameover=false;
    char inputirany='j';
    gout.open(w,h);

    hatter.open(w,h);                       //
    racs.hatter(hatter);                    //   Rács és háttér létrehozása
    gout << stamp(hatter,0,0);              //

    import(be,"fej2le.kep",fejle);          //
    import(be,"fej2fel.kep",fejfel);        //
    import(be,"fej2jobb.kep",fejjobb);      //   Textúrák
    import(be,"fej2bal.kep",fejbal);        //  importálása
    import(be,"fej2dead.kep",fejdead);      //
    import(be,"test.kep",test);             //

    vector<Test*> testek;                                                                           //
    Fej fej(racs.xblockszam(),racs.yblockszam(),fejjobb);                                           //
    fej.rajzol();                                                                                   //
    testek.push_back(new Test(fej.pozicio().x-20,fej.pozicio().y,0,test));                          //  Fej és test
    testek[0]->rajzol();                                                                            //  alapállapota
    testek.push_back(new Test(testek[0]->pozicio().x-20,testek[0]->pozicio().y,testek[0],test));    //
    testek[1]-> rajzol();                                                                           //
    gout << refresh;                                                                                //

    gin.timer(500);
    event ev;
    while(gin >> ev) {
        if(ev.type==ev_key && ev.keycode==key_escape)
        {
            return 0;
        }
        else if(!gamestart && !gameover)
        {
            if(ev.type==ev_timer)
            {
                racs.testclear();
                for(int i=testek.size()-1;i>=0;i--)
                {
                    testek[i]->mozog(fej.pozicio());
                    racs.testadd(testek[i]->pozicio());
                }

                if(racs.utkoz(fej.pozicio(),inputirany))
                {
                    fej.texreplace(fejdead);
                    gameover=true;
                }
                else
                {
                    fej.mozog(inputirany);
                }

                rajzol(hatter,fej,testek);
                if(gameover)
                {
                    racs.ghostmode(test);
                }
                gout << refresh;
            }
            else if(ev.type==ev_key)
            {
                if(ev.keycode==key_right && fej.iranyker()!='b')
                {
                    inputirany='j';
                    fej.texreplace(fejjobb);
                }
                else if(ev.keycode==key_up && fej.iranyker()!='l')
                {
                    inputirany='f';
                    fej.texreplace(fejfel);
                }
                else if(ev.keycode==key_down && fej.iranyker()!='f')
                {
                    inputirany='l';
                    fej.texreplace(fejle);
                }
                if(ev.keycode==key_left && fej.iranyker()!='j')
                {
                    inputirany='b';
                    fej.texreplace(fejbal);
                }
            }
        }
    }
    return 0;
}
