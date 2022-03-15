#include "graphics.hpp"
#include <vector>
#include <fstream>
#include <sstream>

using namespace genv;
using namespace std;

const int w=700,h=700,
          blocksize=20, //rács négyzet mérete (pixelben) - a program 20 köré épült, ennek megváltoztatásával nem mûködik
          falvastag=2,  //pályaszéli oldalfal vastagsága (blockban)
          beltav=4,     //belso fal oldalfaltól való távolsága (blockban)
          almatime=8;   //alma létrehozásának gyakorisága (lépésben) (pl.: megevéstõl számított 7. lépésben)

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
                   (i==falvastag+beltav && j>=falvastag+beltav && j<=grid[i].size()-(falvastag+beltav)-1) || (j==grid[i].size()/2 && i>=falvastag+beltav && i<=grid.size()-(falvastag+beltav)-1))    //belsÃµ fal
                {
                    grid[i][j].status='f';
                }
            }
        }
    }

    void hatter(canvas &hatter, canvas fu, canvas fal)
    {
        for(int i=0;i<grid.size();i++)
        {
            for(int j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].status=='f')
                {
                    hatter << stamp(fal,grid[i][j].koord.x,grid[i][j].koord.y);
                }
                else
                {
                    hatter << stamp(fu,grid[i][j].koord.x,grid[i][j].koord.y);
                }
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

    char detect(pont fejpoz, char irany)
    {
        block* nextblock;
        bool volt=false;
        for(int i=0;i<grid.size();i++)
        {
            for(int j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].koord==fejpoz && !volt)
                {
                    if(irany=='j')
                    {
                        nextblock=&grid[i][j+1];
                    }
                    else if(irany=='f')
                    {
                        nextblock=&grid[i-1][j];
                    }
                    else if(irany=='l')
                    {
                        nextblock=&grid[i+1][j];
                    }
                    else
                    {
                        nextblock=&grid[i][j-1];
                    }
                    volt=true;
                }
            }
        }
        if(nextblock->status=='a')
        {
            nextblock->status='s';
            return 'a';
        }
        return nextblock->status;
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

    pont almakoord()
    {
        int i=rand()%grid.size(),j=rand()%grid[0].size();
        while(grid[i][j].status!='s')
        {
            i=rand()%grid.size();
            j=rand()%grid[0].size();
        }
        grid[i][j].status='a';
        return grid[i][j].koord;
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

struct Alma
{
    private:
        pont koord;
        canvas textura;
        bool palyan=false;

    public:
    Alma(canvas c)
    {
        textura=c;
    }

    void rajzol()
    {
        if(palyan)
        {
            gout << stamp(textura,koord.x,koord.y);
        }
    }

    void makealma(pont bekoord)
    {
        koord=bekoord;
        palyan=true;
    }

    void megesz()
    {
        palyan=false;
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

void rajzol(canvas hatter, Fej fej, vector<Test*> testek, Alma alma, int pontszam)
{
    stringstream ss;
    gout << stamp(hatter,0,0);
    for(int i=0;i<testek.size();i++)
    {
        testek[i]->rajzol();
    }
    alma.rajzol();
    fej.rajzol();
    ss << "Pontok: " << pontszam;
    gout << color(0,255,0) << move_to((w/2)-gout.twidth(ss.str())/2,blocksize/2) << font("LiberationMono-Bold.ttf",20) << text(ss.str());
}

int main()
{
    ifstream be;
    canvas hatter,fejle,fejfel,fejjobb,fejbal,fejdead,test,fal,fu,almatex;
    Racs racs;
    bool gamestart=false, gameover=false;
    char inputirany='j', fejstatus;
    int almakor=0,pontszam=0;
    gout.open(w,h);

    import(be,"fej2le.kep",fejle);          //
    import(be,"fej2fel.kep",fejfel);        //
    import(be,"fej2jobb.kep",fejjobb);      //
    import(be,"fej2bal.kep",fejbal);        //   Textúrák
    import(be,"fej2dead.kep",fejdead);      //  importálása
    import(be,"test.kep",test);             //
    import(be,"fal.kep",fal);               //
    import(be,"fu.kep",fu);                 //
    import(be,"alma.kep",almatex);          //

    hatter.open(w,h);                                                                                       //
    racs.hatter(hatter,fu,fal);                                                                             //   Rács és háttér
    gout << stamp(hatter,0,0) << font("LiberationMono-Bold.ttf",20);                                        //     létrehozása
    gout << color(0,255,0) << move_to((w/2)-gout.twidth("Pontok: 0")/2,blocksize/2) << text("Pontok: 0");   //

    vector<Test*> testek;                                                                                  //
    Fej fej(racs.xblockszam(),racs.yblockszam(),fejjobb);                                                  //
    fej.rajzol();                                                                                          //
    testek.push_back(new Test(fej.pozicio().x-blocksize,fej.pozicio().y,0,test));                          //  Fej és test
    testek[0]->rajzol();                                                                                   //  alapállapota
    testek.push_back(new Test(testek[0]->pozicio().x-blocksize,testek[0]->pozicio().y,testek[0],test));    //
    testek[1]-> rajzol();                                                                                  //
    gout << refresh;                                                                                       //

    Alma alma(almatex);                     //Alma létrehozása

    gin.timer(400);
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
                almakor++;
                if(almakor==almatime)
                {
                    alma.makealma(racs.almakoord());
                }

                racs.testclear();
                for(int i=testek.size()-1;i>=0;i--)
                {
                    testek[i]->mozog(fej.pozicio());
                    racs.testadd(testek[i]->pozicio());
                }

                fejstatus=racs.detect(fej.pozicio(),inputirany);
                if(fejstatus=='f' || fejstatus=='t')
                {
                    fej.texreplace(fejdead);
                    gameover=true;
                }
                else
                {
                    fej.mozog(inputirany);
                    if(fejstatus=='a')
                    {
                        alma.megesz();
                        testek.push_back(new Test(testek[testek.size()-1]->pozicio().x,testek[testek.size()-1]->pozicio().y,testek[testek.size()-1],test));
                        almakor=0;
                        pontszam+=100;
                    }
                }

                rajzol(hatter,fej,testek,alma,pontszam);
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
